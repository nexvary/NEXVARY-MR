#include "RepairBridge.hpp"
#include "RepairCore.hpp"
#include <QVariantMap>
QVariantList RepairBridge::brands() const {
  QVariantList result;
  for (const auto &entry : mr::catalog()) {
    QVariantMap row;
    row.insert(QStringLiteral("brand"), QString::fromStdString(entry.brand));
    QStringList families;
    for (const auto &family : entry.families) families.append(QString::fromStdString(family));
    row.insert(QStringLiteral("families"), families);
    result.append(row);
  }
  return result;
}
QString RepairBridge::diagnose(const QString &symptom) const {
  const auto rule = mr::diagnose(symptom.toStdString());
  return QString::fromStdString(rule.nextCheck) + QStringLiteral("\nSafety: ") + QString::fromStdString(rule.safety);
}
bool RepairBridge::validCase(const QString &brand, const QString &model, const QString &symptom) const {
  return mr::validCase(brand.toStdString(), model.toStdString(), symptom.toStdString());
}

QString RepairBridge::diagnoseLocalized(const QString &symptom, bool arabic) const {
  if (!arabic) return diagnose(symptom);
  if (symptom == "Bootloop") return QStringLiteral("افحص توفر وضع الاسترداد وFastboot، وسجل رسالة الإقلاع ونتائج الفحص قبل محاولة الاستعادة. افحص البطارية ومصدر الطاقة إذا كان الجهاز يعيد التشغيل.\n\nحماية البيانات: لا تمسح الجهاز أو تعدّل نظامه دون موافقة المالك والتحقق من الموديل والنسخة الاحتياطية.");
  if (symptom == "Not charging") return QStringLiteral("ابدأ بكابل بيانات وشاحن معروفين بالصلاحية. افحص الموصل واستجابة البطارية. قارن القياسات بمخطط مرخّص خاص بالموديل قبل تشخيص دائرة الشحن.\n\nالسلامة: قياس الدوائر الحية يحتاج فنيًا مؤهلًا وأدوات مناسبة. لا تفترض أن قيم مخطط التدريب تنطبق على الهاتف.");
  if (symptom == "No network") return QStringLiteral("تحقق من تعرف الجهاز على الشريحة، وحالة النطاق الأساسي والتسجيل في الشبكة باستخدام معلومات النظام المتاحة. اختبر شريحة سليمة قبل الانتقال للهاردوير.\n\nلا تعدّل IMEI أو هوية الجهاز.");
  if (symptom == "USB not detected") return QStringLiteral("اختبر كابل بيانات آخر ومنفذ USB مختلفًا. راجع صلاحية تصحيح USB من الهاتف وحالة ADB: unauthorized تحتاج موافقة على الهاتف؛ offline تحتاج إعادة توصيل. افحص تعريف OEM الرسمي في إدارة الأجهزة.\n\nلا تثبّت حزم تعريفات مجهولة.");
  return QStringLiteral("اجمع الموديل ووصف العطل ونتائج الفحوص غير المدمرة أولًا. لا يمكن تأكيد العطل دون أدلة من الجهاز.");
}
