#include "ServiceCatalog.hpp"
#include "WorkshopBridge.hpp"
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QRegularExpression>
#include <QSet>
#include <functional>
namespace {
bool text(const QString &s,int max){return !s.trimmed().isEmpty()&&s.size()<=max&&!s.contains(QChar::Null);}
QString key(const QString &s){return s.trimmed().toCaseFolded();}
}
ServiceCatalog::ServiceCatalog(const QString &storage,QObject *parent):QObject(parent),storage_(storage){
 QFile f(storage);if(f.size()<=2*1024*1024&&f.open(QIODevice::ReadOnly)){
  const auto parsed=QJsonDocument::fromJson(f.readAll());const auto list=parsed.array().toVariantList();
  if(parsed.isArray()&&list.size()<=200){bool valid=true;for(const auto &g:list)valid=valid&&validGuide(g.toMap());if(valid)guides_=list;else error_="Stored guides failed validation.";}
 }
}
QVariantList ServiceCatalog::drivers() const {
 // Official landing pages only; no driver binary is redistributed or silently installed.
 return {
  QVariantMap{{"brand","Samsung"},{"url","https://developer.samsung.com/android-usb-driver"},{"kind","Official Samsung Android USB driver page"},{"availablePackage",true}},
  QVariantMap{{"brand","Huawei"},{"url","https://consumer.huawei.com/en/support/hisuite/"},{"kind","Official HiSuite/support page; verify model/OS requirements"},{"availablePackage",true}},
  QVariantMap{{"brand","OPPO"},{"url","https://support.oppo.com/en/"},{"kind","Official support; model-specific Windows driver availability must be checked"},{"availablePackage",false}},
  QVariantMap{{"brand","vivo"},{"url","https://www.vivo.com/en/support"},{"kind","Official support; model-specific Windows driver availability must be checked"},{"availablePackage",false}},
  QVariantMap{{"brand","LG"},{"url","https://www.lg.com/us/support?popup=software"},{"kind","Official legacy product support; availability depends on model/region"},{"availablePackage",false}},
  QVariantMap{{"brand","ZTE"},{"url","https://support.ztedevices.com/en-gl/"},{"kind","Official product support; model-specific Windows driver availability must be checked"},{"availablePackage",false}}
 };
}
bool ServiceCatalog::validGuide(const QVariantMap &g){
 if(g.value("schemaVersion").toInt()!=1||!text(g.value("brand").toString(),240)||!text(g.value("model").toString(),240)||!text(g.value("symptom").toString(),240)||!text(g.value("license").toString(),200))return false;
 const QUrl source(g.value("sourceUrl").toString());if(source.scheme()!="https"||source.host().isEmpty()||!source.userInfo().isEmpty()||source.toString().size()>2000)return false;
 const auto list=g.value("nodes").toList();if(list.isEmpty()||list.size()>64)return false;QMap<QString,QVariantMap> nodes;static const QRegularExpression id(QStringLiteral("^[a-z0-9_-]{1,40}$"));
 for(const auto &v:list){const auto n=v.toMap();const auto name=n.value("id").toString();if(!id.match(name).hasMatch()||nodes.contains(name))return false;
  const bool leaf=n.value("terminal").toBool();if(!text(n.value("ar").toString(),2000)||!text(n.value("en").toString(),2000))return false;
  if(leaf){if(n.contains("yes")||n.contains("no"))return false;}else if(!id.match(n.value("yes").toString()).hasMatch()||!id.match(n.value("no").toString()).hasMatch())return false;
  nodes.insert(name,n);
 }
 QSet<QString> active,seen;std::function<bool(const QString&)> visit=[&](const QString &name){if(active.contains(name)||!nodes.contains(name))return false;if(seen.contains(name))return true;active.insert(name);const auto n=nodes.value(name);if(!n.value("terminal").toBool()&&(!visit(n.value("yes").toString())||!visit(n.value("no").toString())))return false;active.remove(name);seen.insert(name);return true;};
 return visit(g.value("root").toString())&&seen.size()==nodes.size();
}
bool ServiceCatalog::importGuide(const QString &value,bool reviewed){
 const auto path=WorkshopBridge::localPath(value);QFile f(path);
 if(!reviewed||!QFileInfo(path).isFile()||QFileInfo(path).isSymLink()||f.size()>128*1024||!f.open(QIODevice::ReadOnly)){error_="Choose a bounded local guide and confirm source/license review.";emit changed();return false;}
 const auto doc=QJsonDocument::fromJson(f.readAll());auto guide=doc.object().toVariantMap();
 if(!doc.isObject()||!validGuide(guide)){error_="Guide must identify exact brand/model/symptom, HTTPS source, license and a bounded acyclic yes/no tree.";emit changed();return false;}
 // Keep only the documented schema; arbitrary imported properties are never interpreted.
 QVariantMap filtered;for(const auto &k:QStringList{"schemaVersion","brand","model","symptom","sourceUrl","license","root","nodes"})filtered.insert(k,guide.value(k));guide=filtered;
 auto updated=guides_;bool replaced=false;for(auto &v:updated){const auto g=v.toMap();if(key(g.value("brand").toString())==key(guide.value("brand").toString())&&key(g.value("model").toString())==key(guide.value("model").toString())&&key(g.value("symptom").toString())==key(guide.value("symptom").toString())){v=guide;replaced=true;break;}}
 if(!replaced){updated.append(guide);}
 if(updated.size()>200){error_="Guide library limit is 200 model/symptom entries.";emit changed();return false;}
 QSaveFile out(storage_);const auto bytes=QJsonDocument::fromVariant(updated).toJson();if(!out.open(QIODevice::WriteOnly)||out.write(bytes)!=bytes.size()||!out.commit()){error_="Cannot save approved guide library.";emit changed();return false;}
 guides_=updated;error_.clear();emit changed();return true;
}
QVariantMap ServiceCatalog::tree(const QString &brand,const QString &model,const QString &symptom,bool arabic) const {
 if(!text(brand,240)||!text(model,240)||!text(symptom,240))return {};
 for(const auto &v:guides_){auto g=v.toMap();if(key(g.value("brand").toString())==key(brand)&&key(g.value("model").toString())==key(model)&&key(g.value("symptom").toString())==key(symptom)){g.insert("scope",arabic?QStringLiteral("دليل خاص بالموديل أدخله المشغل؛ راجع المصدر والترخيص. لم يعتمد كدليل OEM تلقائيًا."):QStringLiteral("Operator-imported exact-model guide; review source/license. Not automatically OEM-certified."));return g;}}
 QVariantList nodes;
 auto question=[&](const QString &id,const QString &ar,const QString &en,const QString &yes,const QString &no){nodes.append(QVariantMap{{"id",id},{"ar",ar},{"en",en},{"yes",yes},{"no",no},{"terminal",false}});};
 auto leaf=[&](const QString &id,const QString &ar,const QString &en){nodes.append(QVariantMap{{"id",id},{"ar",ar},{"en",en},{"terminal",true}});};
 question("safety",QStringLiteral("هل يوجد انتفاخ بطارية أو سائل أو حرارة غير طبيعية؟"),"Is there battery swelling, liquid damage or abnormal heat?","stop","connection");
 leaf("stop",QStringLiteral("أوقف التوصيل والشحن. أحِل الجهاز إلى فني مؤهل وفق إجراءات السلامة؛ لا تضغط البطارية أو تختبر دوائر حية."),"Stop charging/connection. Refer to a qualified technician under the safety procedure; do not press the battery or probe live circuits.");
 if(symptom=="USB not detected"){
  question("connection",QStringLiteral("بعد اختبار كابل بيانات ومنفذ سليمين، هل يظهر الجهاز في Windows؟"),"After testing a known-good data cable and port, is the device visible in Windows?","permission","usb");
  question("permission",QStringLiteral("هل الهاتف مفتوح والموافقة على ADB متاحة وممنوحة من المالك؟"),"Is the phone unlocked and ADB approval available and granted by its owner?","driver","authorize");
  leaf("authorize",QStringLiteral("اطلب موافقة المالك على الهاتف. غياب الموافقة ليس عطل هاردوير، ولا يتم تجاوز القفل."),"Request approval on the owner's phone. Missing authorization is not a hardware diagnosis; no lock bypass.");
  question("driver",QStringLiteral("هل تعريف OEM الصحيح يعمل دون رمز خطأ في إدارة الأجهزة؟"),"Does the correct OEM driver work without a Device Manager error?","verify","install");
  leaf("install",QStringLiteral("افتح تبويب تعريفات الشركات، وطابق الموديل ووضع الاتصال مع المصدر الرسمي ثم أعد الفحص."),"Use the OEM drivers tab, match the exact model/connection mode to its official source, then inspect again.");
  leaf("verify",QStringLiteral("أعد فحص ADB وسجّل الدليل. إذا بقي الانقطاع، قارن الموصل والمسارات بدليل مرخص خاص بهذا الموديل."),"Rescan ADB and record observations. If unstable, compare connector/data paths against a licensed exact-model service guide.");
 }else{
  question("connection",QStringLiteral("هل اختبرت مصدر طاقة وكابلًا سليمين دون مسح أو تعديل النظام؟"),"Have you tested a known-good power source/cable without erasing or changing the OS?","observe","baseline");
  leaf("baseline",QStringLiteral("اختبر الملحقات المعروفة بالصلاحية وسجّل الاستجابة أولًا. لا تستنتج تلف دائرة قبل قياس مؤهل."),"Test known-good accessories and document the response first. Do not infer a circuit fault without qualified measurement.");
  question("observe",QStringLiteral("هل لديك دليل صيانة مرخص ومطابق لرقم الموديل والمراجعة؟"),"Do you have a licensed service guide matching exact model and board revision?","document","specific");
  leaf("document",QStringLiteral("اتبع الدليل المطابق وسجّل الاختبارات والقياسات والقيود. لا تطبق قيم لوحة التدريب على الهاتف."),"Follow the matching guide and record tests, measurements and limits. Training-board values do not apply to the phone.");
  leaf("specific",QStringLiteral("لا توجد هنا شجرة هاردوير موثقة لهذا الموديل. استورد دليلًا مرخصًا مطابقًا؛ لا تستخدم قيم جهد أو مخطط موديل آخر."),"No verified hardware tree is supplied for this model. Import a licensed matching guide; do not use another model's voltages or schematic.");
 }
 if(symptom=="USB not detected")leaf("usb",QStringLiteral("راجع تعريف OEM ووضع USB ثم الموصل بكابل سليم. الفحص الحالي لم يحدد السبب؛ يلزم دليل مطابق قبل قياس اللوحة."),"Review OEM driver and USB mode, then the connector with a known-good cable. Cause remains unconfirmed; use a matching guide before board measurements.");
 return {{"brand",brand.trimmed()},{"model",model.trimmed()},{"symptom",symptom},{"root","safety"},{"nodes",nodes},{"scope",arabic?QStringLiteral("مسار عام مرتبط بسجل هذا الهاتف؛ ليس دليل أعطال خاصًا بالموديل. لا توجد مكتبة شاملة لكل الموديلات بعد."):QStringLiteral("General triage bound to this phone record; not an exact-model service guide. A comprehensive model library is not yet supplied.")}};
}
