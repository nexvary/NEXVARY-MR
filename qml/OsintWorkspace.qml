import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
 id: workspace
 objectName: "osintWorkspace"
 property var app
 property bool demoMode: false
 property bool showAdvanced: false
 property bool showPhoneSearch: false
 property int activeTab: 1
 property bool showInspector: true
 property bool showRequests: true
 readonly property var evaluation: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 readonly property bool administrator: typeof identityBridge !== "undefined"
     && identityBridge.authenticated && identityBridge.session.role === "administrator"
 readonly property bool caseReady: typeof lawCaseBridge !== "undefined"
     && !!lawCaseBridge.activeCase.id && lawCaseBridge.permissions.analyze
     && lawCaseBridge.permissions.addEvidence && lawCaseBridge.devices.length > 0

 onCaseReadyChanged: {
  if (!caseReady)
   showPhoneSearch = false
 }

 onAdministratorChanged: {
  if (!administrator)
   showAdvanced = false
 }

 onDemoModeChanged: {
  if (!demoMode && evaluation)
   evaluation.clear()
  activeTab = 1
 }
 onVisibleChanged: {
  if (!visible) {
   showAdvanced = false
   showPhoneSearch = false
   demoMode = false
  }
 }

 LayoutMirroring.enabled: app.ar
 LayoutMirroring.childrenInherit: true
 spacing: 12
 QtObject {
  id:desktopTokens
  readonly property color text:"#E5EDF3"
  readonly property color muted:"#9EB4C1"
  readonly property color surface:"#0A0A0A"
  readonly property color raised:"#0A2231"
  readonly property color border:"#8396A3"
  readonly property color accent:"#23B9E7"
  readonly property color silver:"#BCCED8"
  readonly property color success:"#75FBA0"
  readonly property color danger:"#FF6A7D"
 }
 QtObject {
  id:desktopApp
  readonly property bool ar:workspace.app.ar
  readonly property var design:desktopTokens
  function trx(a,e){return workspace.app.trx(a,e)}
  function openExternal(url){return workspace.app.openExternal(url)}
  function humanError(error){return workspace.app.humanError(error)}
  readonly property bool governmentMode:workspace.app.governmentMode
  function roleLabel(role){return workspace.app.roleLabel(role)}
  function openAccountPermissions(){workspace.app.openAccountPermissions()}
 }

 Label {
  Layout.fillWidth: true
  color: desktopTokens.text
  font.pixelSize: 24
  font.bold: true
  text: "NEXVARY Intelligence"
 }
 Label {
  Layout.fillWidth: true
  wrapMode: Text.WordWrap
  color: desktopTokens.muted
  text: workspace.app.trx("خدمات البحث والتحليل", "Search and analysis services")
 }
 GridLayout {
  Layout.fillWidth: true
  columns: width >= 620 ? 2 : 1
  columnSpacing: 12
  rowSpacing: 12

  Repeater {
   model: [
    {
     title: workspace.app.trx("تحليل رقم هاتف", "Phone number analysis"),
     description: workspace.app.trx("افحص بيانات خطة الترقيم وسجّل الملاحظات ضمن قضية مخوّلة.",
                                    "Check numbering-plan data and retain observations in an authorized case."),
     input: workspace.app.trx("الإدخال: رقم هاتف دولي", "Input: international phone number"),
     price: workspace.app.trx("دون رسوم لخطة الترقيم المحلية", "No charge for the offline numbering plan"),
     icon: "shield_checkmark",
     status: workspace.caseReady
         ? workspace.app.trx("القضية جاهزة لإدخال الطلب", "Case ready for search setup")
         : workspace.app.trx("يلزم فتح قضية مخوّلة وتسجيل جهاز", "Open an authorized case and register a device")
    },
    {
     title: workspace.app.trx("البحث المدفوع", "Billable search"),
     description: workspace.app.trx("تظهر الخدمات المدفوعة بعد التحقق من إتاحتها.",
                                    "Billable services appear when their availability can be verified."),
     input: workspace.app.trx("نوع الإدخال يحدده دليل الخدمات", "Input type is defined by the service catalog"),
     price: workspace.app.trx("السعر الافتراضي: ‎€14.00‎ لكل بحث", "Default price: €14.00 per search"),
     icon: "search_visual",
     status: workspace.app.trx("غير متاح حاليًا", "Currently unavailable")
    }
   ]
   delegate: Pane {
    id: serviceCard
    required property int index
    required property var modelData
    readonly property bool phoneService: index === 0
    objectName: phoneService ? "intelligencePhoneCard" : "intelligenceBillingCard"
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1
    Layout.minimumWidth: 0
    padding: 16
    background: Rectangle {
     radius: 8
     color: desktopTokens.surface
     border.color: searchAction.hovered || searchAction.activeFocus ? desktopTokens.accent : desktopTokens.border
    }
    contentItem: ColumnLayout {
     spacing: 8
     RowLayout {
      Layout.fillWidth: true
      Image {
       source: "qrc:/assets/icons-professional/" + serviceCard.modelData.icon + ".svg"
       Layout.preferredWidth: 32
       Layout.preferredHeight: 32
      }
      Label {
       Layout.fillWidth: true
       wrapMode: Text.WordWrap
       color: desktopTokens.text
       font.bold: true
       font.pixelSize: 16
       text: serviceCard.modelData.title
      }
     }
     Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.muted; text: serviceCard.modelData.description }
     Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.muted; text: serviceCard.modelData.input }
     Item { Layout.fillHeight: true }
     Label {
      objectName: serviceCard.phoneService ? "intelligenceLocalPrice" : "intelligenceDefaultPrice"
      Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.text; font.bold: true
      text: serviceCard.modelData.price
     }
     Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: serviceCard.phoneService && workspace.caseReady ? desktopTokens.success : desktopTokens.danger
      text: serviceCard.modelData.status
     }
     OsintDesktopCommand {
      id: searchAction
      objectName: serviceCard.phoneService ? "intelligenceOpenPhoneSearch" : "intelligenceStartPaidSearch"
      primary: serviceCard.phoneService
      enabled: serviceCard.phoneService && workspace.caseReady
      text: serviceCard.phoneService && workspace.showPhoneSearch
          ? workspace.app.trx("إغلاق نموذج البحث", "Close search form")
          : workspace.app.trx("بدء البحث", "Start search")
      onClicked: workspace.showPhoneSearch = !workspace.showPhoneSearch
     }
    }
   }
  }
 }
 Loader {
  Layout.fillWidth: true
  active: workspace.showPhoneSearch && workspace.caseReady
  visible: active
  sourceComponent: PhoneOsintPanel { app: desktopApp }
 }
 OsintDesktopCommand {
  objectName: "intelligenceAccountPermissions"
  text: workspace.app.trx("الحسابات والصلاحيات", "Accounts and permissions")
  onClicked: workspace.app.openAccountPermissions()
 }
 OsintDesktopCommand {
  objectName: "intelligenceAdvancedSettings"
  visible: workspace.administrator
  text: workspace.showAdvanced
      ? workspace.app.trx("إغلاق الإعدادات المتقدمة", "Close advanced settings")
      : workspace.app.trx("الإعدادات المتقدمة", "Advanced settings")
  onClicked: workspace.showAdvanced = !workspace.showAdvanced
 }
 Loader {
  active: workspace.administrator && workspace.showAdvanced
  visible: active
  Layout.fillWidth: true
  sourceComponent: LampyreConnectionPanel { app: desktopApp }
 }
 CheckBox {
  objectName: "osintDemoMode"
  text: workspace.app.trx("عرض تجريبي ببيانات اختبار", "Demonstration with test data")
  checked: workspace.demoMode
  onToggled: workspace.demoMode = checked
 }
 Loader {
  active: workspace.demoMode
  visible: active
  Layout.fillWidth: true
  sourceComponent: ColumnLayout {
   spacing: 8
   OsintDesktopCommand {
    text: workspace.activeTab === 2
        ? workspace.app.trx("العودة إلى المثال", "Back to example")
        : workspace.app.trx("نتائج المثال", "Example results")
    onClicked: workspace.activeTab = workspace.activeTab === 2 ? 1 : 2
   }
   OsintRequestDesk {
    Layout.fillWidth: true
    app: desktopApp
    demoMode: true
    visible: workspace.activeTab === 1
   }
   OsintAnalysisPane {
    Layout.fillWidth: true
    app: desktopApp
    demoMode: true
    visible: workspace.activeTab === 2
    showInspector: workspace.showInspector
    showRequests: workspace.showRequests
   }
  }
 }
}
