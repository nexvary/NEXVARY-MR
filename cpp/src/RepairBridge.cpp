#include "RepairBridge.hpp"
#include "RepairCore.hpp"
#include <QVariantMap>
#include <QMap>
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
QStringList RepairBridge::supportedSymptoms() const {
  QStringList result;
  for(const auto &symptom:mr::supportedSymptoms()) result.append(QString::fromStdString(symptom));
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
  if (symptom == "Bootloop") return QStringLiteral("حدد منصة الجهاز وموديله أولًا، ثم افحص وضع الاسترداد الرسمي للشركة. Fastboot يخص أجهزة Android التي تدعمه ولا ينطبق على iPhone. سجل رسالة الإقلاع ونتائج الفحص قبل محاولة الاستعادة. افحص البطارية ومصدر الطاقة إذا كان الجهاز يعيد التشغيل.\n\nحماية البيانات: لا تمسح الجهاز أو تعدّل نظامه دون موافقة المالك والتحقق من الموديل والنسخة الاحتياطية.");
  if (symptom == "Not charging") return QStringLiteral("توقف عن الشحن عند وجود انتفاخ بطارية أو سائل أو حرارة غير طبيعية، واعزل مصدر الطاقة وفق إجراءات السلامة. إذا لم توجد علامات خطر، ابدأ بكابل بيانات وشاحن معروفين بالصلاحية. افحص الموصل واستجابة البطارية. قارن القياسات بمخطط مرخّص خاص بالموديل قبل تشخيص دائرة الشحن.\n\nالسلامة: قياس الدوائر الحية يحتاج فنيًا مؤهلًا وأدوات مناسبة. لا تفترض أن قيم مخطط التدريب تنطبق على الهاتف.");
  if (symptom == "No network") return QStringLiteral("تحقق من تعرف الجهاز على الشريحة، وحالة النطاق الأساسي والتسجيل في الشبكة باستخدام معلومات النظام المتاحة. اختبر شريحة سليمة قبل الانتقال للهاردوير.\n\nلا تعدّل IMEI أو هوية الجهاز.");
  if (symptom == "USB not detected") return QStringLiteral("اختبر كابل بيانات آخر ومنفذ USB مختلفًا. راجع صلاحية تصحيح USB من الهاتف وحالة ADB: unauthorized تحتاج موافقة على الهاتف؛ offline تحتاج إعادة توصيل. افحص تعريف OEM الرسمي في إدارة الأجهزة.\n\nلا تثبّت حزم تعريفات مجهولة.");
  static const QMap<QString, QString> guidanceAr{
    {QStringLiteral("No power"), QStringLiteral("افحص آثار التلف والسوائل وانتفاخ البطارية والحرارة. إذا كان ذلك آمنًا، جرّب شاحنًا متوافقًا وسجل مؤشرات الاستجابة. ميّز انعدام الطاقة عن غياب الصورة، واتبع إجراء إعادة التشغيل الرسمي للموديل. لا تحقن جهدًا ولا تقصر نقاط الاختبار.")},
    {QStringLiteral("Overheating"), QStringLiteral("افصل الشاحن وأوقف المهام الثقيلة، واترك الهاتف يبرد طبيعيًا بعيدًا عن الشمس. سجل وقت السخونة وتحذيرات النظام. لا تستخدم سوائل أو تبريدًا سريعًا ولا تتجاوز الحماية الحرارية. أوقف الاستخدام عند انتفاخ البطارية.")},
    {QStringLiteral("Battery drains quickly"), QStringLiteral("سجل المدة ونسبة الشحن وظروف الاستخدام. راجع إحصاءات البطارية والإشارة ونشاط التطبيقات في الخلفية. استخدم معلومات صحة البطارية التي يوفرها المصنع فقط. قراءة واحدة لا تثبت تلف البطارية؛ تجنب التفريغ العميق والأدوات المجهولة.")},
    {QStringLiteral("Touchscreen not responding"), QStringLiteral("افحص الرطوبة والتلف والملحقات. بإذن المالك، قارن دون الجراب أو الواقي القابل للإزالة. حدد هل المشكلة في تطبيق واحد أم الشاشة كلها، واستخدم اختبار اللمس الرسمي إن توفر. لا تتجاوز قفل الشاشة وسجل الكسور قبل أي تدخل.")},
    {QStringLiteral("No display"), QStringLiteral("سجل الاهتزاز والرنين وظهور الجهاز عبر USB. افحص السطوع إن أمكن واتبع إعادة التشغيل الرسمية. غياب الصورة لا يثبت تلف الشاشة أو اللوحة. لا تضغط على جهاز منتفخ ولا تفحص موصل الشاشة تحت الطاقة.")},
    {QStringLiteral("Liquid exposure"), QStringLiteral("افصل الطاقة والملحقات وأوقف الشحن. سجل نوع التعرض ورسائل اكتشاف السائل، واتبع تعليمات التجفيف والصيانة الرسمية. لا تستخدم حرارة أو هواء مضغوطًا أو أرزًا ولا تتجاوز تحذير السوائل. جفاف السطح لا يثبت سلامة الداخل.")},
    {QStringLiteral("Storage full"), QStringLiteral("سجل المساحة الكلية والمتاحة من الإعدادات. حدد الفئات الكبيرة مع المالك وميّز الملفات الشخصية عن التخزين المؤقت. تحقق من النسخة الاحتياطية قبل أي تنظيف، ثم قارن المساحة والعرض. لا تحذف البيانات أو تهيئ التخزين دون موافقة.")},
    {QStringLiteral("App crashes"), QStringLiteral("سجل التطبيق وإصداره وخطوات تكرار التعطل وتوقيته. راجع السجلات المصرح بها وقارن تطبيقًا آخر والمساحة المتاحة. راجع التحديثات الرسمية. لا تمسح بيانات التطبيق أو تعِد تثبيته قبل مناقشة البيانات المحلية والنسخة الاحتياطية.")},
    {QStringLiteral("Camera not working"), QStringLiteral("سجل رسالة الخطأ والعدسة المتأثرة. افحص إذن الكاميرا ومفتاح الخصوصية وقارن تطبيق الكاميرا الرسمي بتطبيق مصرح به. افحص العوائق الخارجية. لا تصل إلى صور المالك؛ خطأ التطبيق لا يثبت تلف المستشعر.")},
    {QStringLiteral("Microphone not working"), QStringLiteral("افحص الكتم وإذن الميكروفون والخصوصية والجراب. بإذن المالك قارن تسجيلًا قصيرًا بمكالمة، وافحص مسار الصوت السلكي والبلوتوث. تجنب المحادثات الخاصة واحذف التسجيل التجريبي حسب الاتفاق. لا تدخل أدوات في فتحات الميكروفون.")},
    {QStringLiteral("Speaker not working"), QStringLiteral("افحص مستوى الصوت والكتم وعدم الإزعاج ومسار إخراج الصوت. قارن صوت اختبار معروفًا بصوت المكالمات بمستوى مريح. سجل المخارج المتأثرة. لا تدخل أدوات في الفتحات؛ صمت تطبيق واحد لا يثبت تلف دائرة الصوت.")},
    {QStringLiteral("Wi-Fi not working"), QStringLiteral("حدد هل الشبكة لا تظهر أم يفشل الاتصال أم يغيب الإنترنت. افحص وضع الطيران وقارن شبكة أو جهازًا مصرحًا به. سجل الخطأ والتوقيت وراجع الإرشادات الرسمية. إعادة إعدادات الشبكة تمسح الإعدادات وتتطلب موافقة؛ لا تكشف كلمات المرور.")},
    {QStringLiteral("Bluetooth not working"), QStringLiteral("افحص تشغيل البلوتوث وطاقة الملحق ووضع الاقتران والتوافق الرسمي. قارن ملحقًا معروفًا ومصرحًا به، وحدد هل الفشل في الاكتشاف أم الاقتران أم الصوت. لا تقترن بأجهزة مجهولة؛ حذف الاقترانات يحتاج موافقة.")},
    {QStringLiteral("SIM not detected"), QStringLiteral("سجل رسالة الشريحة وتحقق من تفعيل الخط لدى المشغل. للشريحة القابلة للإزالة، اتبع طريقة الدرج الرسمية وقارن شريحة سليمة مصرحًا بها. حدد هل الخلل يتبع الشريحة أم الهاتف. لا تحذف ملفات eSIM أو تعدل الهوية أو تتجاوز أقفال المشغل.")},
    {QStringLiteral("ADB unauthorized"), QStringLiteral("تأكد من تفعيل تصحيح USB بواسطة المالك على Android. افتح قفل الهاتف بالطريقة المعتادة وراجع طلب تفويض RSA؛ يوافق المالك على الكمبيوتر المقصود فقط. أعد التوصيل وسجل الحالة. لا تتجاوز التفويض أو قفل الشاشة.")},
    {QStringLiteral("ADB offline"), QStringLiteral("سجل الجهاز والحالة وافحص الكابل والمنفذ واستجابة الهاتف. أعد توصيل الجهاز المقصود وتحقق من التفويض وإصدار أدوات المنصة. هذه الحالة لا تثبت تلف النظام؛ لا تمسح البيانات ولا توقف جلسات أخرى ولا تغير وضع الإقلاع تلقائيًا.")},
    {QStringLiteral("Fastboot not detected"), QStringLiteral("تحقق من دعم الموديل لوضع Fastboot ومن وجوده في الوضع الرسمي المناسب. افحص USB والكابل والتعريف الرسمي وسجل وضع الهاتف ونتيجة الأداة. غياب Fastboot لا يثبت عطلًا؛ لا تفتح محمل الإقلاع أو تفلش أو تمسح تلقائيًا.")},
    {QStringLiteral("Slow performance"), QStringLiteral("1. سجل العملية ومدتها وهل تؤثر في تطبيق واحد أم النظام كله.\n2. افحص المساحة المتاحة وتغييرات التطبيقات وتحذيرات الحرارة.\n3. قارن بعد إعادة تشغيل عادية بموافقة المالك.\n4. راجع إرشادات التطبيق والنظام الرسمية وسجل دليلًا قابلًا للتكرار.\n\nتجنب تطبيقات التنظيف المجهولة وحذف بيانات المستخدم أو تعطيل الحماية. البطء وحده لا يثبت تلف الذاكرة أو التخزين.")},
  };
  return QStringLiteral("إرشادات قائمة على قواعد، وليست تشخيصًا مؤكدًا:\n\n") + guidanceAr.value(symptom, QStringLiteral("لا توجد إرشادات لهذا العَرَض. اختر عَرَضًا من القائمة."));
}

QString RepairBridge::analyzeLogReport(const QString &log,bool arabic) const {
  if(log.trimmed().isEmpty())return arabic?QStringLiteral("اقرأ السجلات من جهاز مصرح به أولًا."):QStringLiteral("Read logs from an authorized device first.");
  if(log.toUtf8().size()>512*1024)return arabic?QStringLiteral("تجاوز حجم السجل الحد المسموح."):QStringLiteral("Log exceeds the allowed size.");
  const auto findings=mr::analyzeLog(log.toUtf8().toStdString());
  QString out=arabic?QStringLiteral("مؤشرات نصية إرشادية فقط، لا تؤكد سبب العطل. راجع الدليل والسياق قبل اتخاذ إجراء.\n\n"):QStringLiteral("Text indicators only; these do not confirm a fault cause. Review evidence and context before acting.\n\n");
  const QMap<QString,QString> titles={{"runtime_crash",QStringLiteral("تعطل تطبيق أو عملية")},{"watchdog",QStringLiteral("ذكر مراقب الاستجابة Watchdog")},{"memory_pressure",QStringLiteral("ضغط ذاكرة محتمل")},{"usb_transport",QStringLiteral("انقطاع أو مشكلة اتصال")},{"permission_denial",QStringLiteral("رفض صلاحية؛ قد يكون سلوك حماية طبيعيًا")}};
  for(const auto &finding:findings){const auto code=QString::fromStdString(finding.code);out+=(arabic?titles.value(code):code)+QStringLiteral(" (%1)\n").arg(finding.count)+QString::fromUtf8(finding.firstEvidence.data(),static_cast<qsizetype>(finding.firstEvidence.size()))+QStringLiteral("\n\n");}
  if(findings.empty())out+=arabic?QStringLiteral("لم تظهر الأنماط المعروفة في هذه العينة. هذا لا يثبت سلامة الجهاز."):QStringLiteral("No known patterns in this sample. This does not establish device health.");
  return out;
}
