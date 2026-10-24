#include "ConnectionDiagnosis.hpp"
#include "UsbDriver.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace mr::diagnostics {
namespace {
bool fresh(const QVariantMap &observation,const QDateTime &now) {
 const auto at=QDateTime::fromString(observation.value("observedAtUtc").toString(),Qt::ISODateWithMs);
 return at.isValid()&&now.isValid()&&at.msecsTo(now)>=0&&at.msecsTo(now)<=connectionFreshnessMs;
}
QString freshness(const QVariantMap &observation,const QDateTime &now) {
 if(observation.value("state").toString()=="running")return "running";
 if(observation.isEmpty())return "not-read";
 return fresh(observation,now)?"current":"stale";
}
}
QVariantMap connectionDiagnosis(const ConnectionSnapshot &s) {
 QVariantList findings,sources,usbNodes;
 auto add=[&](const QString &id,const QString &severity,const QString &confidence,const QString &source,
              const QString &ar,const QString &en,const QString &detailAr,const QString &detailEn,
              const QString &nextAr,const QString &nextEn) {
  if(findings.size()>=32)return;
  findings.append(QVariantMap{{"id",id},{"severity",severity},{"confidence",confidence},{"source",source},
   {"titleAr",ar},{"titleEn",en},{"detailAr",detailAr},{"detailEn",detailEn},{"nextAr",nextAr},{"nextEn",nextEn}});
 };
 bool currentScan=false;QVariantList currentDevices;
 for(const auto &transport:{QStringLiteral("adb"),QStringLiteral("fastboot")}) {
  const auto observation=s.scans.value(transport).toMap();const auto state=observation.value("state").toString();
  const auto age=freshness(observation,s.now);
  sources.append(QVariantMap{{"source",transport},{"state",state.isEmpty()?"not-read":state},{"freshness",age},{"observedAtUtc",observation.value("observedAtUtc")}});
  if(age=="current"&&state=="ok") {
   currentScan=true;
   for(const auto &entry:s.devices)if(entry.toMap().value("transport").toString()==transport)currentDevices.append(entry);
  } else if(state=="unavailable"&&age=="current") {
   add(transport+"-unavailable","warning","reported",transport,"أداة الاكتشاف غير متاحة","Discovery tool unavailable",
       "لم تنفذ قراءة لهذا النقل؛ لا يمكن استنتاج أن الهاتف غير متصل.","This transport was not read; this is not evidence that the phone is absent.",
       "حدد الملف التنفيذي الرسمي من الإعدادات ثم أعد الاكتشاف.","Select the official executable in Settings, then discover again.");
  } else if(state=="failed"&&age=="current") {
   add(transport+"-failed","warning","reported",transport,"لم تكتمل قراءة الاكتشاف","Discovery did not complete",
       "فشل تشغيل الأداة أو ألغيت القراءة أو انتهت مهلتها؛ النتيجة السابقة لا تستخدم كدليل حالي.","The process failed, was cancelled or timed out; its previous rows are not current evidence.",
       "راجع سجل العملية وأعد الفحص؛ لا تبدل تعريفات عشوائيًا.","Review the operation log and retry; avoid speculative driver changes.");
  } else if(age=="stale") {
   add(transport+"-stale","warning","unknown",transport,"قراءة اتصال قديمة","Connection observation expired",
       "انقضت صلاحية القراءة أو وقتها غير صالح؛ يلزم فحص جديد.","The observation expired or its timestamp is invalid; collect a new scan.",
       "أعد اكتشاف هذا النقل.","Discover this transport again.");
  }
 }
 QVariantList selected;
 for(const auto &entry:currentDevices)if(entry.toMap().value("serial").toString()==s.selectedSerial)selected.append(entry);
 if(s.selectedSerial.isEmpty()) {
  add("select-target","info","unknown","selection","لم يحدد جهاز","No target selected",
      "لا تربط خطأ USB بهاتف معين قبل اختيار هدف ومطابقته.","Do not attribute a USB fault to a particular phone before selecting and correlating it.",
      "اكتشف الأجهزة واختر الهدف؛ يمكنك قراءة جرد Windows دون هدف.","Discover devices and select a target; Windows inventory can be read without one.");
 } else if(selected.size()!=1||selected.first().toMap().value("state").toString()=="ambiguous") {
  add("target-unresolved","warning","unknown","selection","الهدف غائب أو ملتبس","Target missing or ambiguous",
      "لا توجد قراءة حديثة وحيدة لهذا الهدف؛ لن يفترض النقل أو التفويض.","No unique current observation identifies this target; transport and authorization stay unresolved.",
      "أعد الاكتشاف وتحقق من كابل واحد وهوية واحدة.","Rediscover and verify one cable and one target identity.");
 } else {
  const auto d=selected.first().toMap();const auto transport=d.value("transport").toString(),state=d.value("state").toString();
  if(transport=="fastboot") {
   const bool profileCurrent=s.profile.value("serial").toString()==s.selectedSerial&&s.profile.value("transport").toString()=="fastboot"&&fresh(QVariantMap{{"observedAtUtc",s.profile.value("verifiedAtUtc")}},s.now);
   const auto mode=profileCurrent?s.profile.value("is-userspace").toString():QString();
   add("fastboot-mode","info","reported","fastboot",mode=="yes"?"وضع fastbootd معلن":mode=="no"?"وضع Bootloader Fastboot معلن":"Fastboot مكتشف؛ النوع غير مقروء",
       mode=="yes"?"fastbootd reported":mode=="no"?"Bootloader Fastboot reported":"Fastboot detected; subtype unread",
       "اكتشاف Fastboot لا يثبت قابلية الكتابة أو التوافق أو فتح القفل.","Fastboot discovery does not establish write permission, compatibility or unlock status.",
       "اقرأ القدرات عبر فحص الخدمات قبل اختيار خدمة.","Read capabilities in Device services before selecting a service.");
  } else if(state=="unauthorized") {
   add("adb-unauthorized","warning","reported","adb","ADB ينتظر التفويض","ADB authorization required",
       "وجود صف unauthorized يثبت أن خادم ADB رصد الحالة؛ ليس دليلًا على تلف التعريف.","An unauthorized row is an ADB server observation, not proof of a faulty driver.",
       "على هاتف يمكنك فتحه، راجع طلب تصحيح USB ووافق فقط لحاسوب موثوق، ثم أعد الفحص.","On a phone you can unlock, review USB debugging approval for this trusted computer, then rescan.");
  } else if(state=="offline") {
   add("adb-offline","warning","reported","adb","ADB غير مستجيب","ADB offline",
       "سبب عدم الاستجابة غير محسوم؛ لا يستنتج تلف الكابل أو الهاتف.","The lack of response has no established physical cause; cable or phone failure is not inferred.",
       "قارن كابل بيانات ومنفذًا معروفين وراجع إعدادات الاتصال، ثم أعد القراءة.","Compare a known data cable and port, review device connection settings, then rescan.");
  } else if(state=="device") {
   add("adb-observed","info","reported","adb","اتصال ADB مفوض مرصود","Authorized ADB connection observed",
       "حالة device لا تثبت اكتمال الإقلاع أو سلامة العتاد أو مطابقة USB المادي.","The device state does not prove completed boot, hardware health or physical USB binding.",
       "اقرأ الهوية والقدرات وتحقق من نتائج الخدمة اللاحقة.","Read identity/capabilities and verify subsequent service results.");
  } else {
   add("adb-mode","warning","reported","adb","وضع ADB محدود أو مختلف","Restricted or different ADB state",
       "الوضع المرصود: "+state,"Observed state: "+state,
       "اتبع مسار الشركة الرسمي لهذا الوضع؛ لا تفترض إتاحة shell أو التثبيت.","Use the OEM-documented workflow for this mode; do not assume shell or installation access.");
  }
 }
 if(currentScan&&currentDevices.isEmpty())add("no-transport-device","warning","unknown","discovery","لم يرصد النقل جهازًا","No transport device observed",
    "القراءة الناجحة الفارغة لا تحدد السبب: قد يكون الكابل أو الوضع أو الإعدادات أو التعريف. لم تختبر سلامة العتاد.","A successful empty scan cannot distinguish cable, mode, settings or driver causes. Hardware health was not tested.",
    "قارن كابل بيانات معروفًا، افحص وضع الهاتف واقرأ جرد USB دون تثبيت تعريفات.","Compare a known data cable, check phone mode and read USB inventory without installing drivers.");
 const auto usbAge=freshness(s.usb,s.now);const auto usbState=s.usb.value("state").toString();
 sources.append(QVariantMap{{"source","windows-pnp"},{"state",usbState.isEmpty()?"not-read":usbState},{"freshness",usbAge},{"observedAtUtc",s.usb.value("observedAtUtc")}});
 QString usbScope="not-correlated";
 if(usbAge=="current"&&usbState=="ok") {
  const auto parsed=mr::usb::inventory(QJsonDocument(QJsonObject::fromVariantMap(s.usb.value("inventory").toMap())).toJson(QJsonDocument::Compact));
  if(!parsed.isEmpty()) {
   const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(parsed)).toJson(QJsonDocument::Compact);
   if(!s.selectedSerial.isEmpty()&&selected.size()==1&&selected.first().toMap().value("state").toString()!="ambiguous") {
    const auto correlated=mr::usb::correlate(bytes,s.selectedSerial);usbScope=correlated.value("state").toString();
    if(usbScope=="serial-correlated")usbNodes=correlated.value("nodes").toList();
   } else if(s.selectedSerial.isEmpty()) {usbScope="host-inventory-only";usbNodes=parsed.value("devices").toList();}
   if(usbScope=="no-match"||usbScope=="ambiguous"||usbScope=="unsupported-serial")add("usb-not-correlated","warning","unknown","windows-pnp","لا توجد مطابقة USB موثوقة للهدف","No reliable target USB correlation",
       "قد تكون هوية USB مختلفة أو الاتصال لاسلكيًا أو ملتبسًا؛ لا تنسب أخطاء أجهزة أخرى لهذا الهاتف.","USB identity may differ, be wireless or ambiguous; other nodes' faults are not attributed to this phone.",
       "راجع هوية Windows والكابل ووضع الاتصال ثم أعد القراءة.","Review Windows identity, cable and connection mode, then recollect.");
   QSet<int> seen;
   for(auto &entry:usbNodes) {
    auto node=entry.toMap();
    const auto match=QRegularExpression("^USB\\\\VID_([0-9A-F]{4})&PID_([0-9A-F]{4})(?:&MI_([0-9A-F]{2}))?\\\\",QRegularExpression::CaseInsensitiveOption).match(node.value("instanceId").toString());
    if(match.hasMatch()){node.insert("vid",match.captured(1).toUpper());node.insert("pid",match.captured(2).toUpper());node.insert("interfaceNumber",match.captured(3).toUpper());}
    entry=node;
    if(!node.contains("problemCode"))continue;
    const int code=node.value("problemCode").toInt();if(code==0||seen.contains(code))continue;seen.insert(code);
    const QMap<int,QStringList> meanings{
     {10,{"التعريف لم يبدأ","Driver failed to start"}},{22,{"جهاز معطل في Windows","Device disabled in Windows"}},
     {28,{"تعريف غير مثبت","Driver not installed"}},{31,{"فشل إضافة التعريف","Driver could not be added"}},
     {43,{"Windows تلقى بلاغ فشل","Windows received a device failure report"}},{52,{"Windows تعذر عليه التحقق من التوقيع","Windows could not verify the driver signature"}}};
    const auto meaning=meanings.value(code,{"رمز مشكلة Windows","Windows problem code"});
    add("pnp-"+QString::number(code),"warning","reported","windows-pnp",meaning[0]+" ("+QString::number(code)+")",meaning[1]+" ("+QString::number(code)+")",
        usbScope=="host-inventory-only"?"رصد على الحاسوب فقط؛ لم ينسب لهاتف محدد.":"رصد على عقدة مترابطة بالرقم التسلسلي؛ لا يثبت سببًا ماديًا أو اعتماد الواجهة.",
        usbScope=="host-inventory-only"?"Host inventory observation only; not attributed to a selected phone.":"Reported on a serial-correlated node; not physical-cause diagnosis or interface certification.",
        "راجع خصائص العقدة ورقم الموديل ومصدر تعريف OEM الرسمي؛ لا تعطل التحقق من التوقيع.","Review that node's properties, exact model and official OEM driver source; keep signature enforcement enabled.");
   }
  } else usbScope="invalid-inventory";
 }
 if(usbAge=="stale")add("usb-stale","warning","unknown","windows-pnp","جرد USB قديم","USB inventory expired","لا تستخدم أخطاء العقد القديمة كحالة حالية.","Old node errors are not current state.","أعد قراءة جرد USB.","Read USB inventory again.");
 return {{"schemaVersion",1},{"assessment","observations-only"},{"evaluatedAtUtc",s.now.toUTC().toString(Qt::ISODateWithMs)},
  {"freshnessLimitMs",connectionFreshnessMs},{"findings",findings},{"sources",sources},{"usbScope",usbScope},{"nodes",usbNodes},
  {"interfaceDescriptorsRead",false},{"interfaceBindingVerified",false},{"physicalCauseVerified",false},{"modelAcceptance","hardware-pending"}};
}
}
