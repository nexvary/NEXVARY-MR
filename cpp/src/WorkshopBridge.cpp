#include "WorkshopBridge.hpp"
#include "Code39Label.hpp"
#include "RecoveryFile.hpp"
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QUuid>
#include <QSaveFile>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QFile>
#include <QSet>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <limits>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QRegularExpression>
namespace {
QStringList migrationSql() {
    return {
      QStringLiteral("CREATE TABLE customers(id INTEGER PRIMARY KEY,name TEXT NOT NULL,phone TEXT NOT NULL DEFAULT '')"),
      QStringLiteral("CREATE TABLE devices(id INTEGER PRIMARY KEY,customer_id INTEGER NOT NULL REFERENCES customers(id),brand TEXT NOT NULL,model TEXT NOT NULL,serial TEXT NOT NULL DEFAULT '')"),
      QStringLiteral("CREATE TABLE orders(id INTEGER PRIMARY KEY,device_id INTEGER NOT NULL REFERENCES devices(id),symptom TEXT NOT NULL,technician TEXT NOT NULL DEFAULT '',status TEXT NOT NULL DEFAULT 'Received' CHECK(status IN ('Received','Diagnosing','Awaiting parts','Repairing','Ready','Delivered','Cancelled')),labor_cents INTEGER NOT NULL DEFAULT 0 CHECK(labor_cents BETWEEN 0 AND 100000000),notes TEXT NOT NULL DEFAULT '',created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
      QStringLiteral("CREATE TABLE parts(id INTEGER PRIMARY KEY,sku TEXT NOT NULL UNIQUE,name TEXT NOT NULL,quantity INTEGER NOT NULL CHECK(quantity BETWEEN 0 AND 1000000),unit_cents INTEGER NOT NULL CHECK(unit_cents BETWEEN 0 AND 100000000))"),
      QStringLiteral("CREATE TABLE order_parts(id INTEGER PRIMARY KEY,order_id INTEGER NOT NULL REFERENCES orders(id),part_id INTEGER NOT NULL REFERENCES parts(id),quantity INTEGER NOT NULL CHECK(quantity>0),unit_cents INTEGER NOT NULL CHECK(unit_cents>=0))"),
      QStringLiteral("CREATE TABLE events(id INTEGER PRIMARY KEY,order_id INTEGER REFERENCES orders(id),action TEXT NOT NULL,created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
      QStringLiteral("CREATE INDEX orders_status ON orders(status)"),
      QStringLiteral("PRAGMA user_version=1")};
}
QString diagnosisMigrationSql() {
    return QStringLiteral("CREATE TABLE diagnoses(id INTEGER PRIMARY KEY,order_id INTEGER NOT NULL REFERENCES orders(id),symptom TEXT NOT NULL CHECK(length(symptom) BETWEEN 1 AND 2000),guidance TEXT NOT NULL CHECK(length(guidance) BETWEEN 1 AND 8000),findings TEXT NOT NULL CHECK(length(findings) BETWEEN 1 AND 8000),evidence TEXT NOT NULL DEFAULT '' CHECK(length(evidence)<=4000),technician TEXT NOT NULL CHECK(length(technician) BETWEEN 1 AND 160),created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)");
}
QString trackingSql() {
    return QStringLiteral("CREATE TABLE tracking_links(order_id INTEGER PRIMARY KEY REFERENCES orders(id),token_hash TEXT NOT NULL UNIQUE CHECK(length(token_hash)=64),expires_at TEXT NOT NULL,message TEXT NOT NULL DEFAULT '' CHECK(length(message)<=1000))");
}
QString labelSql(){return QStringLiteral("CREATE TABLE order_labels(order_id INTEGER PRIMARY KEY REFERENCES orders(id),barcode TEXT NOT NULL UNIQUE CHECK(length(barcode)=20 AND substr(barcode,1,4)='NMR-' AND substr(barcode,5) NOT GLOB '*[^0-9A-F]*'),created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)");}
QVariantList rows(QSqlQuery &q) {
    QVariantList list;
    while(q.next()) { QVariantMap row; for(int i=0;i<q.record().count();++i) row.insert(q.record().fieldName(i),q.value(i)); list.append(row); }
    return list;
}
bool textOk(const QString &s,int max=240) { return !s.trimmed().isEmpty() && s.size()<=max && !s.contains(QChar::Null); }
QString sqlText(const QString &value) { return value.isNull()?QStringLiteral(""):value; }
bool optionalTextOk(const QString &s,int max) { return s.size()<=max && !s.contains(QChar::Null); }
bool sameFile(const QString &a,const QString &b) {
    if(a.isEmpty()||b.isEmpty()) return false;
    const QFileInfo left(a),right(b);
#ifdef Q_OS_WIN
    constexpr auto pathCase=Qt::CaseInsensitive;
#else
    constexpr auto pathCase=Qt::CaseSensitive;
#endif
    return QString::compare(left.absoluteFilePath(),right.absoluteFilePath(),pathCase)==0 ||
        (!left.canonicalFilePath().isEmpty()&&!right.canonicalFilePath().isEmpty()&&QString::compare(left.canonicalFilePath(),right.canonicalFilePath(),pathCase)==0);
}
bool databaseTarget(const QString &target,const QString &db) {
    for(const auto &suffix:QStringList{"","-wal","-shm","-journal"}) if(sameFile(target,db+suffix)) return true;
    return false;
}
QString money(qint64 cents) { return QString::number(cents/100)+QLatin1Char('.')+QString::number(cents%100).rightJustified(2,QLatin1Char('0')); }
QString csv(QString s) {
    // Prevent formula execution when a technician opens exported records in a spreadsheet.
    const QString trimmed=s.trimmed();
    if(!trimmed.isEmpty() && QStringLiteral("=+-@\t\r").contains(trimmed.front())) s.prepend('\'');
    s.replace('"',QStringLiteral("\"\"")); return '"'+s+'"';
}
}
QString WorkshopBridge::localPath(const QString &s) { const QUrl u(s); if(u.isLocalFile()) return u.toLocalFile();return u.scheme().isEmpty()||QFileInfo(s).isAbsolute()?s:QString(); }
WorkshopBridge::WorkshopBridge(const QString &path,QObject *parent):QObject(parent),path_(localPath(path)),connection_(QUuid::createUuid().toString()) { if(!path_.isEmpty()) path_=QFileInfo(path_).absoluteFilePath();ready_=open(); }
WorkshopBridge::~WorkshopBridge() { db_.close(); db_=QSqlDatabase(); QSqlDatabase::removeDatabase(connection_); }
bool WorkshopBridge::fail(const QString &m) { error_=m; emit errorChanged(); return false; }
bool WorkshopBridge::open() {
    if(path_.isEmpty()||path_.contains(QChar::Null)) return fail(QStringLiteral("Choose a valid local database path."));
    QDir().mkpath(QFileInfo(path_).absolutePath());
    db_=QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),connection_); db_.setDatabaseName(path_);
    if(!db_.open()) return fail(db_.lastError().text());
    QSqlQuery q(db_);
    if(!q.exec(QStringLiteral("PRAGMA foreign_keys=ON"))) return fail(q.lastError().text());
    if(!q.exec(QStringLiteral("PRAGMA trusted_schema=OFF"))) return fail(q.lastError().text());
    q.exec(QStringLiteral("PRAGMA busy_timeout=3000"));
    if(!migrate()) return false;
    // User data remains local. Restrict file access on systems that support POSIX permissions.
    QFile::setPermissions(path_,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    return true;
}
bool WorkshopBridge::migrate() {
    QSqlQuery q(db_); if(!q.exec(QStringLiteral("PRAGMA user_version"))||!q.next()) return fail(q.lastError().text());
    const int v=q.value(0).toInt(); q.finish();
    if(v>4) return fail(QStringLiteral("Database belongs to a newer version; refusing downgrade."));
    if(v==4) return validDb(db_) || fail(QStringLiteral("Database schema/integrity validation failed."));
    if(v>0 && !validDb(db_,true)) return fail(QStringLiteral("Legacy database schema/integrity validation failed."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    if(v==0) for(const auto &sql:migrationSql()) if(!q.exec(sql)) { db_.rollback(); return fail(q.lastError().text()); }
    if(v<2&&!q.exec(diagnosisMigrationSql())) { db_.rollback();return fail(q.lastError().text()); }
    if((v<3&&!q.exec(trackingSql()))||!q.exec(labelSql())||!q.exec(QStringLiteral("PRAGMA user_version=4"))) { db_.rollback();return fail(q.lastError().text()); }
    if(!db_.commit()) {db_.rollback();return fail(db_.lastError().text());}
    return validDb(db_) || fail(QStringLiteral("Migrated database failed validation."));
}
bool WorkshopBridge::validDb(QSqlDatabase &db,bool allowLegacy) const {
    QSqlQuery q(db); if(!q.exec(QStringLiteral("PRAGMA integrity_check"))||!q.next()||q.value(0).toString()!=QStringLiteral("ok")) return false;
    q.finish(); if(!q.exec(QStringLiteral("PRAGMA foreign_key_check"))||q.next()) return false;
    q.finish(); if(!q.exec(QStringLiteral("PRAGMA user_version"))||!q.next()||(q.value(0).toInt()!=4 && !(allowLegacy&&(q.value(0).toInt()>=1&&q.value(0).toInt()<=3)))) return false;
    const int version=q.value(0).toInt();
    QMap<QString,QStringList> required={
      {"customers",{"id","name","phone"}}, {"devices",{"id","customer_id","brand","model","serial"}},
      {"orders",{"id","device_id","symptom","technician","status","labor_cents","notes","created_at","updated_at"}},
      {"parts",{"id","sku","name","quantity","unit_cents"}}, {"order_parts",{"id","order_id","part_id","quantity","unit_cents"}}, {"events",{"id","order_id","action","created_at"}}};
    if(version>=2) required.insert(QStringLiteral("diagnoses"),{"id","order_id","symptom","guidance","findings","evidence","technician","created_at"});
    if(version>=3) required.insert(QStringLiteral("tracking_links"),{"order_id","token_hash","expires_at","message"});
    if(version>=4)required.insert(QStringLiteral("order_labels"),{"order_id","barcode","created_at"});
    // Match our own schema, including CHECK/UNIQUE/foreign-key constraints. A file
    // with familiar column names but relaxed constraints is not an application backup.
    QMap<QString,QString> expectedSql;
    for(const auto &sql:migrationSql()) {
        if(!sql.startsWith(QStringLiteral("CREATE TABLE "))) continue;
        const auto name=sql.mid(13,sql.indexOf('(')-13);
        expectedSql.insert(name,sql);
    }
    if(version>=2) expectedSql.insert(QStringLiteral("diagnoses"),diagnosisMigrationSql());
    if(version>=3) expectedSql.insert(QStringLiteral("tracking_links"),trackingSql());
    if(version>=4)expectedSql.insert(QStringLiteral("order_labels"),labelSql());
    // A backup is application data, not a mechanism to install executable triggers/views.
    q.finish();if(!q.exec(QStringLiteral("SELECT name,type,sql FROM sqlite_master WHERE name NOT LIKE 'sqlite_%'"))) return false;
    while(q.next()) {
        const auto name=q.value(0).toString(),type=q.value(1).toString();
        if(type==QStringLiteral("table")) {if(!required.contains(name)||q.value(2).toString()!=expectedSql.value(name)) return false;}
        else if(type!=QStringLiteral("index")||name!=QStringLiteral("orders_status")) return false;
    }
    for(auto it=required.cbegin();it!=required.cend();++it) {
        q.finish(); if(!q.exec(QStringLiteral("PRAGMA table_info(%1)").arg(it.key()))) return false;
        QSet<QString> cols; while(q.next()) cols.insert(q.value(1).toString());
        for(const auto &col:it.value()) if(!cols.contains(col)) return false;
    }
    // Validate numeric affinity and cumulative totals in integer arithmetic. SQL SUM
    // can itself overflow on a hostile or corrupted backup before a limit is checked.
    q.finish();if(!q.exec(QStringLiteral("SELECT 1 FROM parts WHERE typeof(quantity)!='integer' OR quantity NOT BETWEEN 0 AND 1000000 OR typeof(unit_cents)!='integer' OR unit_cents NOT BETWEEN 0 AND 100000000 LIMIT 1"))||q.next()) return false;
    q.finish();if(!q.exec(QStringLiteral("SELECT o.id,o.labor_cents,p.quantity,p.unit_cents,typeof(o.labor_cents),typeof(p.quantity),typeof(p.unit_cents) FROM orders o LEFT JOIN order_parts p ON p.order_id=o.id ORDER BY o.id"))) return false;
    qint64 previous=-1,total=0;
    while(q.next()) {
        const auto id=q.value(0).toLongLong(),labor=q.value(1).toLongLong();
        if(q.value(4).toString()!=QStringLiteral("integer")||labor<0||labor>100000000) return false;
        if(id!=previous) {previous=id;total=labor;}
        if(q.value(2).isNull()) continue;
        bool quantityOk=false,priceOk=false;
        const auto quantity=q.value(2).toLongLong(&quantityOk),price=q.value(3).toLongLong(&priceOk);
        if(!quantityOk||!priceOk||q.value(5).toString()!=QStringLiteral("integer")||q.value(6).toString()!=QStringLiteral("integer")||quantity<1||quantity>1000000||price<0||price>100000000) return false;
        const auto amount=quantity*price;
        if(total>std::numeric_limits<qint64>::max()-amount) return false;
        total+=amount;
    }
    if(q.lastError().isValid()) return false;
    if(version>=2) {
        q.finish();if(!q.exec(QStringLiteral("SELECT symptom,guidance,findings,evidence,technician FROM diagnoses"))) return false;
        while(q.next()) if(!textOk(q.value(0).toString(),2000)||!textOk(q.value(1).toString(),8000)||!textOk(q.value(2).toString(),8000)||!optionalTextOk(q.value(3).toString(),4000)||!textOk(q.value(4).toString(),160)) return false;
        if(q.lastError().isValid()) return false;
        q.finish();if(!q.exec(QStringLiteral("SELECT order_id FROM diagnoses GROUP BY order_id HAVING COUNT(*)>100 LIMIT 1"))||q.next()) return false;
    }
    if(version>=4){
        q.finish();if(!q.exec(QStringLiteral("SELECT barcode,created_at,typeof(barcode) FROM order_labels")))return false;
        while(q.next())if(q.value(2).toString()!=QStringLiteral("text")||!mr::workshop::validBarcode(q.value(0).toString())||!QDateTime::fromString(q.value(1).toString(),QStringLiteral("yyyy-MM-dd HH:mm:ss")).isValid())return false;
        if(q.lastError().isValid())return false;
    }
    return true;
}
QString WorkshopBridge::checkIntegrity() { return validDb(db_)?QStringLiteral("ok"):QStringLiteral("failed"); }
QVariantList WorkshopBridge::orders() const {
    if(!ready_) return {};
    QSqlQuery q(db_); q.exec(QStringLiteral("SELECT o.*,c.name AS customer,c.phone,c.id AS customer_id,d.brand,d.model,d.serial,COALESCE((SELECT barcode FROM order_labels WHERE order_id=o.id),'') AS barcode,COALESCE((SELECT SUM(quantity*unit_cents) FROM order_parts WHERE order_id=o.id),0)+o.labor_cents AS total_cents FROM orders o JOIN devices d ON d.id=o.device_id JOIN customers c ON c.id=d.customer_id ORDER BY o.id DESC")); auto result=rows(q);for(auto &v:result){auto row=v.toMap();row.insert("total_display",money(row.value("total_cents").toLongLong()));v=row;}return result;
}
QVariantList WorkshopBridge::inventory() const { if(!ready_) return {}; QSqlQuery q(db_); q.exec(QStringLiteral("SELECT * FROM parts ORDER BY name"));return rows(q); }
QVariantList WorkshopBridge::events() const { if(!ready_) return {}; QSqlQuery q(db_); q.exec(QStringLiteral("SELECT * FROM events ORDER BY id DESC LIMIT 300"));return rows(q); }
bool WorkshopBridge::log(int id,const QString &action) { QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO events(order_id,action) VALUES(?,?)"));q.addBindValue(id>0?QVariant(id):QVariant());q.addBindValue(action);return q.exec(); }
bool WorkshopBridge::finish(bool ok) {
    if(!ok) { const auto e=db_.lastError().text(); db_.rollback(); return fail(QStringLiteral("Transaction failed. ")+e); }
    if(!db_.commit()) { db_.rollback(); return fail(db_.lastError().text()); }
    error_.clear(); emit errorChanged(); emit changed(); return true;
}
int WorkshopBridge::createOrder(const QString &customer,const QString &phone,const QString &brand,const QString &model,const QString &serial,const QString &symptom,const QString &tech,const QString &notes) {
    if(!ready_||!textOk(customer)||!textOk(brand)||!textOk(model)||!textOk(symptom,2000)||!optionalTextOk(phone,80)||!optionalTextOk(serial,160)||!optionalTextOk(tech,160)||!optionalTextOk(notes,20000)) { fail(QStringLiteral("Customer, brand, model and symptom are required; text exceeds allowed length."));return -1; }
    if(!db_.transaction()) { fail(db_.lastError().text()); return -1; }
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO customers(name,phone) VALUES(?,?)"));q.addBindValue(customer.trimmed());q.addBindValue(sqlText(phone.trimmed()));
    if(!q.exec()) { const auto message=q.lastError().text();db_.rollback();fail(message);return -1; } const int customerId=q.lastInsertId().toInt();
    q.prepare(QStringLiteral("INSERT INTO devices(customer_id,brand,model,serial) VALUES(?,?,?,?)"));q.addBindValue(customerId);q.addBindValue(brand.trimmed());q.addBindValue(model.trimmed());q.addBindValue(sqlText(serial.trimmed()));
    if(!q.exec()) { const auto message=q.lastError().text();db_.rollback();fail(message);return -1; } const int deviceId=q.lastInsertId().toInt();
    q.prepare(QStringLiteral("INSERT INTO orders(device_id,symptom,technician,notes) VALUES(?,?,?,?)"));q.addBindValue(deviceId);q.addBindValue(symptom.trimmed());q.addBindValue(sqlText(tech.trimmed()));q.addBindValue(sqlText(notes));
    if(!q.exec()) { const auto message=q.lastError().text();db_.rollback();fail(message);return -1; } const int id=q.lastInsertId().toInt();
    return finish(log(id,QStringLiteral("Created")))?id:-1;
}
bool WorkshopBridge::updateOrder(int id,const QString &status,const QString &tech,int cents,const QString &notes) {
    const QStringList states={"Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"};
    if(!ready_||!states.contains(status)||cents<0||cents>100000000||!optionalTextOk(notes,20000)||!optionalTextOk(tech,160)) return fail(QStringLiteral("Invalid order values."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT COALESCE(SUM(quantity*unit_cents),0) FROM order_parts WHERE order_id=?"));q.addBindValue(id);
    if(!q.exec()||!q.next()||q.value(0).toLongLong()>std::numeric_limits<qint64>::max()-cents) {db_.rollback();return fail(QStringLiteral("Invoice total exceeds the integer monetary limit."));}
    q.finish();q.prepare(QStringLiteral("UPDATE orders SET status=?,technician=?,labor_cents=?,notes=?,updated_at=CURRENT_TIMESTAMP WHERE id=?"));q.addBindValue(status);q.addBindValue(sqlText(tech));q.addBindValue(cents);q.addBindValue(sqlText(notes));q.addBindValue(id);
    return finish(q.exec()&&q.numRowsAffected()==1&&log(id,QStringLiteral("Status: ")+status));
}
bool WorkshopBridge::addPart(const QString &sku,const QString &name,int qty,int cents) {
    if(!ready_||!textOk(sku,100)||!textOk(name)||qty<0||qty>1000000||cents<0||cents>100000000) return fail(QStringLiteral("Invalid stock values."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO parts(sku,name,quantity,unit_cents) VALUES(?,?,?,?)"));q.addBindValue(sku.trimmed());q.addBindValue(name.trimmed());q.addBindValue(qty);q.addBindValue(cents);
    return finish(q.exec()&&log(0,QStringLiteral("Stock added: ")+sku));
}
bool WorkshopBridge::usePart(int orderId,int partId,int qty) {
    if(!ready_||qty<1||qty>1000000) return fail(QStringLiteral("Invalid part quantity."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT unit_cents FROM parts WHERE id=? AND quantity>=?"));q.addBindValue(partId);q.addBindValue(qty);
    if(!q.exec()||!q.next()) { db_.rollback(); return fail(QStringLiteral("Insufficient stock.")); } const int cents=q.value(0).toInt();q.finish();
    q.prepare(QStringLiteral("SELECT labor_cents,COALESCE((SELECT SUM(quantity*unit_cents) FROM order_parts WHERE order_id=orders.id),0) FROM orders WHERE id=?"));q.addBindValue(orderId);
    const qint64 cost=static_cast<qint64>(qty)*cents;
    if(!q.exec()||!q.next()||q.value(1).toLongLong()>std::numeric_limits<qint64>::max()-q.value(0).toLongLong()-cost) {db_.rollback();return fail(QStringLiteral("Order missing or invoice exceeds the integer monetary limit."));}
    q.finish();
    q.prepare(QStringLiteral("INSERT INTO order_parts(order_id,part_id,quantity,unit_cents) VALUES(?,?,?,?)"));q.addBindValue(orderId);q.addBindValue(partId);q.addBindValue(qty);q.addBindValue(cents);
    if(!q.exec()) return finish(false);
    q.prepare(QStringLiteral("UPDATE parts SET quantity=quantity-? WHERE id=? AND quantity>=?"));q.addBindValue(qty);q.addBindValue(partId);q.addBindValue(qty);
    return finish(q.exec()&&q.numRowsAffected()==1&&log(orderId,QStringLiteral("Part allocated: %1 x%2").arg(partId).arg(qty)));
}
QVariantMap WorkshopBridge::invoice(int id) const {
    QVariantMap result;
    for(const auto &r:orders()) if(r.toMap().value("id").toInt()==id) { result=r.toMap();break; }
    if(result.isEmpty()) return result;
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT p.sku,p.name,op.quantity,op.unit_cents FROM order_parts op JOIN parts p ON p.id=op.part_id WHERE order_id=?"));q.addBindValue(id);q.exec();result.insert("parts",rows(q));result.insert("diagnoses",diagnoses(id));return result;
}
bool WorkshopBridge::exportOrders(const QString &value) {
    if(!ready_) return fail(QStringLiteral("Database is not ready."));
    const auto target=localPath(value);
    if(!ready_||target.isEmpty()||databaseTarget(target,path_)) return fail(QStringLiteral("Export requires a local file separate from the active database."));
    QSaveFile file(target);if(!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    QString out=QString(QChar(0xFEFF))+QStringLiteral("id,customer,phone,brand,model,serial,symptom,status,technician,total_cents,notes,barcode\r\n");
    for(const auto &v:orders()) {const auto m=v.toMap(); QStringList line;for(const auto &key:QStringList{"id","customer","phone","brand","model","serial","symptom","status","technician","total_cents","notes","barcode"}) line<<csv(m.value(key).toString());out+=line.join(',')+QStringLiteral("\r\n");}
    const auto bytes=out.toUtf8();if(file.write(bytes)!=bytes.size()||!file.commit()) return fail(file.errorString());error_.clear();emit errorChanged();return true;
}
bool WorkshopBridge::exportInvoice(int id,const QString &value) {
    const auto target=localPath(value);
    if(!ready_||target.isEmpty()||databaseTarget(target,path_)) return fail(QStringLiteral("Export requires a local file separate from the active database."));
    const auto m=invoice(id); if(m.isEmpty()) return fail(QStringLiteral("Order not found."));
    QString html=QStringLiteral("<html><body><h1>NEXVARY MR</h1><h2>Repair service invoice / فاتورة صيانة #%1</h2><p>%2<br>%3 %4<br>%5</p><table width='100%'><tr><th>Part / القطعة</th><th>Quantity</th><th>Unit price</th></tr>").arg(id).arg(m.value("customer").toString().toHtmlEscaped(),m.value("brand").toString().toHtmlEscaped(),m.value("model").toString().toHtmlEscaped(),m.value("status").toString().toHtmlEscaped());
    for(const auto &r:m.value("parts").toList()) { const auto p=r.toMap();html+=QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(p.value("name").toString().toHtmlEscaped()).arg(p.value("quantity").toInt()).arg(money(p.value("unit_cents").toLongLong())); }
    html+=QStringLiteral("</table><p>Labor / أجرة العمل: %1</p><h2>Total / الإجمالي: %2</h2><p>Local currency • No tax calculated / العملة المحلية، دون حساب ضريبة</p><p>%3</p></body></html>").arg(money(m.value("labor_cents").toLongLong())).arg(money(m.value("total_cents").toLongLong())).arg(m.value("notes").toString().toHtmlEscaped());
    if(!m.value("diagnoses").toList().isEmpty()) {
        QString details=QStringLiteral("<h2>Saved diagnostic findings / نتائج التشخيص المحفوظة</h2><p>Technician observations and rule guidance; not a certified diagnosis / ملاحظات الفني وإرشادات قواعد، وليست تشخيصًا معتمدًا</p>");
        auto plainHtml=[](const QString &text) {return text.toHtmlEscaped().replace(QLatin1Char('\n'),QStringLiteral("<br>"));};
        for(const auto &entry:m.value("diagnoses").toList()) {
            const auto d=entry.toMap();
            details+=QStringLiteral("<h3>#%1 · %2 · %3</h3><p>Technician / الفني: %4</p><p><b>Findings / الملاحظات:</b><br>%5</p><p><b>Evidence reviewed / الدليل المُراجع:</b><br>%6</p><p><b>Rule guidance / إرشاد القواعد:</b><br>%7</p>").arg(d.value("id").toInt()).arg(plainHtml(d.value("created_at").toString()),plainHtml(d.value("symptom").toString()),plainHtml(d.value("technician").toString()),plainHtml(d.value("findings").toString()),plainHtml(d.value("evidence").toString()),plainHtml(d.value("guidance").toString()));
        }
        html.replace(QStringLiteral("</body>"),details+QStringLiteral("</body>"));
    }
    QSaveFile output(target);if(!output.open(QIODevice::WriteOnly)) return fail(output.errorString());
    { QPdfWriter pdf(&output);pdf.setPageSize(QPageSize(QPageSize::A4));pdf.setResolution(96);QTextDocument doc;doc.setHtml(html);doc.print(&pdf); }
    if(output.error()!=QFileDevice::NoError||output.size()==0) return fail(QStringLiteral("Invoice PDF generation failed."));
    if(!output.commit()) return fail(output.errorString());
    error_.clear();emit errorChanged();return true;
}
bool WorkshopBridge::backup(const QString &value) {
    if(!ready_) return fail(QStringLiteral("Database is not ready."));
    const QString target=localPath(value);if(!ready_||target.isEmpty()||databaseTarget(target,path_)||QFileInfo::exists(target)) return fail(QStringLiteral("Choose a new local backup filename separate from the active database."));
    QSqlQuery q(db_);q.prepare(QStringLiteral("VACUUM INTO ?"));q.addBindValue(target);
    // Never delete a destination on failure: another process may have created it after the existence check.
    if(!q.exec()) return fail(q.lastError().text());
    QFile::setPermissions(target,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    error_.clear();emit errorChanged();
    return true;
}
bool WorkshopBridge::restore(const QString &value,bool confirmed) {
    if(!ready_) return fail(QStringLiteral("The current database is not ready."));
    if(!confirmed) return fail(QStringLiteral("Restore requires explicit confirmation."));
    const QString source=localPath(value);
    if(source.isEmpty()||databaseTarget(source,path_)||QFileInfo(source).size()>100*1024*1024||!QFileInfo(source).isFile()) return fail(QStringLiteral("Invalid backup file."));
    // Read and validate a staged copy to prevent source mutation between validation and restore.
    QTemporaryFile staged(QFileInfo(path_).absolutePath()+QStringLiteral("/.nexvary-restore-XXXXXX"));
    QFile original(source);
    if(!staged.open()||!original.open(QIODevice::ReadOnly)) return fail(QStringLiteral("Cannot stage backup."));
    qint64 copied=0;
    while(!original.atEnd()) {
        const auto chunk=original.read(65536);copied+=chunk.size();
        if(chunk.isEmpty()||copied>100*1024*1024||staged.write(chunk)!=chunk.size()) return fail(QStringLiteral("Backup read failed or exceeded 100 MiB."));
    }
    if(original.error()!=QFileDevice::NoError||!staged.flush()) return fail(QStringLiteral("Cannot finish staging backup."));
    original.close();const auto stage=staged.fileName();staged.close();
    const QString name=QUuid::createUuid().toString();bool valid=false;
    {auto check=QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),name);check.setDatabaseName(stage);check.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));if(check.open()) { QSqlQuery safe(check);safe.exec(QStringLiteral("PRAGMA trusted_schema=OFF"));valid=validDb(check,true); }check.close();}
    QSqlDatabase::removeDatabase(name);
    if(!valid) {QFile::remove(stage);return fail(QStringLiteral("Backup failed schema, integrity or foreign key checks."));}
    const QString previous=path_+QStringLiteral(".before-restore-")+QUuid::createUuid().toString(QUuid::Id128)+QStringLiteral(".db");
    if(!backup(previous)) {QFile::remove(stage);return false;}
    db_.close();
    QSaveFile dest(path_);QFile input(stage);bool ok=input.open(QIODevice::ReadOnly)&&dest.open(QIODevice::WriteOnly);
    if(ok) {const auto bytes=input.readAll();ok=dest.write(bytes)==bytes.size()&&dest.commit();}
    input.close();QFile::remove(stage);
    ready_=db_.open()&&migrate();QSqlQuery q(db_);ready_=q.exec(QStringLiteral("PRAGMA foreign_keys=ON"))&&q.exec(QStringLiteral("PRAGMA trusted_schema=OFF"))&&ready_;
    if(ready_) ready_=q.exec(QStringLiteral("DELETE FROM tracking_links"));
    QFile::setPermissions(path_,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    emit changed();if(ok&&ready_) {error_.clear();emit errorChanged();return true;}return fail(QStringLiteral("Restore failed; original preserved in ")+previous);
}

bool WorkshopBridge::recordDiagnostic(const QString &operation,bool success) {
    const QStringList allowed={"adb-scan","fastboot-scan","properties","logcat","drivers"};
    if(!ready_||!allowed.contains(operation)) return false;
    if(!db_.transaction()) return fail(db_.lastError().text());
    return finish(log(0,QStringLiteral("Read-only %1: %2").arg(operation,success?QStringLiteral("success"):QStringLiteral("failed/cancelled"))));
}

bool WorkshopBridge::restockPart(int partId,int quantity,int unitCents) {
    if(!ready_||quantity<1||quantity>1000000||unitCents<0||unitCents>100000000) return fail(QStringLiteral("Invalid restock values."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("UPDATE parts SET quantity=quantity+?,unit_cents=? WHERE id=? AND quantity<=1000000-?"));
    q.addBindValue(quantity);q.addBindValue(unitCents);q.addBindValue(partId);q.addBindValue(quantity);
    return finish(q.exec()&&q.numRowsAffected()==1&&log(0,QStringLiteral("Stock received: %1 x%2").arg(partId).arg(quantity)));
}

QVariantList WorkshopBridge::diagnoses(int orderId) const {
    if(!ready_||orderId<=0) return {};
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT * FROM diagnoses WHERE order_id=? ORDER BY id DESC LIMIT 100"));q.addBindValue(orderId);
    if(!q.exec()) return {};
    return rows(q);
}
bool WorkshopBridge::saveDiagnosis(int orderId,const QString &symptom,const QString &guidance,const QString &findings,const QString &evidence,const QString &technician) {
    if(!ready_||orderId<=0||!textOk(symptom,2000)||!textOk(guidance,8000)||!textOk(findings,8000)||!optionalTextOk(evidence,4000)||!textOk(technician,160)) return fail(QStringLiteral("Select an existing repair order; symptom, rule guidance, findings and technician are required within their size limits."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT (SELECT COUNT(*) FROM diagnoses WHERE order_id=orders.id) FROM orders WHERE id=?"));q.addBindValue(orderId);
    if(!q.exec()||!q.next()||q.value(0).toInt()>=100) {db_.rollback();return fail(QStringLiteral("Repair order not found or already has 100 saved diagnostic entries."));}
    q.finish();q.prepare(QStringLiteral("INSERT INTO diagnoses(order_id,symptom,guidance,findings,evidence,technician) VALUES(?,?,?,?,?,?)"));
    q.addBindValue(orderId);q.addBindValue(symptom.trimmed());q.addBindValue(guidance);q.addBindValue(findings.trimmed());q.addBindValue(sqlText(evidence));q.addBindValue(technician.trimmed());
    return finish(q.exec()&&log(orderId,QStringLiteral("Diagnostic findings saved")));
}
bool WorkshopBridge::exportDiagnosisReport(int orderId,const QString &value) {
    const auto target=localPath(value);
    if(!ready_||target.isEmpty()||databaseTarget(target,path_)) return fail(QStringLiteral("Export requires a local report file separate from the active database."));
    const auto order=invoice(orderId);
    if(order.isEmpty()) return fail(QStringLiteral("Repair order not found."));
    const auto entries=diagnoses(orderId);
    if(entries.isEmpty()) return fail(QStringLiteral("No saved diagnostic findings for this repair order."));
    const QJsonObject report{{QStringLiteral("schemaVersion"),1},{QStringLiteral("reportType"),QStringLiteral("nexvary-mr-diagnostic-findings")},{QStringLiteral("generatedAtUtc"),QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{QStringLiteral("notice"),QStringLiteral("Technician observations and rule-based guidance only; not a certified fault diagnosis. Raw ADB logs are not captured automatically. Contains customer data; review before sharing.")},{QStringLiteral("order"),QJsonObject::fromVariantMap(order)}};
    const auto bytes=QJsonDocument(report).toJson(QJsonDocument::Indented);
    QSaveFile file(target);if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit()) return fail(QStringLiteral("Could not write diagnostic report."));
    error_.clear();emit errorChanged();return true;
}

QVariantList WorkshopBridge::customers() const {
    if(!ready_)return {};
    QSqlQuery q(db_);q.exec(QStringLiteral("SELECT c.*,(SELECT COUNT(*) FROM devices d WHERE d.customer_id=c.id) AS device_count,(SELECT COUNT(*) FROM orders o JOIN devices d ON d.id=o.device_id WHERE d.customer_id=c.id) AS order_count FROM customers c ORDER BY c.name,c.id"));return rows(q);
}
QVariantList WorkshopBridge::devices() const {
    if(!ready_)return {};
    QSqlQuery q(db_);q.exec(QStringLiteral("SELECT d.*,c.name AS owner,c.phone,(SELECT COUNT(*) FROM orders o WHERE o.device_id=d.id) AS order_count FROM devices d JOIN customers c ON c.id=d.customer_id ORDER BY d.id DESC"));return rows(q);
}
QVariantMap WorkshopBridge::customerProfile(int id) const {
    for(const auto &item:customers())if(item.toMap().value("id").toInt()==id){
        auto result=item.toMap();QVariantList phones,history;
        for(const auto &d:devices())if(d.toMap().value("customer_id").toInt()==id)phones.append(d);
        for(const auto &o:orders())if(o.toMap().value("customer_id").toInt()==id)history.append(o);
        result.insert("devices",phones);result.insert("orders",history);return result;
    }return {};
}
QVariantMap WorkshopBridge::deviceProfile(int id) const {
    for(const auto &item:devices())if(item.toMap().value("id").toInt()==id){
        auto result=item.toMap();QVariantList history;
        for(const auto &o:orders())if(o.toMap().value("device_id").toInt()==id)history.append(o);
        result.insert("orders",history);return result;
    }return {};
}
bool WorkshopBridge::updateCustomer(int id,const QString &name,const QString &phone) {
    if(!ready_||!textOk(name)||!optionalTextOk(phone,80))return fail(QStringLiteral("Valid customer name and phone are required."));
    if(!db_.transaction())return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("UPDATE customers SET name=?,phone=? WHERE id=?"));q.addBindValue(name.trimmed());q.addBindValue(sqlText(phone.trimmed()));q.addBindValue(id);
    return finish(q.exec()&&q.numRowsAffected()==1&&log(0,QStringLiteral("Customer profile updated #%1").arg(id)));
}
int WorkshopBridge::createOrderForDevice(int id,const QString &symptom,const QString &tech,const QString &notes) {
    if(!ready_||!textOk(symptom,2000)||!optionalTextOk(tech,160)||!optionalTextOk(notes,20000)){fail(QStringLiteral("Invalid order values."));return -1;}
    if(!db_.transaction()){fail(db_.lastError().text());return -1;}
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO orders(device_id,symptom,technician,notes) VALUES(?,?,?,?)"));q.addBindValue(id);q.addBindValue(symptom.trimmed());q.addBindValue(sqlText(tech.trimmed()));q.addBindValue(sqlText(notes));
    if(!q.exec()){db_.rollback();fail(QStringLiteral("Existing device not found or order could not be saved."));return -1;}
    const int order=q.lastInsertId().toInt();return finish(log(order,QStringLiteral("Created")))?order:-1;
}
int WorkshopBridge::createOrderForCustomer(int customerId,const QString &brand,const QString &model,const QString &serial,const QString &symptom,const QString &tech,const QString &notes) {
    if(!ready_||!textOk(brand)||!textOk(model)||!optionalTextOk(serial,160)||!textOk(symptom,2000)||!optionalTextOk(tech,160)||!optionalTextOk(notes,20000)){fail(QStringLiteral("Invalid device/order values."));return -1;}
    if(!db_.transaction()){fail(db_.lastError().text());return -1;}
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO devices(customer_id,brand,model,serial) VALUES(?,?,?,?)"));q.addBindValue(customerId);q.addBindValue(brand.trimmed());q.addBindValue(model.trimmed());q.addBindValue(sqlText(serial.trimmed()));
    if(!q.exec()){db_.rollback();fail(QStringLiteral("Customer not found or phone could not be saved."));return -1;}
    const int device=q.lastInsertId().toInt();q.prepare(QStringLiteral("INSERT INTO orders(device_id,symptom,technician,notes) VALUES(?,?,?,?)"));q.addBindValue(device);q.addBindValue(symptom.trimmed());q.addBindValue(sqlText(tech.trimmed()));q.addBindValue(sqlText(notes));
    if(!q.exec()){db_.rollback();fail(QStringLiteral("Could not save order."));return -1;}
    const int order=q.lastInsertId().toInt();return finish(log(order,QStringLiteral("Created")))?order:-1;
}
QString WorkshopBridge::issueTrackingToken(int id,int days,bool consent) {
    if(!ready_||!consent||days<1||days>90){fail(QStringLiteral("Tracking needs explicit sharing consent and expiry of 1–90 days."));return {};}
    QByteArray random(32,Qt::Uninitialized);for(int i=0;i<32;i+=4){const auto word=QRandomGenerator::system()->generate();for(int j=0;j<4;++j)random[i+j]=static_cast<char>((word>>(8*j))&255);}
    const auto token=random.toHex();const auto digest=QCryptographicHash::hash(token,QCryptographicHash::Sha256).toHex();
    if(!db_.transaction()){fail(db_.lastError().text());return {};}
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO tracking_links(order_id,token_hash,expires_at) VALUES(?,?,?) ON CONFLICT(order_id) DO UPDATE SET token_hash=excluded.token_hash,expires_at=excluded.expires_at"));q.addBindValue(id);q.addBindValue(QString::fromLatin1(digest));q.addBindValue(QDateTime::currentDateTimeUtc().addDays(days).toString(Qt::ISODate));
    return finish(q.exec()&&log(id,QStringLiteral("Tracking link issued/rotated")))?QString::fromLatin1(token):QString();
}
bool WorkshopBridge::revokeTracking(int id) {
    if(!ready_||!db_.transaction())return fail(QStringLiteral("Database unavailable."));
    QSqlQuery q(db_);q.prepare(QStringLiteral("DELETE FROM tracking_links WHERE order_id=?"));q.addBindValue(id);return finish(q.exec()&&q.numRowsAffected()==1&&log(id,QStringLiteral("Tracking link revoked")));
}
bool WorkshopBridge::setPublicMessage(int id,const QString &message) {
    if(!ready_||!optionalTextOk(message,1000)||!db_.transaction())return fail(QStringLiteral("Customer update must be at most 1000 characters."));
    QSqlQuery q(db_);q.prepare(QStringLiteral("UPDATE tracking_links SET message=? WHERE order_id=?"));q.addBindValue(sqlText(message));q.addBindValue(id);return finish(q.exec()&&q.numRowsAffected()==1&&log(id,QStringLiteral("Customer-visible update changed")));
}
QVariantMap WorkshopBridge::publicTracking(const QString &token) const {
    static const QRegularExpression format(QStringLiteral("^[0-9a-f]{64}$"));
    if(!ready_||!format.match(token).hasMatch())return {};
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT order_id,expires_at,message FROM tracking_links WHERE token_hash=?"));q.addBindValue(QString::fromLatin1(QCryptographicHash::hash(token.toLatin1(),QCryptographicHash::Sha256).toHex()));
    if(!q.exec()||!q.next())return {};
    const int id=q.value(0).toInt();const auto expiry=QDateTime::fromString(q.value(1).toString(),Qt::ISODate);const auto message=q.value(2).toString();
    if(!expiry.isValid()||expiry<=QDateTime::currentDateTimeUtc())return {};
    return publicOrderStatus(id,message);
}
QVariantMap WorkshopBridge::publicOrderStatus(int id,const QString &message) const {
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT o.id,o.status,o.updated_at,d.brand,d.model FROM orders o JOIN devices d ON d.id=o.device_id WHERE o.id=?"));q.addBindValue(id);if(!q.exec()||!q.next())return {};
    QVariantMap result;for(int i=0;i<q.record().count();++i)result.insert(q.record().fieldName(i),q.value(i));result.insert("message",message);
    q.finish();q.prepare(QStringLiteral("SELECT action,created_at FROM events WHERE order_id=? ORDER BY id"));q.addBindValue(id);if(!q.exec())return {};
    const QStringList states={"Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"};QVariantList history;
    while(q.next()) {const auto action=q.value(0).toString();const auto status=action==QStringLiteral("Created")?QStringLiteral("Received"):action.startsWith(QStringLiteral("Status: "))?action.mid(8):QString();if(states.contains(status))history.append(QVariantMap{{"status",status},{"atUtc",q.value(1).toString()+QStringLiteral("Z")}});}
    result.insert("history",history);return result;
}

QVariantList WorkshopBridge::publicTrackingSnapshot() const {
    if(!ready_)return {};
    QSqlQuery q(db_);if(!q.exec(QStringLiteral("SELECT order_id,token_hash,expires_at,message FROM tracking_links ORDER BY order_id")))return {};
    QVariantList records;
    while(q.next()){
        const auto expiry=QDateTime::fromString(q.value(2).toString(),Qt::ISODate);if(!expiry.isValid()||expiry<=QDateTime::currentDateTimeUtc())continue;
        auto record=publicOrderStatus(q.value(0).toInt(),q.value(3).toString());if(record.isEmpty())continue;
        const auto updated=QDateTime::fromString(record.take("updated_at").toString().replace(QLatin1Char(' '),QLatin1Char('T'))+QStringLiteral("Z"),Qt::ISODate);
        record.insert("updatedAtUtc",updated.toString(Qt::ISODate));record.insert("expiresAtUtc",expiry.toUTC().toString(Qt::ISODate));record.insert("tokenHash",q.value(1));
        auto history=record.value("history").toList();if(history.size()>100)history=history.mid(history.size()-100);record.insert("history",history);
        records.append(record);
    }
    return records;
}


QVariantMap WorkshopBridge::deviceCard(int orderId) const {
 if(!ready_||orderId<=0)return {};
 QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT o.id AS order_id,o.device_id,c.name AS customer,d.brand,d.model,d.serial,o.symptom,o.status,o.created_at AS received_at,l.barcode FROM orders o JOIN devices d ON d.id=o.device_id JOIN customers c ON c.id=d.customer_id JOIN order_labels l ON l.order_id=o.id WHERE o.id=?"));q.addBindValue(orderId);
 if(!q.exec()||!q.next())return {};
 QVariantMap card;for(int i=0;i<q.record().count();++i)card.insert(q.record().fieldName(i),q.value(i));
 const auto image=mr::workshop::barcodeImage(card.value("barcode").toString());if(image.isNull())return {};
 QByteArray png;QBuffer buffer(&png);if(!buffer.open(QIODevice::WriteOnly)||!image.save(&buffer,"PNG"))return {};
 card.insert("barcodeImage",QStringLiteral("data:image/png;base64,")+QString::fromLatin1(png.toBase64()));return card;
}
QVariantMap WorkshopBridge::issueDeviceCard(int orderId){
 if(!ready_||orderId<=0){fail(QStringLiteral("Select an existing repair order."));return {};}
 QSqlQuery exists(db_);exists.prepare(QStringLiteral("SELECT id FROM orders WHERE id=?"));exists.addBindValue(orderId);if(!exists.exec()||!exists.next()){fail(QStringLiteral("Repair order not found."));return {};}
 exists.finish();const auto existing=deviceCard(orderId);if(!existing.isEmpty())return existing;
 if(!db_.transaction()){fail(db_.lastError().text());return {};}
 for(int attempt=0;attempt<5;++attempt){
  const auto code=QStringLiteral("NMR-%1%2").arg(QRandomGenerator::system()->generate(),8,16,QChar('0')).arg(QRandomGenerator::system()->generate(),8,16,QChar('0')).toUpper();
  QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT OR IGNORE INTO order_labels(order_id,barcode) VALUES(?,?)"));q.addBindValue(orderId);q.addBindValue(code);
  if(!q.exec()){db_.rollback();fail(QStringLiteral("Could not create device barcode."));return {};}
  const bool inserted=q.numRowsAffected()==1;q.finish();const auto card=deviceCard(orderId);
  if(card.isEmpty())continue;
  if((inserted&&!log(orderId,QStringLiteral("Device intake barcode issued")))||!db_.commit()){db_.rollback();fail(QStringLiteral("Device card transaction failed."));return {};}
  error_.clear();emit errorChanged();emit changed();return card;
 }
 db_.rollback();fail(QStringLiteral("Could not allocate a unique device barcode."));return {};
}
QVariantMap WorkshopBridge::lookupBarcode(const QString &input){
 const auto code=input.trimmed();
 if(!ready_||input.size()>64||!mr::workshop::validBarcode(code)){fail(QStringLiteral("Scan or enter a complete NMR device barcode."));return {};}
 QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT order_id FROM order_labels WHERE barcode=?"));q.addBindValue(code);
 if(!q.exec()||!q.next()){fail(QStringLiteral("Barcode not found in this workshop database."));return {};}
 const int id=q.value(0).toInt();q.finish();error_.clear();emit errorChanged();return deviceCard(id);
}
QVariantList WorkshopBridge::orderHistory(int orderId) const {
 if(!ready_||orderId<=0)return {};
 QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT id,action,created_at FROM events WHERE order_id=? ORDER BY id DESC LIMIT 200"));q.addBindValue(orderId);return q.exec()?rows(q):QVariantList();
}
bool WorkshopBridge::exportIntakeCard(int orderId,const QString &input,bool approved){
 const auto path=mr::recovery::localFilePath(input);const auto card=deviceCard(orderId);
 if(!ready_||!approved||path.isEmpty()||card.isEmpty()||databaseTarget(path,path_)||QFileInfo::exists(path))return fail(QStringLiteral("Approve customer-data export and choose a new local PDF filename."));
 auto safe=[&](const char *key){return card.value(QLatin1String(key)).toString().toHtmlEscaped();};
 const auto code=card.value("barcode").toString();
 QString html=QStringLiteral("<html><body><h1>NEXVARY MR</h1><h2>Device intake receipt / بطاقة استلام جهاز</h2><p>Order / الأمر: #%1</p><p>%2<br>%3 %4<br>Serial / التسلسلي: %5</p><p>Received UTC / الاستلام: %6<br>Status / الحالة: %7</p><p>Symptom / العَرَض: %8</p><p><img src='mr-device-barcode' width='600' height='72'></p><p style='font-family:monospace'>%9</p><p>Local order lookup only; not a tracking credential / باركود محلي للأمر، وليس تصريحًا لدخول بوابة التتبع</p></body></html>").arg(orderId).arg(safe("customer"),safe("brand"),safe("model"),safe("serial"),safe("received_at"),safe("status"),safe("symptom"),code);
 QByteArray bytes;QBuffer buffer(&bytes);if(!buffer.open(QIODevice::WriteOnly))return fail(QStringLiteral("Could not prepare intake PDF."));
 {QPdfWriter writer(&buffer);writer.setTitle(QStringLiteral("NEXVARY MR intake card"));writer.setCreator(QStringLiteral("NEXVARY MR"));writer.setPageSize(QPageSize(QPageSize::A4));writer.setResolution(96);QTextDocument document;document.addResource(QTextDocument::ImageResource,QUrl("mr-device-barcode"),mr::workshop::barcodeImage(code));document.setHtml(html);document.print(&writer);}
 if(!bytes.startsWith("%PDF-"))return fail(QStringLiteral("Intake PDF generation failed."));
 QFile out(path);if(!out.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail(QStringLiteral("Could not create new intake PDF."));out.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 const bool ok=out.write(bytes)==bytes.size()&&out.flush();out.close();if(!ok){QFile::remove(path);return fail(QStringLiteral("Intake PDF write failed."));}
 if(!log(orderId,QStringLiteral("Device intake PDF exported with customer-data approval")))return fail(QStringLiteral("Intake PDF exported, but its audit entry failed."));
 error_.clear();emit errorChanged();emit changed();return true;
}
