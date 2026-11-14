import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"aboutSystemPage";contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;title:"NEXVARY MR";iconName:"shield_checkmark"
   RowLayout {Layout.fillWidth:true;spacing:18
    Image {source:"qrc:/assets/branding/nexvary-eye-seal.png";Layout.preferredWidth:100;Layout.preferredHeight:100;fillMode:Image.PreserveAspectFit;mipmap:true;Accessible.name:page.app.trx("ختم نيكسفاري","Nexvary seal")}
    ColumnLayout {Layout.fillWidth:true
     Label {objectName:"aboutSystemSubtitle";text:page.app.trx("منظومة العمليات والاستخبارات","Operations & Intelligence System");font.pixelSize:22;font.bold:true;color:page.app.design.text;Layout.fillWidth:true;wrapMode:Text.WordWrap}
     Label {objectName:"aboutSystemVersion";text:page.app.trx("الإصدار: ","Version: ")+page.app.productVersion;color:page.app.design.silver}
    }
   }
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("بيئة عمل موحدة لإدارة الأجهزة والمركز والقضايا والأدلة الرقمية والبحث الاستخباراتي. تعرض نتائج العمليات ومصادرها وحالتها الفعلية، وتفصل بين البيانات الحقيقية والأمثلة التدريبية.","A unified workspace for devices, center operations, cases, digital evidence and intelligence search. It displays actual operation states and provenance and distinguishes real data from training examples.")}
  }
  Repeater {model:[
   ["phone","الأجهزة وتشخيص USB","Devices & USB diagnostics","اكتشاف ADB وFastboot، اختيار الجهاز، قراءة خصائصه وسجلاته وفحص جرد USB والتعريفات. يلزم الكابل والتعريفات وموافقة الهاتف المناسبة.","Discover ADB/Fastboot targets, select a device, inspect identity and logs, USB inventory and drivers. A suitable cable, drivers and handset authorization are required."],
   ["clipboard_task","إدارة المركز","Center management","تنظيم العملاء والأجهزة وأوامر الصيانة وحالاتها وقطع الغيار والفواتير ضمن السجلات المحلية.","Organize customers, devices, repair orders and their states, parts inventory and invoices in local records."],
   ["document_lock","القضايا والأدلة","Cases & evidence","إنشاء القضايا وتحريرها وحفظها وإعادة فتحها، وربط الأدلة والنتائج وسجل الإجراءات والتقارير ضمن النطاق المتاح.","Create, edit, save and reopen cases; associate evidence, findings, audit history and reports within the available scope."],
   ["search_visual","البحث الاستخباراتي","Intelligence search","بطاقات بحث مرتبطة بمحوّل الخدمة؛ البحث الموحد للهاتف والإيميل واسم المستخدم، وخدمات متخصصة بحسب دليل المزوّد. يلزم مفتاح صالح واتصال بالإنترنت وموافقة صريحة قبل أي طلب مدفوع. السعر المعروض للعميل منفصل عن تكلفة المزوّد؛ الفاتورة المستحقة ليست إثبات تحصيل.","Cards connect to the service adapter: unified phone, email and username lookup, plus specialized tools supported by the provider catalog. A valid key, Internet connection and explicit billable-request confirmation are required. Retail price is separate from supplier cost; an invoice due does not establish payment collection."],
   ["database","النتائج والعلاقات والتقارير","Results, relationships & reports","عرض النتائج المنظمة وروابط المصدر، وربطها بالقضية وتصديرها حسب الوظيفة. لا تُنشأ علاقات أو نتائج افتراضية على أنها أدلة حقيقية.","Display structured findings and provenance; associate them with cases and export where supported. Invented relationships or demonstration results are never real evidence."],
   ["book_open_lightbulb","الأبحاث والفحص المحلي","Research & local inspection","مراجعة مصادر التقنيات وحدودها، وفحص بيانات ملفات النظام والحزم والأدلة محليًا بالأدوات المتاحة. فهرس أداة خارجية ليس تضمينًا لها ولا إثبات توافق معها.","Review technology sources and limitations and inspect firmware, packages and evidence locally using configured tools. A catalog entry is neither bundled third-party software nor verified interoperability."],
   ["shield_checkmark","الحماية وحدود التشغيل","Protection & operating limits","حماية المفاتيح المحلية على Windows باستخدام DPAPI، وتوثيق العمليات. نسخة الإنتاج تقيد أوامر الهاتف بالقراءة والفحص؛ العمليات المعدّلة غير المختبرة تبقى غير متاحة. تشغيل ADB مباشرة لا يمنح تلقائيًا صلاحية تعديل الجهاز.","Local Windows credentials use DPAPI and operations are audited. Production device commands remain limited to inspection and reads; unverified modifying operations stay unavailable. Direct ADB access does not enable device writes automatically."]]
   delegate:Panel {required property var modelData;Layout.fillWidth:true;iconName:modelData[0];title:page.app.trx(modelData[1],modelData[2])
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx(modelData[3],modelData[4]);textFormat:Text.PlainText}
   }
  }
  Panel {Layout.fillWidth:true;title:page.app.trx("الترخيص والاستخدام المؤسسي","Institutional licensing & use");iconName:"document_lock"
   Label {objectName:"institutionalUsePolicy";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("تطوّر Nexvary Inc. هذا النظام وتخصص ترخيصه حصريًا لجهات إنفاذ القانون والجهات والوكالات المسجلة والمخوّلة قانونًا بالتحقيقات الجنائية الرقمية. لا يُطرح للاستخدام الشخصي ولا يُرخّص للأفراد بصفتهم الشخصية؛ يقتصر التشغيل على ممثلي الجهة وموظفيها المفوّضين ضمن ترخيصها وصلاحياتهم القانونية. التثبيت أو الشراء لا يمنح صفة رسمية أو سلطة تحقيق أو تصريح وصول إلى أي جهاز أو بيانات.","Nexvary Inc. develops this system exclusively for licensed institutional use by law-enforcement bodies and registered entities legally authorized to conduct digital criminal investigations. It is not offered for personal use or licensed to individuals in their private capacity. Only authorized institutional personnel may operate it within their organization's license and lawful authority. Installation or purchase grants no official status, investigative power or permission to access devices or data.")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;text:page.app.trx("تستلزم نتائج المصادر الخارجية تحققًا مستقلًا؛ لا تثبت وحدها هوية شخص أو ارتكابه مخالفة. اعتماد جهاز فعلي أو نجاح بحث مدفوع لا يُستنتج من اختبارات المحاكاة.","External findings require independent verification; they alone do not establish identity or wrongdoing. Mock tests do not establish physical-device acceptance or successful live paid searches.")}
  }
 }
}
