#pragma once
#include <QVariantMap>
#include <QStringList>
#include <atomic>
#include <memory>
namespace mr::osint {
using Cancellation=std::shared_ptr<std::atomic_bool>;
struct Request { QString number, executable, executableSha256; QStringList scanners; };
QString normalize(const QString &input);
QStringList supportedScanners();
QVariantMap classifyResponse(const QString &scanner,int status,const QByteArray &body,const QString &transportError={});
class Provider {
public:
 virtual ~Provider()=default;
 virtual QVariantMap run(const Request &,const Cancellation &)=0;
};
class PhoneInfoga final:public Provider {
public:
 QVariantMap run(const Request &,const Cancellation &) override;
};
// Deliberately has no endpoint, credentials or HTTP implementation.
class OsintIndustries final:public Provider {
public:
 QVariantMap run(const Request &,const Cancellation &) override {
  return {{"error","provider-disabled-license-required"},{"provider","osint-industries"}};
 }
};
}
