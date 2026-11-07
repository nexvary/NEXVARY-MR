#pragma once
#include "OsintProcessPolicy.hpp"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QThread>
#include <atomic>

namespace mr::osint {
struct LocalRunResult {
    bool completed = false;
    QByteArray standardOutput;
    QString error;
    int exitCode = -1;
};
// Execute only a pinned local executable, without a shell. Caller must provide
// a non-GUI worker thread; this synchronous function must not run on the UI thread.
inline LocalRunResult runPinnedTool(const ProcessLaunchPlan& plan,
                                   const std::atomic_bool& cancelled) {
    LocalRunResult result;
    if (!mayLaunchProcess(plan)) { result.error = "policy-denied"; return result; }
    if (cancelled.load()) { result.error = "cancelled"; return result; }
    const QString path = QString::fromStdString(plan.executablePath);
    QFileInfo info(path);
    if (!info.isAbsolute() || info.isSymLink() || !info.isFile() ||
        !info.isExecutable() || info.size() > 128 * 1024 * 1024) {
        result.error = "untrusted-executable"; return result;
    }
    QFile binary(path);
    if (!binary.open(QIODevice::ReadOnly)) {
        result.error = "executable-unreadable"; return result;
    }
    QCryptographicHash digest(QCryptographicHash::Sha256);
    if (!digest.addData(&binary)) { result.error = "hash-read-failed"; return result; }
    binary.close();
    if (QString::fromLatin1(digest.result().toHex()) !=
        QString::fromStdString(plan.executableSha256)) {
        result.error = "digest-mismatch"; return result;
    }
    QStringList args;
    for (const auto& arg : plan.arguments) args << QString::fromStdString(arg);
    QProcess process;
    process.setProgram(path);
    process.setArguments(args);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(QIODevice::ReadOnly);
    if (!process.waitForStarted(3000)) { result.error = "start-failed"; return result; }
    QElapsedTimer elapsed;
    elapsed.start();
    const auto limit = static_cast<qint64>(plan.maximumOutputBytes);
    while (process.state() != QProcess::NotRunning) {
        if (cancelled.load() || elapsed.elapsed() > static_cast<qint64>(plan.timeoutSeconds) * 1000) {
            result.error = cancelled.load() ? "cancelled" : "timeout";
            process.kill();
            process.waitForFinished(3000);
            return result;
        }
        process.waitForReadyRead(50);
        result.standardOutput += process.readAllStandardOutput();
        // Drain stderr without retaining potentially sensitive output.
        process.readAllStandardError();
        if (result.standardOutput.size() > limit) {
            result.standardOutput.clear();
            result.error = "output-limit";
            process.kill();
            process.waitForFinished(3000);
            return result;
        }
    }
    result.standardOutput += process.readAllStandardOutput();
    process.readAllStandardError();
    if (result.standardOutput.size() > limit) {
        result.standardOutput.clear();
        result.error = "output-limit";
        return result;
    }
    result.exitCode = process.exitCode();
    if (process.exitStatus() != QProcess::NormalExit || result.exitCode != 0) {
        result.error = "tool-failed"; return result;
    }
    result.completed = true;
    return result;
}
} // namespace mr::osint
