#pragma once
#include <QJsonObject>
#include <QVariantList>
namespace mr::forensic {
// A local consistency chain, not a signature or independently anchored custody proof.
void appendAudit(QJsonObject &document, const QString &operation, const QString &outcome);
bool validateHistory(const QJsonObject &document, QString &error);
void upgradeLegacy(QJsonObject &document);
QVariantList timeline(const QJsonObject &document);
QString htmlReport(const QJsonObject &document, bool arabic);
}
