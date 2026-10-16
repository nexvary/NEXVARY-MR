#include "RepairBridge.hpp"
#include "RepairCore.hpp"
#include <QVariantMap>
QVariantList RepairBridge::brands() const {
  QVariantList result;
  for (const auto &entry : mr::catalog()) {
    QVariantMap row;
    row.insert(QStringLiteral("brand"), QString::fromStdString(entry.brand));
    QStringList families;
    for (const auto &family : entry.families) families.append(QString::fromStdString(family));
    row.insert(QStringLiteral("families"), families);
    result.append(row);
  }
  return result;
}
QString RepairBridge::diagnose(const QString &symptom) const {
  const auto rule = mr::diagnose(symptom.toStdString());
  return QString::fromStdString(rule.nextCheck) + QStringLiteral("\nSafety: ") + QString::fromStdString(rule.safety);
}
bool RepairBridge::validCase(const QString &brand, const QString &model, const QString &symptom) const {
  return mr::validCase(brand.toStdString(), model.toStdString(), symptom.toStdString());
}
