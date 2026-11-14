#include "LawCaseStore.hpp"
#include "CaseVault.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUuid>
QString LawCaseStore::registerOsint(const QString &id,qint64 revision,const QString &device,const QVariantMap &report,const QString &actor) {
 const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(report)).toJson(QJsonDocument::Compact);
 if(report.value("schema")!="nexvary-phone-osint/v1" || report.value("caseId")!=id || QUuid(report.value("operationId").toString()).isNull() || bytes.size()>2*1024*1024){fail("Invalid bounded OSINT report.");return {};}
 const auto sealed=mr::recovery::sealCase(bytes,synthetic_,error_);if(sealed.isEmpty())return {};
 const auto evidence=QUuid::createUuid().toString(QUuid::WithoutBraces);
 const auto now=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
 const bool ok=mutate(id,revision,"phone-osint-report-retained",actor,[&](QVariantMap &record){
  bool found=device.isEmpty();for(const auto &d:record.value("devices").toList())if(d.toMap().value("id")==device)found=true;
  auto rows=record.value("evidence").toList();if(!found||rows.size()>=100)return fail("Select an existing case device within the evidence limit.");
  QSqlQuery q(db_);q.prepare("INSERT INTO law_evidence(id,case_id,payload) VALUES(?,?,?)");q.addBindValue(evidence);q.addBindValue(id);q.addBindValue(sealed);if(!q.exec())return fail("OSINT report persistence failed.");
  rows.append(QVariantMap{{"id",evidence},{"deviceId",device},{"name","phone-osint-"+evidence+".json"},{"description","Unverified phone-number OSINT; operation "+report.value("operationId").toString()},{"custodian",actor},{"size",bytes.size()},{"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},{"registeredAtUtc",now}});
  record.insert("evidence",rows);auto custody=record.value("custody").toList();custody.append(QVariantMap{{"evidenceId",evidence},{"from","PhoneInfoga local REST response"},{"to",actor},{"reason","OSINT report; observations unverified"},{"atUtc",now},{"actor",actor}});record.insert("custody",custody);return true;
 });return ok?evidence:QString();
}
