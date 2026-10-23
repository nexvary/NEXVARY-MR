#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVariantMap>

// Owned, fixed-argument Laboratory screen-view process. No detached launch,
// recording, control, arbitrary flags or automatic reconnection to another phone.
class MirrorSession final:public QObject {
 Q_OBJECT
public:
 explicit MirrorSession(QObject *parent=nullptr);
 ~MirrorSession() override;
 bool active()const{return active_;}
 QString state()const{return state_;}
 QString status()const{return status_;}
 QString output()const{return QString::fromUtf8(viewOutput_);}
 QString version()const{return version_;}
 bool start(const QString &scrcpy,const QString &adb,const QString &serial,
            const QVariantMap &identity,bool ownerApproved,bool componentApproved);
 void stop(const QString &reason=QStringLiteral("Screen-view process stopped by operator."));
 void shutdown();
 static bool supportedVersion(const QString &output);
signals:
 void changed();
 void finished(bool processStoppedWithoutError);
private:
 void query(const QStringList &arguments,const QString &state);
 void collectCheck();
 void checkFinished(int code,QProcess::ExitStatus exit);
 void collectView();
 void launchView();
 void end(const QString &reason,bool failed);
 void finalize();
 void message(const QString &text);
 QProcess check_,viewer_;
 QTimer deadline_,monitor_,kill_;
 QByteArray checkOutput_,viewOutput_;
 QString scrcpy_,adb_,serial_,state_=QStringLiteral("idle"),status_,version_;
 QVariantMap identity_;
 bool active_=false,failed_=false;
};
