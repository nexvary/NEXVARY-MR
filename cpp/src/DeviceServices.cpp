#include "ToolBridge.hpp"
#include "DeviceProtocol.hpp"
#include "DeviceHealth.hpp"
#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
struct Action { const char *id,*ar,*en,*group,*risk,*blocked; };
const Action actions[]{
 {"probe","قراءة المعلومات والقدرات","Read information and capabilities","common","read",""},
 {"reboot_normal","إعادة التشغيل العادية","Reboot to normal","adb","change",""},
 {"reboot_recovery","إعادة التشغيل إلى Recovery","Reboot to recovery","adb","change",""},
 {"reboot_bootloader","إعادة التشغيل إلى Bootloader","Reboot to bootloader","adb","change",""},
 {"reboot_sideload","بدء Sideload عبر Recovery","Reboot to sideload","adb","change",""},
 {"dark_on","تفعيل الوضع الداكن","Enable dark mode","adb","change",""},
 {"dark_off","إلغاء الوضع الداكن","Disable dark mode","adb","change",""},
 {"languages","إعداد اللغات: العربية والإنجليزية والفرنسية والتركية","Configure Arabic, English, French and Turkish","adb","guided",""},
 {"keyboard","فتح إعدادات لوحات المفاتيح","Open keyboard settings","adb","guided",""},
 {"settings","فتح إعدادات الجهاز للاستعادة الرسمية","Open device settings for official recovery","adb","guided",""},
 {"apn","فتح إعدادات نقاط الوصول APN","Open APN settings","adb","guided",""},
 {"install_apk","تثبيت APK / لوحة مفاتيح من الكمبيوتر","Install APK / keyboard from PC","adb","package",""},
 {"sideload_zip","إرسال ZIP إلى وضع ADB Sideload","Transfer ZIP to ADB sideload","recovery","destructive",""},
 {"fastboot_reboot","Fastboot: إعادة التشغيل العادية","Fastboot: reboot to normal","fastboot","change",""},
 {"fastboot_bootloader","Fastboot: إعادة تشغيل Bootloader","Fastboot: reboot bootloader","fastboot","change",""},
 {"unlock_ability","قراءة أهلية فتح Bootloader الرسمية","Read official bootloader unlock eligibility","fastboot","read",""},
 {"erase_cache","مسح قسم Cache الموجود","Erase existing cache partition","fastboot","destructive",""},
 {"erase_userdata","مسح قسم Userdata — حذف البيانات","Erase userdata — data loss","fastboot","destructive",""},
 {"download","Samsung Download Mode","Samsung download mode","oem","unavailable","Requires an exact-model documented adapter; manufacturer alone does not prove support."},
 {"edl","EDL / Qualcomm 9008 / MTK / Unisoc","EDL / Qualcomm 9008 / MTK / Unisoc","oem","unavailable","No generic EDL/DA/firehose command: requires supported hardware, signed vendor tools and authorization."},
 {"frp","FRP وحسابات Google / Samsung / Huawei","FRP and Google / Samsung / Huawei accounts","restricted","unavailable","Use the account owner's official account recovery or authorized OEM service; no protection bypass adapter."},
 {"lockscreen","قفل الشاشة / الأمان / RMM — مع Root أو بدونه","Screen lock / security / RMM — with or without root","restricted","unavailable","No credential deletion or screen-lock bypass; use owner-authenticated on-device recovery."},
 {"modem","إصلاح الشبكة / Baseband / CP / Modem","Network / baseband / CP / modem repair","oem","unavailable","Requires licensed exact-model firmware and a tested OEM protocol; APN settings are available separately."},
 {"efs","EFS / PIT / Factory Mode / Diag / SPC","EFS / PIT / factory mode / DIAG / SPC","oem","unavailable","No generic EFS erasure, SPC change, root shell or radio-identity modification."},
 {"sim_unlock","فتح SIM وتغيير شركة الاتصالات","SIM unlock and carrier switching","oem","unavailable","Use the carrier's authorized unlock procedure; no universal vendor/chipset unlock command."},
 {"factory_reset","ضبط المصنع","Factory reset","recovery","unavailable","Complete reset through the owner's device settings/official recovery. Userdata erasure is separate and does not remove FRP."},
 {"f2fs","تهيئة Data بصيغة F2FS","Format data as F2FS","fastboot","unavailable","No universal filesystem conversion; requires exact partition layout and a supported recovery/OEM procedure."},
 {"bootloader_unlock","فتح Bootloader قديم / جديد / بكود","Old / new / code-based bootloader unlock","fastboot","unavailable","Year is not a capability. Follow the exact OEM procedure; eligibility reading does not unlock or bypass protection."},
 {"bootloader_relock","إعادة قفل Bootloader","Relock bootloader","fastboot","unavailable","Requires verified signed stock firmware and an exact-model OEM procedure; generic relocking can prevent boot."},
 {"carrier_apps","إزالة تطبيقات شركات الاتصالات","Remove carrier applications","oem","unavailable","No unreviewed package removal lists; model/build-specific dependency review is required."}
};
const Action *definition(const QString &id) { for (const auto &a:actions) if (id==QLatin1String(a.id)) return &a; return nullptr; }
QString vendor(const QString &value) {
 const auto v=value.toLower().simplified();
 if(v=="samsung")return "Samsung";
 if(v=="lg"||v=="lge"||v=="lg electronics")return "LG";
 if(v=="huawei")return "Huawei";
 if(v=="oppo")return "Oppo";
 if(v=="vivo")return "Vivo";
 if(v=="zte")return "ZTE";
 if(v=="xiaomi"||v=="redmi"||v=="poco")return "Xiaomi / Redmi / POCO";
 if(v=="honor")return "Huawei / Honor";
 if(v=="realme")return "Realme";
 if(v=="motorola")return "Motorola";
 if(v=="lenovo")return "Lenovo";
 if(v=="tecno")return "Tecno";
 if(v=="infinix")return "Infinix";
 if(v=="itel")return "Itel";
 if(v=="nokia"||v=="hmd global"||v=="hmd")return "Nokia / HMD";
 return value;
}
bool positivePartition(const QString &value) {
 static const QRegularExpression hex(QStringLiteral("^0x[0-9a-fA-F]{1,16}$"));
 bool ok=false;const auto size=value.mid(2).toULongLong(&ok,16);return hex.match(value).hasMatch()&&ok&&size>0;
}
bool failedText(const QString &text) {
 static const QRegularExpression error(QStringLiteral("(^|[\\r\\n])\\s*(error:|failure|failed|permission denied|securityexception|exception occurred|unknown command)"),QRegularExpression::CaseInsensitiveOption);
 return error.match(text).hasMatch();
}
QString fastbootValue(const QString &key,const QString &text) {
 const QRegularExpression pattern(QStringLiteral("^(?:\\(bootloader\\)\\s*)?")+QRegularExpression::escape(key)+QStringLiteral(":\\s*([^\\r\\n]+)$"));
 // Windows tools emit CRLF; match trimmed individual lines rather than a
 // multiline $ anchor immediately before an unconsumed carriage return.
 for(const auto &line:text.split(QLatin1Char('\n'))){const auto match=pattern.match(line.trimmed());if(match.hasMatch())return match.captured(1).trimmed().left(240);}
 return {};
}
QVariantMap preparePackage(const QString &input,const QString &destination,bool zip,const QString &expected,const std::shared_ptr<std::atomic_bool> &cancel) {
 auto fail=[](const QString &message){return QVariantMap{{"ok",false},{"error",message}};};
 QString path=input;
 if(input.startsWith("file:",Qt::CaseInsensitive)){const QUrl url(input,QUrl::StrictMode);if(!url.isLocalFile()||!url.host().isEmpty()||url.hasQuery()||url.hasFragment())return fail("Choose a local package, not a network URL.");path=url.toLocalFile();}
 if(!QDir::isAbsolutePath(path)||path.startsWith("//")||path.startsWith("\\\\"))return fail("Choose an absolute local package path.");
 const QFileInfo before(path);const qint64 limit=zip?2LL*1024*1024*1024:512LL*1024*1024;
 if(!before.isFile()||before.isSymLink()||before.size()<4||before.size()>limit||before.suffix().compare(zip?"zip":"apk",Qt::CaseInsensitive)!=0)return fail("Invalid package type, symlink or size (APK <=512 MiB; ZIP <=2 GiB).");
 QFile source(path),copy(destination);if(!source.open(QIODevice::ReadOnly)||source.read(4)!=QByteArray("PK\003\004",4)||!source.seek(0)||!copy.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("Package is unreadable or lacks a ZIP/APK header; no device command was sent.");
 QElapsedTimer deadline;deadline.start();
 auto digest=[&](QFile &file,bool write){QCryptographicHash hash(QCryptographicHash::Sha256);qint64 read=0;while(!file.atEnd()){
  if(cancel->load()||deadline.elapsed()>120000)return QByteArray();
  const auto bytes=file.read(256*1024);if(bytes.isEmpty()||read+bytes.size()>limit)return QByteArray();read+=bytes.size();hash.addData(bytes);if(write&&copy.write(bytes)!=bytes.size())return QByteArray();
 }return file.error()==QFileDevice::NoError&&read==before.size()?hash.result().toHex():QByteArray();};
 const auto first=digest(source,false);if(first.isEmpty()||!source.seek(0))return fail("Package read cancelled, interrupted or timed out.");
 const auto copied=digest(source,true);if(!copy.flush())return fail("Could not flush the private package copy.");copy.close();
 if(!source.seek(0)){return fail("Source became unavailable.");}
 const auto after=digest(source,false);
 QFile staged(destination);if(!staged.open(QIODevice::ReadOnly))return fail("Private copy became unavailable.");const auto verified=digest(staged,false);staged.close();
 const QFileInfo now(path);
 if(cancel->load()||first!=copied||first!=after||first!=verified||before.size()!=now.size()||before.lastModified()!=now.lastModified())return fail("Package changed or became unavailable; no device write was started.");
 if(!expected.isEmpty()&&QString::fromLatin1(first)!=expected)return fail("SHA-256 does not match the reviewed package source.");
 return {{"ok",true},{"sha256",QString::fromLatin1(first)},{"bytes",before.size()},{"path",destination}};
}
}

QVariantMap ToolBridge::selectedServiceDevice() const {
 QVariantMap found;
 for(const auto &row:devices_){const auto item=row.toMap();if(item.value("serial").toString()!=selectedSerial_)continue;if(!found.isEmpty())return {};found=item;}
 return found;
}
void ToolBridge::initServices() {
 serviceExpiry_.setSingleShot(true);serviceExpiry_.setInterval(60000);
 connect(&serviceExpiry_,&QTimer::timeout,this,&ToolBridge::serviceInvalidate);
 connect(this,&ToolBridge::devicesChanged,this,&ToolBridge::serviceChanged);
 connect(this,&ToolBridge::selectedSerialChanged,this,&ToolBridge::serviceChanged);
 connect(&serviceCopy_,&QFutureWatcher<QVariantMap>::finished,this,[this]{
  const auto result=serviceCopy_.result();
  if(!serviceRunning_)return;
  if(serviceCancel_->load()||!result.value("ok").toBool()){serviceComplete(false,serviceCancel_->load()?QStringLiteral("Cancelled before device write."):result.value("error").toString());return;}
  serviceScratch_.insert("package",result);
  serviceSteps_.enqueue({mr::service::adapter(serviceTool_)->discovery(),"state"});serviceNext();
 });
}
void ToolBridge::shutdownServices() {
 serviceExpiry_.stop();serviceCopy_.disconnect(this);if(serviceCancel_)serviceCancel_->store(true);serviceCopy_.waitForFinished();serviceRunning_=false;
}
void ToolBridge::serviceInvalidate(){serviceProfile_.clear();serviceExpiry_.stop();emit serviceChanged();}
bool ToolBridge::serviceAvailable(const QString &id,QString *reason) const {
 auto no=[reason](const QString &s){if(reason)*reason=s;return false;};
 const auto *action=definition(id);if(!action)return no("Unknown service.");
 if(*action->blocked)return no(QString::fromUtf8(action->blocked));
 if(busy())return no("Wait for the current operation or cancel it.");
 const auto device=selectedServiceDevice();const auto transport=device.value("transport").toString(),state=device.value("state").toString();
 if(device.isEmpty()||!isValidSerial(selectedSerial_))return no("Select one unambiguous device from current discovery results.");
 if((transport=="adb"&&state!="device"&&state!="sideload")||(transport=="fastboot"&&state!="fastboot"))return no("Authorize the device on its screen and rescan; offline/unauthorized modes are not usable.");
 if(id=="probe")return state!="sideload"?true:no("Sideload exposes transfer only; normal ADB properties are unavailable.");
 if(id=="sideload_zip")return transport=="adb"&&state=="sideload"?true:no("Manually enter ADB sideload in a compatible recovery, then rescan. TWRP must already be installed for the exact model.");
 if(serviceProfile_.value("serial").toString()!=selectedSerial_||serviceProfile_.value("transport").toString()!=transport)return no("Read capabilities first; the snapshot expires after 60 seconds or a discovery/selection change.");
 const bool fast=id.startsWith("fastboot_")||id.startsWith("erase_")||id=="unlock_ability";
 if(fast!=(transport=="fastboot"))return no("This operation requires the other device transport; rescan in the requested mode.");
 if(transport=="adb"&&state!="device")return no("Requires authorized normal Android ADB, not recovery/sideload.");
 if((id=="dark_on"||id=="dark_off")&&!serviceProfile_.value("night").toBool())return no("Android 10+ and a responding cmd uimode night interface are required.");
 if(!mr::service::settingsIntent(id).isEmpty()&&!serviceProfile_.value("intent:"+id).toBool())return no("No supported Settings activity was resolved on this firmware (probe requires Android 7+).");
 if(id.startsWith("erase_")){
  const auto part=id=="erase_cache"?QStringLiteral("cache"):QStringLiteral("userdata");
  if(serviceProfile_.value("unlocked").toString()!="yes"||!positivePartition(serviceProfile_.value("partition-size:"+part).toString()))return no("Requires a reported unlocked bootloader and a nonzero existing partition; no guessing a partition layout.");
 }
 return true;
}
QVariantList ToolBridge::serviceActions() const {
 QVariantList rows;
 for(const auto &a:actions){QString reason;const bool available=serviceAvailable(QString::fromLatin1(a.id),&reason);rows.append(QVariantMap{{"id",a.id},{"ar",QString::fromUtf8(a.ar)},{"en",a.en},{"group",a.group},{"risk",a.risk},{"available",available},{"reason",reason},{"implemented",!*a.blocked}});}
 return rows;
}
QVariantList ToolBridge::requestedModels() {
 const QStringList oppo{"A30","A33","A33F","A33M","A37F","A51F","A51W","CPH1607 R9S","CPH1611 R9s Plus","CPH1613 F3 Plus","CPH1701 A57","CPH1707 R11","F1f","R7 (R7g)","R7 Plus F","R7 Plus M","R7sf","R7sm","R7s Plus","R9 PlusmA","R9 PlusTM","R9 PlustmA","R9st","X9079"};
 const QStringList vivo{"V3 MAX PD1523F","V3 PD1524F","V5 PLUS PD1624F","V7 PD1718F","V7 PLUS PD1708F","V9 PD1730F","V9 YOUTH PD1730BF","X5 PRO PD1421F","X9 PD1616","Y51-Y51L PD1510F","Y53 PD1628F","Y55S PD1613BF","Y55-Y55L PD1613F","Y65 PD1621BF","Y71 PD1731F","XPLAY5 PD1522A","Y35 PD1502F","Y37 PD1503F"};
 QVariantList result;for(const auto &name:oppo)result.append(QVariantMap{{"brand","Oppo"},{"model",name},{"validated",false}});for(const auto &name:vivo)result.append(QVariantMap{{"brand","Vivo"},{"model",name},{"validated",false}});return result;
}
bool ToolBridge::startService(const QString &action,const QString &expectedSerial,const QVariantMap &approval) {
 QString reason;if(expectedSerial!=selectedSerial_||!serviceAvailable(action,&reason)){setStatus(reason.isEmpty()?QStringLiteral("Device selection changed; review the target again."):reason);return false;}
 const QSet<QString> allowed{"authorized","destructive","challenge","file","sha256","compatible","replace"};
 for(auto it=approval.cbegin();it!=approval.cend();++it)if(!allowed.contains(it.key())){setStatus("Unexpected service options.");return false;}
 if(!approval.value("authorized").toBool()){setStatus("Explicit owner authorization is required.");return false;}
 const auto *def=definition(action);const bool destructive=QLatin1String(def->risk)=="destructive";
 if(action!="probe"&&(approval.value("challenge").toString()!=expectedSerial||(destructive&&!approval.value("destructive").toBool()))){setStatus("Review the exact device serial and explicitly confirm possible data loss.");return false;}
 const auto hash=approval.value("sha256").toString().trimmed().toLower();static const QRegularExpression hex("^[a-f0-9]{64}$");
 if((!hash.isEmpty()&&!hex.match(hash).hasMatch())||(action=="sideload_zip"&&(!hex.match(hash).hasMatch()||!approval.value("compatible").toBool()))){setStatus("Sideload needs a reviewed SHA-256 and explicit exact-model/recovery compatibility confirmation.");return false;}
 serviceRunning_=true;serviceAction_=action;serviceTarget_=expectedSerial;serviceTool_=selectedServiceDevice().value("transport").toString();serviceApproval_=approval;serviceScratch_.clear();serviceSteps_.clear();serviceOutput_.clear();serviceCancel_=std::make_shared<std::atomic_bool>(false);serviceExpiry_.stop();emit busyChanged();emit serviceChanged();
 if(action=="install_apk"||action=="sideload_zip"){
  serviceDirectory_=std::make_unique<QTemporaryDir>();if(!serviceDirectory_->isValid()){serviceComplete(false,"Could not create a private package workspace.");return false;}
  servicePhase_="preparing";emit serviceChanged();const bool zip=action=="sideload_zip";
  const auto destination=serviceDirectory_->filePath(zip?"package.zip":"package.apk"),input=approval.value("file").toString();
  const auto cancel=serviceCancel_;
  serviceCopy_.setFuture(QtConcurrent::run([input,destination,zip,hash,cancel]{return preparePackage(input,destination,zip,hash,cancel);}));return true;
 }
 serviceSteps_.enqueue({mr::service::adapter(serviceTool_)->discovery(),"state"});serviceNext();return true;
}
void ToolBridge::serviceNext() {
 if(!serviceRunning_)return;
 if(serviceCancel_->load()||selectedSerial_!=serviceTarget_){serviceComplete(false,"Cancelled or device selection changed; verify any operation already started on the phone.");return;}
 if(serviceSteps_.isEmpty()){serviceDispatch();return;}
 serviceCurrent_=serviceSteps_.dequeue();servicePhase_=serviceCurrent_.key=="state"?"connection":serviceCurrent_.key=="identity"?"identity":serviceCurrent_.key=="execute"?"applying":serviceCurrent_.key=="verify-night"?"verifying":"capability";emit serviceChanged();
 if(!runReadOnly(serviceTool_,serviceCurrent_.arguments,Operation::Service))serviceComplete(false,"Device tool is missing or unavailable.");
 else {timeoutTimer_.setInterval(serviceCurrent_.key=="execute"&&(serviceAction_=="install_apk"||serviceAction_=="sideload_zip")?600000:12000);timeoutTimer_.start();}
}
void ToolBridge::serviceAfterState() {
 const auto *protocol=mr::service::adapter(serviceTool_);
 if(!protocol){serviceComplete(false,"Unsupported protocol adapter.");return;}
 for(const auto &query:protocol->queries(serviceAction_))serviceSteps_.enqueue({protocol->addressed(serviceTarget_,query.arguments),query.key,query.optional});
 serviceNext();
}
void ToolBridge::serviceStepFinished(bool success,const QString &out,const QString &err) {
 if(!serviceRunning_)return;
 const auto text=(out+"\n"+err).trimmed();const auto key=serviceCurrent_.key;
 if(serviceCancel_->load()||!terminalReason_.isEmpty()){serviceComplete(false,terminalReason_.isEmpty()?QStringLiteral("Cancelled; verify device state."):terminalReason_);return;}
 success=success&&!failedText(text);
 if(!success&&!serviceCurrent_.optional){serviceComplete(false,"Device command rejected, disconnected or failed; no further queued commands were sent.\n"+text.left(2048));return;}
 if(key=="state"){
  // Discovery's legacy display parser deduplicates rows. Execution must reject
  // repeated serials rather than silently selecting one physical transport.
  int occurrences=0;
  for(const auto &line:out.split(QLatin1Char('\n'))){const auto fields=line.trimmed().split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts);if(fields.size()>=2&&fields[0]==serviceTarget_)++occurrences;}
  if(occurrences!=1){serviceComplete(false,"Target is missing or its serial is ambiguous; no device write was sent.");return;}
  const auto list=serviceTool_=="adb"?parseAdbDevices(out):parseFastbootDevices(out);bool found=false;
  for(const auto &row:list){const auto d=row.toMap();if(d.value("serial").toString()==serviceTarget_&&d.value("state").toString()==(serviceTool_=="fastboot"?"fastboot":serviceAction_=="sideload_zip"?"sideload":"device"))found=true;}
  if(!found){serviceComplete(false,"Target is disconnected, unauthorized or in a different mode; no device write was sent.");return;}serviceAfterState();return;
 }
 if(key=="identity"){
  const auto properties=parseDeviceProperties(out);
  if(properties.value("ro.product.manufacturer").toString().isEmpty()||properties.value("ro.product.model").toString().isEmpty()){serviceComplete(false,"Device identity could not be read completely.");return;}
  if(serviceAction_!="probe")for(const auto &field:{"ro.product.manufacturer","ro.product.model","ro.build.fingerprint"})if(properties.value(field)!=serviceProfile_.value(field)){serviceComplete(false,"Device identity/build changed since capability review; read capabilities again.");return;}
  for(auto it=properties.cbegin();it!=properties.cend();++it)serviceScratch_.insert(it.key(),it.value());
  serviceScratch_.insert("vendor",vendor(properties.value("ro.product.manufacturer").toString()));
 }
 if(key=="night")serviceScratch_.insert("night",success&&serviceScratch_.value("ro.build.version.sdk").toInt()>=29&&text.contains("night")&&text.contains("yes")&&text.contains("no"));
 if(key.startsWith("intent:")){static const QRegularExpression component(QStringLiteral("(?:^|[\\r\\n])[A-Za-z0-9_.]+/[A-Za-z0-9_.$]+\\s*$"));serviceScratch_.insert(key,success&&serviceScratch_.value("ro.build.version.sdk").toInt()>=24&&component.match(text).hasMatch());}
 if(key.startsWith("var:"))serviceScratch_.insert(key.mid(4),success?fastbootValue(key.mid(4),text):QString());
 if(key.startsWith("observation:"))serviceScratch_.insert(key,success?text.left(16384):QStringLiteral("Unavailable with current device permissions."));
 if(key=="execute"){
  if(serviceAction_=="install_apk"&&!QRegularExpression(QStringLiteral("(?:^|[\\r\\n])Success(?:$|[\\r\\n])")).match(text).hasMatch()){serviceComplete(false,"Package Manager did not confirm installation.\n"+text.left(2048));return;}
  serviceOutput_=text.left(8192);emit serviceChanged();
 }
 if(key=="verify-night"){
  const auto expected=serviceAction_=="dark_on"?QStringLiteral("yes"):QStringLiteral("no");
  if(!QRegularExpression("Night mode:\\s*"+expected+"(?:$|\\s)",QRegularExpression::CaseInsensitiveOption).match(text).hasMatch()){serviceComplete(false,"Dark-mode readback did not confirm the requested setting.");return;}
 }
 serviceNext();
}
void ToolBridge::serviceDispatch() {
 const QStringList prefix{"-s",serviceTarget_};
 if(serviceCurrent_.key=="execute"||serviceCurrent_.key=="verify-night"){
  const auto message=serviceAction_=="sideload_zip"?QStringLiteral("ADB reports transfer completion. Check recovery's installation/signature/compatibility result on the device; this is not proof of a successful flash."):serviceAction_.startsWith("reboot_")||serviceAction_.startsWith("fastboot_")?QStringLiteral("Reboot request accepted; rescan and verify the new device mode."):QStringLiteral("Device command completed; verify the result on the phone.");serviceComplete(true,message);return;
 }
 if(serviceAction_=="probe"){
  if(serviceTool_=="fastboot"&&serviceScratch_.value("product").toString().isEmpty()){serviceComplete(false,"Fastboot did not report a product identity.");return;}
  serviceProfile_=serviceScratch_;serviceProfile_.insert("serial",serviceTarget_);serviceProfile_.insert("transport",serviceTool_);serviceProfile_.insert("verifiedAtUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
  if(serviceTool_=="adb"){deviceInfo_=serviceScratch_;deviceInfo_.insert("serial",serviceTarget_);emit deviceInfoChanged();}
  serviceProfile_.insert("diagnostics",mr::service::deviceHealth(serviceProfile_));
  serviceExpiry_.start();serviceComplete(true,"Capabilities read. Reported support is not physical-device/model acceptance; snapshot expires in 60 seconds.");return;
 }
 const auto *protocol=mr::service::adapter(serviceTool_);
 if(!protocol){serviceComplete(false,"Unsupported protocol adapter.");return;}
 if(serviceTool_=="fastboot"){
  if(serviceScratch_.value("product")!=serviceProfile_.value("product")){serviceComplete(false,"Fastboot product changed; read capabilities again.");return;}
  if(serviceAction_.startsWith("erase_")){
   const auto part=serviceAction_=="erase_cache"?QStringLiteral("cache"):QStringLiteral("userdata");
   if(serviceScratch_.value("unlocked").toString()!="yes"||!positivePartition(serviceScratch_.value("partition-size:"+part).toString())){serviceComplete(false,"Fresh bootloader/partition checks rejected erasure.");return;}
  }
 }
 const auto package=serviceScratch_.value("package").toMap();
 if((serviceAction_=="install_apk"||serviceAction_=="sideload_zip")&&!package.value("ok").toBool()){serviceComplete(false,"No verified private package copy.");return;}
 const auto command=protocol->command(serviceAction_,package.value("path").toString(),serviceApproval_.value("replace").toBool());
 if(command.isEmpty()){serviceComplete(false,"No implemented command for this action.");return;}
 serviceSteps_.enqueue({prefix+command,"execute"});
 if(serviceAction_=="dark_on"||serviceAction_=="dark_off"){serviceSteps_.enqueue({prefix+QStringList{"shell","cmd","uimode","night"},"verify-night"});}
 serviceNext();
}
void ToolBridge::serviceComplete(bool success,const QString &reason) {
 const auto action=serviceAction_,transport=serviceTool_;
 const auto hash=serviceScratch_.value("package").toMap().value("sha256").toString();
 serviceRunning_=false;active_=false;operation_=Operation::None;timeoutTimer_.stop();timeoutTimer_.setInterval(12000);serviceSteps_.clear();serviceDirectory_.reset();
 servicePhase_=success?"complete":"failed";serviceOutput_=(reason+(serviceOutput_.isEmpty()?QString():"\n\n"+serviceOutput_)).left(12288);
 if(!hash.isEmpty())serviceOutput_+="\nSHA-256: "+hash;
 serviceHistory_.prepend(QVariantMap{{"serial",serviceTarget_},{"transport",transport},{"result",serviceOutput_},{"atUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"action",action},{"success",success},{"sha256",hash},{"ownerAuthorizationDeclared",serviceApproval_.value("authorized").toBool()}});if(serviceHistory_.size()>100)serviceHistory_.removeLast();
 if(action!="probe"||!success)serviceInvalidate();
 if(!success){deviceInfo_.clear();logcat_.clear();emit deviceInfoChanged();emit logcatChanged();}
 if(success&&(action.startsWith("reboot_")||action.startsWith("fastboot_"))){QVariantList retained;for(const auto &row:devices_){const auto d=row.toMap();if(d.value("transport").toString()==transport&&d.value("serial").toString()!=serviceTarget_)retained.append(row);}replaceDevices(retained,transport);}
 setStatus(reason);emit serviceChanged();emit busyChanged();emit diagnosticCompleted("service:"+action,success);emit operationFinished(success);
}
bool ToolBridge::exportServiceReport(const QString &input,bool approved) {
 const QUrl url(input);const QString path=input.startsWith("file:")?url.toLocalFile():input;
 if(!approved||busy()||serviceHistory_.isEmpty()||!QDir::isAbsolutePath(path)||path.startsWith("//")||path.startsWith("\\\\")||(!url.host().isEmpty()&&url.isLocalFile())||QFileInfo::exists(path)){setStatus("Approve a plaintext report and choose a new local filename.");return false;}
 const auto document=QJsonObject{{"schemaVersion",2},{"generatedAtUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"profile",QJsonObject::fromVariantMap(serviceProfile_)},{"operations",QJsonArray::fromVariantList(serviceHistory_)},{"lastResult",serviceOutput_},{"hardwareValidation","Pending Hardware Validation; observed capabilities are not model certification"},{"ownership","Operator declaration; not independent ownership verification"},{"usbDriverBinding","Transport interface binding not independently verified; see usbDriverCorrelation for optional Windows serial ancestry evidence"},{"usbDriverCorrelation",QJsonObject::fromVariantMap(driverBinding_)}};
 const auto bytes=QJsonDocument(document).toJson();QFile file(path);
 if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)){setStatus("Could not create the report.");return false;}
 file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);const bool ok=file.write(bytes)==bytes.size()&&file.flush();file.close();if(!ok)QFile::remove(path);setStatus(ok?"Service report exported; review personal identifiers before sharing.":"Report write failed.");return ok;
}
