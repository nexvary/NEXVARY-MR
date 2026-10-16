#include "WorkshopBridge.hpp"
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
#include <limits>
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
QVariantList rows(QSqlQuery &q) {
    QVariantList list;
    while(q.next()) { QVariantMap row; for(int i=0;i<q.record().count();++i) row.insert(q.record().fieldName(i),q.value(i)); list.append(row); }
    return list;
}
bool textOk(const QString &s,int max=240) { return !s.trimmed().isEmpty() && s.size()<=max && !s.contains(QChar::Null); }
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
    if(v>1) return fail(QStringLiteral("Database belongs to a newer version; refusing downgrade."));
    if(v==1) return validDb(db_) || fail(QStringLiteral("Database schema/integrity validation failed."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    const auto migration=migrationSql();
    for(const auto &sql:migration) if(!q.exec(sql)) { db_.rollback(); return fail(q.lastError().text()); }
    return db_.commit() || fail(db_.lastError().text());
}
bool WorkshopBridge::validDb(QSqlDatabase &db) const {
    QSqlQuery q(db); if(!q.exec(QStringLiteral("PRAGMA integrity_check"))||!q.next()||q.value(0).toString()!=QStringLiteral("ok")) return false;
    q.finish(); if(!q.exec(QStringLiteral("PRAGMA foreign_key_check"))||q.next()) return false;
    q.finish(); if(!q.exec(QStringLiteral("PRAGMA user_version"))||!q.next()||q.value(0).toInt()!=1) return false;
    const QMap<QString,QStringList> required={
      {"customers",{"id","name","phone"}}, {"devices",{"id","customer_id","brand","model","serial"}},
      {"orders",{"id","device_id","symptom","technician","status","labor_cents","notes","created_at","updated_at"}},
      {"parts",{"id","sku","name","quantity","unit_cents"}}, {"order_parts",{"id","order_id","part_id","quantity","unit_cents"}}, {"events",{"id","order_id","action","created_at"}}};
    // Match our own schema, including CHECK/UNIQUE/foreign-key constraints. A file
    // with familiar column names but relaxed constraints is not an application backup.
    QMap<QString,QString> expectedSql;
    for(const auto &sql:migrationSql()) {
        if(!sql.startsWith(QStringLiteral("CREATE TABLE "))) continue;
        const auto name=sql.mid(13,sql.indexOf('(')-13);
        expectedSql.insert(name,sql);
    }
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
    return true;
}
QString WorkshopBridge::checkIntegrity() { return validDb(db_)?QStringLiteral("ok"):QStringLiteral("failed"); }
QVariantList WorkshopBridge::orders() const {
    QSqlQuery q(db_); q.exec(QStringLiteral("SELECT o.*,c.name AS customer,c.phone,d.brand,d.model,d.serial,COALESCE((SELECT SUM(quantity*unit_cents) FROM order_parts WHERE order_id=o.id),0)+o.labor_cents AS total_cents FROM orders o JOIN devices d ON d.id=o.device_id JOIN customers c ON c.id=d.customer_id ORDER BY o.id DESC")); return rows(q);
}
QVariantList WorkshopBridge::inventory() const { QSqlQuery q(db_); q.exec(QStringLiteral("SELECT * FROM parts ORDER BY name"));return rows(q); }
QVariantList WorkshopBridge::events() const { QSqlQuery q(db_); q.exec(QStringLiteral("SELECT * FROM events ORDER BY id DESC LIMIT 300"));return rows(q); }
bool WorkshopBridge::log(int id,const QString &action) { QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO events(order_id,action) VALUES(?,?)"));q.addBindValue(id>0?QVariant(id):QVariant());q.addBindValue(action);return q.exec(); }
bool WorkshopBridge::finish(bool ok) {
    if(!ok) { const auto e=db_.lastError().text(); db_.rollback(); return fail(QStringLiteral("Transaction failed. ")+e); }
    if(!db_.commit()) { db_.rollback(); return fail(db_.lastError().text()); }
    error_.clear(); emit errorChanged(); emit changed(); return true;
}
int WorkshopBridge::createOrder(const QString &customer,const QString &phone,const QString &brand,const QString &model,const QString &serial,const QString &symptom,const QString &tech,const QString &notes) {
    if(!ready_||!textOk(customer)||!textOk(brand)||!textOk(model)||!textOk(symptom,2000)||!optionalTextOk(phone,80)||!optionalTextOk(serial,160)||!optionalTextOk(tech,160)||!optionalTextOk(notes,20000)) { fail(QStringLiteral("Customer, brand, model and symptom are required; text exceeds allowed length."));return -1; }
    if(!db_.transaction()) { fail(db_.lastError().text()); return -1; }
    QSqlQuery q(db_);q.prepare(QStringLiteral("INSERT INTO customers(name,phone) VALUES(?,?)"));q.addBindValue(customer.trimmed());q.addBindValue(phone.trimmed());
    if(!q.exec()) { finish(false);return -1; } const int customerId=q.lastInsertId().toInt();
    q.prepare(QStringLiteral("INSERT INTO devices(customer_id,brand,model,serial) VALUES(?,?,?,?)"));q.addBindValue(customerId);q.addBindValue(brand.trimmed());q.addBindValue(model.trimmed());q.addBindValue(serial.trimmed());
    if(!q.exec()) { finish(false);return -1; } const int deviceId=q.lastInsertId().toInt();
    q.prepare(QStringLiteral("INSERT INTO orders(device_id,symptom,technician,notes) VALUES(?,?,?,?)"));q.addBindValue(deviceId);q.addBindValue(symptom.trimmed());q.addBindValue(tech.trimmed());q.addBindValue(notes);
    if(!q.exec()) { finish(false);return -1; } const int id=q.lastInsertId().toInt();
    return finish(log(id,QStringLiteral("Created")))?id:-1;
}
bool WorkshopBridge::updateOrder(int id,const QString &status,const QString &tech,int cents,const QString &notes) {
    const QStringList states={"Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"};
    if(!ready_||!states.contains(status)||cents<0||cents>100000000||!optionalTextOk(notes,20000)||!optionalTextOk(tech,160)) return fail(QStringLiteral("Invalid order values."));
    if(!db_.transaction()) return fail(db_.lastError().text());
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT COALESCE(SUM(quantity*unit_cents),0) FROM order_parts WHERE order_id=?"));q.addBindValue(id);
    if(!q.exec()||!q.next()||q.value(0).toLongLong()>std::numeric_limits<qint64>::max()-cents) {db_.rollback();return fail(QStringLiteral("Invoice total exceeds the integer monetary limit."));}
    q.finish();q.prepare(QStringLiteral("UPDATE orders SET status=?,technician=?,labor_cents=?,notes=?,updated_at=CURRENT_TIMESTAMP WHERE id=?"));q.addBindValue(status);q.addBindValue(tech);q.addBindValue(cents);q.addBindValue(notes);q.addBindValue(id);
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
    QSqlQuery q(db_);q.prepare(QStringLiteral("SELECT p.sku,p.name,op.quantity,op.unit_cents FROM order_parts op JOIN parts p ON p.id=op.part_id WHERE order_id=?"));q.addBindValue(id);q.exec();result.insert("parts",rows(q));return result;
}
bool WorkshopBridge::exportOrders(const QString &value) {
    const auto target=localPath(value);
    if(!ready_||target.isEmpty()||databaseTarget(target,path_)) return fail(QStringLiteral("Export requires a local file separate from the active database."));
    QSaveFile file(target);if(!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    QString out=QStringLiteral("\xFEFFid,customer,phone,brand,model,serial,symptom,status,technician,total_cents,notes\r\n");
    for(const auto &v:orders()) {const auto m=v.toMap(); QStringList line;for(const auto &key:QStringList{"id","customer","phone","brand","model","serial","symptom","status","technician","total_cents","notes"}) line<<csv(m.value(key).toString());out+=line.join(',')+QStringLiteral("\r\n");}
    const auto bytes=out.toUtf8();if(file.write(bytes)!=bytes.size()||!file.commit()) return fail(file.errorString());error_.clear();emit errorChanged();return true;
}
bool WorkshopBridge::exportInvoice(int id,const QString &value) {
    const auto target=localPath(value);
    if(!ready_||target.isEmpty()||databaseTarget(target,path_)) return fail(QStringLiteral("Export requires a local file separate from the active database."));
    const auto m=invoice(id); if(m.isEmpty()) return fail(QStringLiteral("Order not found."));
    QString html=QStringLiteral("<html><body><h1>NEXVARY MR</h1><h2>Repair service invoice / فاتورة صيانة #%1</h2><p>%2<br>%3 %4<br>%5</p><table width='100%'><tr><th>Part / القطعة</th><th>Quantity</th><th>Unit price</th></tr>").arg(id).arg(m.value("customer").toString().toHtmlEscaped(),m.value("brand").toString().toHtmlEscaped(),m.value("model").toString().toHtmlEscaped(),m.value("status").toString().toHtmlEscaped());
    for(const auto &r:m.value("parts").toList()) { const auto p=r.toMap();html+=QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(p.value("name").toString().toHtmlEscaped()).arg(p.value("quantity").toInt()).arg(money(p.value("unit_cents").toLongLong())); }
    html+=QStringLiteral("</table><p>Labor / أجرة العمل: %1</p><h2>Total / الإجمالي: %2</h2><p>Local currency • No tax calculated / العملة المحلية، دون حساب ضريبة</p><p>%3</p></body></html>").arg(money(m.value("labor_cents").toLongLong())).arg(money(m.value("total_cents").toLongLong())).arg(m.value("notes").toString().toHtmlEscaped());
    QSaveFile output(target);if(!output.open(QIODevice::WriteOnly)) return fail(output.errorString());
    { QPdfWriter pdf(&output);pdf.setPageSize(QPageSize(QPageSize::A4));pdf.setResolution(96);QTextDocument doc;doc.setHtml(html);doc.print(&pdf); }
    if(output.error()!=QFileDevice::NoError||output.size()==0) return fail(QStringLiteral("Invoice PDF generation failed."));
    if(!output.commit()) return fail(output.errorString());
    error_.clear();emit errorChanged();return true;
}
bool WorkshopBridge::backup(const QString &value) {
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
    {auto check=QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),name);check.setDatabaseName(stage);check.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));if(check.open()) { QSqlQuery safe(check);safe.exec(QStringLiteral("PRAGMA trusted_schema=OFF"));valid=validDb(check); }check.close();}
    QSqlDatabase::removeDatabase(name);
    if(!valid) {QFile::remove(stage);return fail(QStringLiteral("Backup failed schema, integrity or foreign key checks."));}
    const QString previous=path_+QStringLiteral(".before-restore-")+QUuid::createUuid().toString(QUuid::Id128)+QStringLiteral(".db");
    if(!backup(previous)) {QFile::remove(stage);return false;}
    db_.close();
    QSaveFile dest(path_);QFile input(stage);bool ok=input.open(QIODevice::ReadOnly)&&dest.open(QIODevice::WriteOnly);
    if(ok) {const auto bytes=input.readAll();ok=dest.write(bytes)==bytes.size()&&dest.commit();}
    input.close();QFile::remove(stage);
    ready_=db_.open()&&validDb(db_);QSqlQuery q(db_);ready_=q.exec(QStringLiteral("PRAGMA foreign_keys=ON"))&&q.exec(QStringLiteral("PRAGMA trusted_schema=OFF"))&&ready_;
    QFile::setPermissions(path_,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
    emit changed();if(ok&&ready_) {error_.clear();emit errorChanged();return true;}return fail(QStringLiteral("Restore failed; original preserved in ")+previous);
}
