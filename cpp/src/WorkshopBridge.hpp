#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>
#include <QVariantMap>
class WorkshopBridge final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList orders READ orders NOTIFY changed)
    Q_PROPERTY(QVariantList customers READ customers NOTIFY changed)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
    Q_PROPERTY(QVariantList inventory READ inventory NOTIFY changed)
    Q_PROPERTY(QVariantList events READ events NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
public:
    explicit WorkshopBridge(const QString &path, QObject *parent=nullptr);
    ~WorkshopBridge() override;
    bool ready() const { return ready_; }
    QString error() const { return error_; }
    QString databasePath() const { return path_; }
    QVariantList orders() const;
    QVariantList inventory() const;
    QVariantList events() const;
    QVariantList customers() const;
    QVariantList devices() const;
    Q_INVOKABLE QVariantMap issueDeviceCard(int orderId);
    Q_INVOKABLE QVariantMap deviceCard(int orderId) const;
    Q_INVOKABLE QVariantMap lookupBarcode(const QString &barcode);
    Q_INVOKABLE QVariantList orderHistory(int orderId) const;
    Q_INVOKABLE bool exportIntakeCard(int orderId,const QString &path,bool customerDataApproved);
    Q_INVOKABLE QVariantMap customerProfile(int id) const;
    Q_INVOKABLE QVariantMap deviceProfile(int id) const;
    Q_INVOKABLE bool updateCustomer(int id,const QString &name,const QString &phone);
    Q_INVOKABLE int createOrderForDevice(int deviceId,const QString &symptom,const QString &technician,const QString &notes);
    Q_INVOKABLE int createOrderForCustomer(int customerId,const QString &brand,const QString &model,const QString &serial,const QString &symptom,const QString &technician,const QString &notes);
    Q_INVOKABLE QString issueTrackingToken(int orderId,int days,bool consent);
    Q_INVOKABLE bool revokeTracking(int orderId);
    Q_INVOKABLE bool setPublicMessage(int orderId,const QString &message);
    QVariantMap publicTracking(const QString &token) const;
    QVariantList publicTrackingSnapshot() const;
    Q_INVOKABLE int createOrder(const QString &customer,const QString &phone,const QString &brand,const QString &model,const QString &serial,const QString &symptom,const QString &technician,const QString &notes);
    Q_INVOKABLE bool updateOrder(int id,const QString &status,const QString &technician,int laborCents,const QString &notes);
    Q_INVOKABLE bool addPart(const QString &sku,const QString &name,int quantity,int unitCents);
    Q_INVOKABLE bool restockPart(int partId,int quantity,int unitCents);
    Q_INVOKABLE bool usePart(int orderId,int partId,int quantity);
    Q_INVOKABLE QVariantMap invoice(int orderId) const;
    Q_INVOKABLE bool exportOrders(const QString &path);
    Q_INVOKABLE bool exportInvoice(int id,const QString &path);
    Q_INVOKABLE QVariantList diagnoses(int orderId) const;
    Q_INVOKABLE bool saveDiagnosis(int orderId,const QString &symptom,const QString &guidance,const QString &findings,const QString &evidence,const QString &technician);
    Q_INVOKABLE bool exportDiagnosisReport(int orderId,const QString &path);
    Q_INVOKABLE bool backup(const QString &path);
    Q_INVOKABLE bool restore(const QString &path,bool confirmed);
    Q_INVOKABLE QString checkIntegrity();
    bool recordDiagnostic(const QString &operation,bool success);
    static QString localPath(const QString &value);
signals:
    void changed();
    void errorChanged();
private:
    QVariantMap publicOrderStatus(int id,const QString &message) const;
    bool open();
    bool migrate();
    bool fail(const QString &message);
    bool log(int orderId,const QString &action);
    bool finish(bool ok);
    bool validDb(QSqlDatabase &db,bool allowLegacy=false) const;
    QSqlDatabase db_;
    QString path_, connection_, error_;
    bool ready_=false;
};

