#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QTemporaryDir>
#include <QVariantMap>
#include <memory>
class EvidenceImportRunner final:public QObject {
 Q_OBJECT
public:
 explicit EvidenceImportRunner(QObject *parent=nullptr,const QString &testWorker=QString());
 ~EvidenceImportRunner()override;
 bool busy()const{return active_;}
 bool start(const QByteArray &bytes,const QString &expectedSha256,const QString &adapter);
 void cancel();
signals:
 void changed();
 void completed(const QVariantMap &report,const QString &errorCode);
private:
 void collect();
 void finish(int code,QProcess::ExitStatus status);
 QProcess process_;
 QTimer deadline_;
 std::unique_ptr<QTemporaryDir> directory_;
 QByteArray output_;
 QString hash_,adapter_,terminal_,testWorker_;
 qsizetype sourceSize_=0,errorBytes_=0;
 bool active_=false;
};
