#pragma once
#include <QProcess>
#include <QTemporaryDir>
#include <QVariantMap>
#include <memory>

// Ownership and bounded shutdown, not a process/network security sandbox.
// Linux workers receive /dev/null stdin; the supervisor owns the control pipe.
class OwnedProcess final : public QProcess {
public:
 explicit OwnedProcess(QObject *parent=nullptr);
 ~OwnedProcess() override;
 void setProgram(const QString &program){program_=program;}
 void setArguments(const QStringList &arguments){arguments_=arguments;}
 void start(OpenMode mode=ReadWrite);
 void start(const QString &program,const QStringList &arguments={},OpenMode mode=ReadWrite){setProgram(program);setArguments(arguments);start(mode);}
 void stop(const QString &reason);
 void kill(){stop(QStringLiteral("forced-stop"));}
 void terminate(){stop(QStringLiteral("termination-request"));}
 void closeWriteChannel(){} // Do not close the supervisor's death/control pipe.
 QVariantMap cancellationReceipt()const{return receipt_;}
 bool cleanupComplete()const{return receipt_.value("cleanupComplete").toBool();}
private:
 void collectReceipt();
 struct Native;
 std::unique_ptr<Native> native_;
 std::unique_ptr<QTemporaryDir> directory_;
 QString program_,reason_;
 QStringList arguments_;
 QVariantMap receipt_;
};
