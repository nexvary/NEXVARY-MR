import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
 id: page
 property var app
 property bool consent: false
 property bool reset: false
 property int sourceIndex: 0
 property int readabilityIndex: 0
 readonly property var sourceKeys: ["external_card", "accessible_backup", "cloud_export", "internal_storage"]
 readonly property bool blocked: !consent || (sourceIndex === 3 && reset) || (sourceIndex === 0 && readabilityIndex === 2)
 readonly property string verdict: !consent ? "not_supported" :
   sourceIndex === 3 && reset ? "not_supported" :
   sourceIndex === 0 && readabilityIndex === 0 ? "need_more_information" :
   sourceIndex === 0 && readabilityIndex === 2 ? "not_supported" :
   sourceIndex === 0 ? "acquire_copy" :
   sourceIndex === 1 ? "verify_backup" :
   sourceIndex === 2 ? "request_export" : "need_more_information"
 readonly property var instructionsAr: ({
  "not_supported":"لا يمكن المتابعة بهذه الحالة. يلزم إثبات موافقة المالك، وقد تكون مفاتيح تشفير التخزين الداخلي قد حُذفت بعد ضبط المصنع.",
  "need_more_information":"وثّق نوع التخزين وحالة الوصول والمعلومات المتاحة قبل أي إجراء.",
  "acquire_copy":"أنشئ صورة للبطاقة بطريقة القراءة فقط، ثم احسب SHA-256 وافحص نسخة العمل دون الكتابة على الأصل.",
  "verify_backup":"احصر النسخ الاحتياطية المتاحة وتحقق من سلامتها قبل الاستعادة.",
  "request_export":"اطلب تصدير البيانات من الحساب المعتمد عبر الواجهة الرسمية للمزوّد."
 })
 readonly property var instructionsEn: ({
  "not_supported":"Do not proceed. Owner consent is required; factory reset may have destroyed internal encryption keys.",
  "need_more_information":"Record the storage type and access state before taking any action.",
  "acquire_copy":"Create a read-only card image, calculate SHA-256, and examine a working copy only.",
  "verify_backup":"Inventory accessible backups and verify integrity before restoring.",
  "request_export":"Request an authorized export from the provider's official interface."
 })
 ScrollView {
  id: recoveryScroll
  objectName: "recoveryScroll"
  anchors.fill: parent
  contentWidth: availableWidth
  clip: true
  ColumnLayout {
   width: recoveryScroll.availableWidth
   spacing: 14
   Panel {
    title: app.trx("تقييم استعادة البيانات","Recovery feasibility assessment")
    Layout.fillWidth: true
    Label {
     Layout.fillWidth: true
     wrapMode: Text.WordWrap
     color: app.tone("#b8d1e6")
     text: app.trx("تقييم محلي لا يقرأ الهاتف ولا يعد باستعادة الملفات. لا يبدأ أي فحص دون إذن صاحب الجهاز.",
       "Local assessment only: no device access, no recovery promise, and no acquisition without owner authorization.")
    }
    CheckBox {
     id: authorization
     objectName: "recoveryConsent"
     text: app.trx("لدي موافقة موثقة من مالك الجهاز","I have documented authorization from the owner")
     checked: page.consent
     onCheckedChanged: page.consent = checked
    }
    CheckBox {
     id: resetChoice
     objectName: "recoveryReset"
     text: app.trx("تمت إعادة ضبط المصنع","Device was factory-reset")
     checked: page.reset
     onCheckedChanged: page.reset = checked
    }
    Label { text: app.trx("مصدر البيانات","Data source"); color: app.tone("#e8f2ff") }
    ComboBox {
     id: source
     objectName: "recoverySource"
     Layout.fillWidth: true
     model: app.ar ? ["بطاقة ذاكرة خارجية","نسخة احتياطية متاحة","تصدير من حساب سحابي","تخزين الهاتف الداخلي"] :
       ["External memory card","Accessible backup","Cloud account export","Internal phone storage"]
     currentIndex: page.sourceIndex
     onCurrentIndexChanged: page.sourceIndex = currentIndex
    }
    Label {
     visible: page.sourceIndex === 0
     text: app.trx("هل البطاقة قابلة للقراءة؟","Is the card readable?")
     color: app.tone("#e8f2ff")
    }
    ComboBox {
     id: readable
     objectName: "recoveryReadable"
     visible: page.sourceIndex === 0
     Layout.fillWidth: true
     model: app.ar ? ["غير معروف","نعم","لا"] : ["Unknown","Yes","No"]
     currentIndex: page.readabilityIndex
     onCurrentIndexChanged: page.readabilityIndex = currentIndex
    }
   }
   Panel {
    title: app.trx("نتيجة التقييم الأولي","Preliminary assessment")
    Layout.fillWidth: true
    Label {
     objectName: "recoveryVerdict"
     text: page.verdict
     font.bold: true
     color: page.blocked ? app.tone("#f4b56f") : app.tone("#58d7ae")
    }
    Label {
     Layout.fillWidth: true
     wrapMode: Text.WordWrap
     text: app.ar ? page.instructionsAr[page.verdict] : page.instructionsEn[page.verdict]
     color: app.tone("#e4efff")
    }
    Label {
     Layout.fillWidth: true
     wrapMode: Text.WordWrap
     color: app.tone("#f1bd81")
     text: app.trx("Shizuku لا يوفر صلاحيات Root ولا يستعيد مفاتيح التشفير المفقودة. لا تستخدم الهاتف لكتابة بيانات جديدة قبل التقييم.",
       "Shizuku does not grant root or restore lost encryption keys. Avoid writing new data before evaluation.")
    }
   }
  }
 }
}
