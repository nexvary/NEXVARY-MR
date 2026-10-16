#pragma once
#include <QObject>
#include <QString>
#include <QVariantList>
class RepairBridge final : public QObject {
  Q_OBJECT
public:
  explicit RepairBridge(QObject *parent = nullptr) : QObject(parent) {}
  Q_INVOKABLE QVariantList brands() const;
  Q_INVOKABLE QString diagnoseLocalized(const QString &symptom, bool arabic) const;
  Q_INVOKABLE QString diagnose(const QString &symptom) const;
  Q_INVOKABLE bool validCase(const QString &brand, const QString &model, const QString &symptom) const;
};
