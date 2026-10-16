#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

// Deliberately independent of Qt Quick: validation and session handling are testable headlessly.
class BoardBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool loaded READ loaded NOTIFY boardChanged)
  Q_PROPERTY(QString title READ title NOTIFY boardChanged)
  Q_PROPERTY(QString author READ author NOTIFY boardChanged)
  Q_PROPERTY(QString license READ license NOTIFY boardChanged)
  Q_PROPERTY(QString description READ description NOTIFY boardChanged)
  Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY boardChanged)
  Q_PROPERTY(double boardWidth READ boardWidth NOTIFY boardChanged)
  Q_PROPERTY(double boardHeight READ boardHeight NOTIFY boardChanged)
  Q_PROPERTY(QVariantList components READ components NOTIFY boardChanged)
  Q_PROPERTY(QVariantList nets READ nets NOTIFY boardChanged)
  Q_PROPERTY(QVariantList measurements READ measurements NOTIFY sessionChanged)
  Q_PROPERTY(QVariantList annotations READ annotations NOTIFY sessionChanged)
  Q_PROPERTY(bool dirty READ dirty NOTIFY sessionChanged)
  Q_PROPERTY(QString error READ error NOTIFY errorChanged)
public:
  explicit BoardBridge(QObject *parent = nullptr);
  bool loaded() const { return m_loaded; }
  QString title() const { return m_title; }
  QString author() const { return m_author; }
  QString license() const { return m_license; }
  QString description() const { return m_description; }
  QString sourcePath() const { return m_sourcePath; }
  double boardWidth() const { return m_width; }
  double boardHeight() const { return m_height; }
  QVariantList components() const { return m_components; }
  QVariantList nets() const { return m_nets; }
  QVariantList measurements() const { return m_measurements; }
  QVariantList annotations() const { return m_annotations; }
  bool dirty() const { return m_dirty; }
  QString error() const { return m_error; }

  Q_INVOKABLE bool importBoard(const QString &path, bool rightsConfirmed = false);
  Q_INVOKABLE QVariantList search(const QString &query) const;
  Q_INVOKABLE bool addMeasurement(const QString &net, double value, const QString &note = {});
  Q_INVOKABLE bool addAnnotation(const QString &reference, const QString &text, double x, double y);
  Q_INVOKABLE bool saveSession(const QString &path);
  Q_INVOKABLE bool loadSession(const QString &path);
signals:
  void boardChanged();
  void sessionChanged();
  void errorChanged();
private:
  bool fail(const QString &message);
  void clearError();
  QString comparison(const QString &net, double value) const;
  bool knownReference(const QString &reference) const;
  bool knownNet(const QString &net) const;
  bool m_loaded = false;
  bool m_dirty = false;
  double m_width = 0, m_height = 0;
  QString m_title, m_author, m_license, m_description, m_sourcePath, m_digest, m_error;
  QVariantList m_components, m_nets, m_measurements, m_annotations;
};
