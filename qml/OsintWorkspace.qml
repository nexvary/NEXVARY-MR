import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: workspace
 objectName: "osintWorkspace"
 property var app
 property int activeTab: 0
 property int selectedProvider: 0
 readonly property var evaluation: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 readonly property bool hasCase: !!lawCaseBridge.activeCase.id
 LayoutMirroring.enabled: app.ar
 LayoutMirroring.childrenInherit: true
 spacing: 16
 GlossSurface {
  Layout.fillWidth: true; implicitHeight: 138; color: "#101A28"; border.color: "#304A65"
  RowLayout {
   anchors.fill: parent; anchors.margins: 24; spacing: 20
   Image {source: "qrc:/assets/icons/shield_checkmark.svg"; Layout.preferredWidth: 58; Layout.preferredHeight: 58; sourceSize: Qt.size(58,58)}
   ColumnLayout {
    Layout.fillWidth: true; spacing: 8
    Label {text: "NEXVARY  /  INTELLIGENCE"; color: "#6BB8FF"; font.pixelSize: 12; font.letterSpacing: 2}
    Label {text: workspace.app.trx("مساحة استخبارات المصادر المفتوحة", "Open Source Intelligence"); color: "#F1F5FA"; font.pixelSize: 27; font.bold: true; wrapMode: Text.WordWrap; Layout.fillWidth: true}
    Label {text: workspace.app.trx("قضية واضحة. مصادر موثقة. تكلفة معتمدة قبل البحث.", "Case context. Traceable sources. Approved cost before a search."); color: "#AABBD0"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true}
   }
   StateBadge {text: workspace.app.trx("تقييم", "EVALUATION"); tint: "#6BB8FF"}
  }
 }
 RowLayout {
  Layout.fillWidth: true; spacing: 8
  Repeater {
   model: workspace.app.ar ? ["نظرة عامة", "مساحة البحث", "النتائج والمصادر", "الاستخدام والسجل"] : ["Overview", "Query workspace", "Results & sources", "Usage & history"]
   delegate: ActionButton {
    required property string modelData
    required property int index
    objectName: "osintTab" + index
    text: modelData; primary: workspace.activeTab === index
    Layout.fillWidth: true; Layout.minimumWidth: 0
    onClicked: workspace.activeTab = index
   }
  }
 }
 GridLayout {
  visible: workspace.activeTab === 0; Layout.fillWidth: true
  columns: width > 900 ? 3 : 1; columnSpacing: 14; rowSpacing: 14
  Repeater {
   model: [
    {name:"Lampyre",icon:"shield_checkmark",ar:"API / MCP تجريبي",en:"API / MCP evaluation",stateAr:"التفعيل غير مؤكد",stateEn:"Activation unconfirmed",detailAr:"التوثيق الخاص بالحساب مطلوب قبل الاتصال.",detailEn:"Account documentation is required before connecting."},
    {name:"PhoneInfoga",icon:"phone",ar:"بحث رقم هاتف",en:"Phone number research",stateAr:"محرك خارجي",stateEn:"External engine",detailAr:"يتطلب محركًا مراجعًا وقضية وتفويضًا محددًا.",detailEn:"Requires a reviewed engine, case and specific authorization."},
    {name:"OSINT Industries",icon:"document_lock",ar:"مزوّد تجاري",en:"Commercial provider",stateAr:"غير مفعّل",stateEn:"Not enabled",detailAr:"الترخيص والتكلفة والصلاحيات قيد المراجعة.",detailEn:"License, pricing and access remain under review."}
   ]
   delegate: Panel {
    required property var modelData
    required property int index
    Layout.fillWidth: true; Layout.minimumWidth: 0; Layout.preferredHeight: 252
    title: modelData.name; subtitle: workspace.app.ar ? modelData.ar : modelData.en
    iconName: modelData.icon; accent: index === 0 ? "blue" : "silver"
    Label {text: workspace.app.ar ? modelData.stateAr : modelData.stateEn; color: index === 0 ? "#6BB8FF" : workspace.app.design.silver; font.pixelSize: 17; font.bold: true; wrapMode: Text.WordWrap; Layout.fillWidth: true}
    Label {text: workspace.app.ar ? modelData.detailAr : modelData.detailEn; color: workspace.app.design.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true; Layout.fillHeight: true}
    ActionButton {text: workspace.app.trx("فتح المزوّد", "Open provider"); Layout.fillWidth: true; onClicked: {workspace.selectedProvider = index; workspace.activeTab = 1}}
   }
  }
 }
 GridLayout {
  visible: workspace.activeTab === 0; Layout.fillWidth: true
  columns: width > 800 ? 2 : 1; columnSpacing: 14; rowSpacing: 14
  Panel {
   Layout.fillWidth: true; title: workspace.app.trx("سياق القضية", "Case context"); iconName: "document_lock"
   Label {text: workspace.hasCase ? (lawCaseBridge.activeCase.metadata.caseNumber || lawCaseBridge.activeCase.id) : workspace.app.trx("لا توجد قضية مفتوحة", "No case open"); color: workspace.app.design.text; font.pixelSize: 19; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WrapAnywhere; textFormat: Text.PlainText}
   Label {text: workspace.app.trx("البحث الخارجي يحتاج تفويض الإفصاح وصلاحيات القضية. المختبر الاصطناعي منفصل عن الأدلة.", "External research requires disclosure approval and case permissions. Synthetic fixtures are separate from evidence."); color: workspace.app.design.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  }
  Panel {
   Layout.fillWidth: true; title: workspace.app.trx("ميزانية المزوّد", "Provider budget"); iconName: "database"
   Label {text: workspace.app.trx("الرصيد غير متاح", "Balance unavailable"); color: workspace.app.design.text; font.pixelSize: 19; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap}
   Label {text: workspace.app.trx("لا يوجد اتصال بحساب Lampyre. عرض 200 Photon ليس رصيدًا مؤكدًا. لا يبدأ طلب مدفوع من هذه الواجهة.", "No Lampyre account connection. The offered 200 Photons are not a confirmed balance. This workspace cannot submit paid requests."); color: workspace.app.design.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  }
 }
 ColumnLayout {
  visible: workspace.activeTab === 1; Layout.fillWidth: true; spacing: 14
  RowLayout {
   Layout.fillWidth: true
   Label {text: workspace.app.trx("اختيار المزوّد", "Select provider"); color: workspace.app.design.muted}
   NeonComboBox {objectName: "osintProviderSelector"; Layout.fillWidth: true; model: ["Lampyre", "PhoneInfoga", "OSINT Industries"]; currentIndex: workspace.selectedProvider; onActivated: workspace.selectedProvider = currentIndex}
  }
  LampyrePanel {Layout.fillWidth: true; app: workspace.app; visible: workspace.selectedProvider === 0}
  PhoneOsintPanel {Layout.fillWidth: true; app: workspace.app; visible: workspace.selectedProvider === 1}
  Panel {
   Layout.fillWidth: true; visible: workspace.selectedProvider === 2
   title: "OSINT Industries"; iconName: "document_lock"
   Label {text: workspace.app.trx("الاتصال غير مفعّل. يلزم التحقق من ترخيص التكامل والتوثيق والتكلفة قبل إتاحة البحث.", "Connection is not enabled. Integration licensing, documentation and costs must be verified before searches can be enabled."); color: workspace.app.design.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true}
  }
 }
 Panel {
  visible: workspace.activeTab === 2; Layout.fillWidth: true
  title: workspace.app.trx("النتائج والمصادر", "Results & sources"); iconName: "document_text"
  StateBadge {text: workspace.app.trx("بيانات اصطناعية فقط", "SYNTHETIC DATA ONLY"); tint: "#6BB8FF"}
  Label {text: workspace.evaluation && workspace.evaluation.result.source ? workspace.app.trx("اكتمل اختبار محلي — ليس بحثًا فعليًا", "Local fixture completed — not a live search") : workspace.app.trx("لا توجد نتائج Lampyre في هذه الجلسة", "No Lampyre results in this session"); color: workspace.app.design.text; font.pixelSize: 20; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  Label {text: workspace.app.trx("مصادر البحث الفعلي ستظهر مع مرجع المصدر ووقت الجمع وحالة التحقق. نتائج PhoneInfoga تبقى داخل نموذج البحث والأدلة الحالي.", "Live findings will need source references, collection times and verification status. PhoneInfoga results remain in the existing query and evidence workflow."); color: workspace.app.design.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true}
  Label {visible: !!workspace.evaluation && !!workspace.evaluation.result.source; text: workspace.evaluation ? workspace.evaluation.result.source || "" : ""; textFormat: Text.PlainText; color: workspace.app.design.accent; Layout.fillWidth: true; wrapMode: Text.WrapAnywhere}
  ActionButton {text: workspace.app.trx("فتح مساحة البحث", "Open query workspace"); onClicked: workspace.activeTab = 1}
 }
 Panel {
  visible: workspace.activeTab === 3; Layout.fillWidth: true
  title: workspace.app.trx("الاستخدام والسجل", "Usage & history"); iconName: "history"
  GridLayout {
   Layout.fillWidth: true; columns: width > 700 ? 3 : 1; columnSpacing: 20
   Label {text: workspace.app.trx("الرصيد الفعلي\nغير متاح", "Actual balance\nUnavailable"); color: workspace.app.design.text; font.pixelSize: 18; Layout.fillWidth: true; wrapMode: Text.WordWrap}
   Label {text: workspace.app.trx("تكلفة الاختبار المحلي\n0 Photon", "Local fixture cost\n0 Photons"); color: workspace.app.design.success; font.pixelSize: 18; Layout.fillWidth: true; wrapMode: Text.WordWrap}
   Label {text: workspace.app.trx("التكلفة قبل الطلب\nغير متاحة — يلزم التأكيد", "Pre-query cost\nUnavailable — approval required"); color: workspace.app.design.silver; font.pixelSize: 18; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  }
  Label {text: workspace.app.trx("لم يُربط سجل استخدام Lampyre بالحساب بعد. لا تمثل هذه الصفحة دفتر تدقيق دائمًا. سجلات القضايا الحالية متاحة في التقارير والتدقيق.", "Lampyre account usage history is not connected. This page is not a durable audit ledger. Existing case records are available under Reports & audit."); color: workspace.app.design.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap}
 }
}
