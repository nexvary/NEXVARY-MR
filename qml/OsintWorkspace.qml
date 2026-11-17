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
  property bool showLocalTools:true
 property int activeTab: 1
 property bool showInspector: true
 property bool showRequests: true
 readonly property var evaluation: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 readonly property bool administrator: typeof identityBridge !== "undefined"
     && identityBridge.authenticated && identityBridge.session.role === "administrator"
 readonly property bool intelligenceOperator: typeof identityBridge !== "undefined"
     && identityBridge.authenticated
     && ["administrator", "investigator", "reviewer"].indexOf(identityBridge.session.role) >= 0
 property bool showDashboardDetails:false
 property string dashboardError:""
 readonly property var intelligenceConnection:typeof lampyreConnection!=="undefined"?lampyreConnection:null
 readonly property bool caseReader:typeof identityBridge!=="undefined"&&identityBridge.authenticated&&lawCaseBridge.permissions.read
 readonly property var readableCases:caseReader?lawCaseBridge.cases:[]
 readonly property var intelligenceHistory:typeof identityBridge!=="undefined"&&identityBridge.authenticated?operationPolicy.operationReceipts.filter(r=>["intelligence-mcp","lampyre-mcp","phone-osint"].indexOf(r.kind)>=0):[]
 Connections {target:identityBridge;function onChanged(){workspace.dashboardError=""}}
 readonly property bool caseReady: (typeof identityBridge!=="undefined"&&identityBridge.directWork) || typeof lawCaseBridge !== "undefined"
     && !!lawCaseBridge.activeCase.id && lawCaseBridge.permissions.analyze
     && lawCaseBridge.permissions.addEvidence

 onCaseReadyChanged: {
  if (!caseReady)
   showPhoneSearch = false
 }

 onIntelligenceOperatorChanged: {
  if (!intelligenceOperator)
   showAdvanced = false
 }

 onDemoModeChanged: {
  if (!demoMode && evaluation)
   evaluation.clear()
  activeTab = 1
 }
 onVisibleChanged: {
  if(visible&&intelligenceConnection&&intelligenceConnection.liveProfile&&intelligenceConnection.hasCredential&&!intelligenceConnection.lookupConnected&&!intelligenceConnection.busy)intelligenceConnection.verifyLookupConnection(true)
  if (!visible) {
   showAdvanced = false
   showDashboardDetails = false
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
  readonly property real intelligenceViewportHeight:workspace.app.intelligenceViewportHeight
  readonly property var design:desktopTokens
  function trx(a,e){return workspace.app.trx(a,e)}
  function openExternal(url){return workspace.app.openExternal(url)}
  function humanError(error){return workspace.app.humanError(error)}
  readonly property bool governmentMode:workspace.app.governmentMode
  function roleLabel(role){return workspace.app.roleLabel(role)}
  function openAccountPermissions(){workspace.app.openAccountPermissions()}
 }

 IntelligenceSearchCards {app:desktopApp;Layout.fillWidth:true;onSetupRequested:workspace.showAdvanced=true}
 Label {
  Layout.fillWidth: true
  color: desktopTokens.text
  font.pixelSize:24
  font.bold: true
  visible:false
  text: "NEXVARY Intelligence"
 }
 Label {
  Layout.fillWidth: true
  wrapMode: Text.WordWrap
  color: desktopTokens.muted
  visible:false
  text: workspace.app.trx("خدمات البحث والتحليل", "Search and analysis services")
 }
 GridLayout {
  objectName: "intelligenceDashboard"
  visible:workspace.showDashboardDetails
  Layout.fillWidth: true
  columns: width >= 700 ? 4 : 2
  Repeater {
   model: [
    {title: workspace.app.trx("القضايا النشطة المعروضة", "Shown active cases"), value: workspace.readableCases.filter(c=>c.status!=="closed"&&c.status!=="archived").length},
    {title: workspace.app.trx("أدلة القضية المفتوحة", "Open-case evidence"), value: workspace.caseReader ? (lawCaseBridge.activeCase.evidence||[]).length : 0},
    {title: workspace.app.trx("الأدوات المكتشفة", "Discovered tools"), value: workspace.intelligenceConnection&&workspace.intelligenceConnection.connected ? workspace.intelligenceConnection.tools.length : 0},
    {title: workspace.app.trx("اتصال الاستخبارات", "Intelligence connection"), value: workspace.intelligenceConnection&&workspace.intelligenceConnection.connected ? workspace.app.trx("متحقق", "Verified") : workspace.app.trx("غير متحقق", "Not verified")}
   ]
   delegate: OsintDesktopPane {
    required property var modelData
    Layout.fillWidth: true;Layout.fillHeight:true;Layout.preferredWidth:1
    title: modelData.title;iconName:"shield_checkmark"
    Label {Layout.fillWidth:true;text:String(modelData.value);textFormat:Text.PlainText;color:desktopTokens.text;font.pixelSize:17;font.bold:true}
   }
  }
 }
 OsintDesktopCommand {
  objectName:"intelligenceDashboardDetails"
  text:workspace.showDashboardDetails?workspace.app.trx("إغلاق القضايا والسجل", "Close cases and history"):workspace.app.trx("القضايا وسجل الاستخبارات", "Cases and intelligence history")
  onClicked:workspace.showDashboardDetails=!workspace.showDashboardDetails
 }
 OsintDesktopPane {
  visible:workspace.showDashboardDetails
  Layout.fillWidth:true
  title:workspace.app.trx("القضايا والعمليات والأدلة", "Cases, operations and evidence");iconName:"history"
  RowLayout {
   Layout.fillWidth:true
   Label {objectName:"intelligenceActiveCase";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:desktopTokens.text;text:workspace.app.trx("القضية المفتوحة: ", "Open case: ")+(workspace.caseReader&&lawCaseBridge.activeCase.id?(lawCaseBridge.activeCase.metadata.caseNumber+" · "+lawCaseBridge.activeCase.id):workspace.app.trx("لا توجد قضية متاحة", "No accessible case"))}
   OsintDesktopCommand {objectName:"intelligenceRefreshCases";text:workspace.app.trx("تحديث القضايا", "Refresh cases");enabled:workspace.caseReader;onClicked:{const ok=lawCaseBridge.searchCases("");workspace.dashboardError=ok?"":workspace.app.humanError(lawCaseBridge.error)}}
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:desktopTokens.muted;text:workspace.app.trx("الأعداد من السجلات المحلية المتاحة لحسابك. حدّث القائمة لقراءة كل القضايا المسموح بها. بيانات المزوّد لا تُعد دليلًا موثّقًا تلقائيًا.", "Counts use local records accessible to your account. Refresh to read all permitted cases. A provider response is not automatically verified evidence.")}
  Repeater {
   model:workspace.readableCases.slice(0,12)
   delegate:OsintDesktopCommand {
    required property var modelData
    Layout.fillWidth:true;text:modelData.metadata.caseNumber+" · "+modelData.status
    onClicked:{const ok=lawCaseBridge.openCase(modelData.id);workspace.dashboardError=ok?"":workspace.app.humanError(lawCaseBridge.error)}
   }
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:desktopTokens.danger;text:workspace.dashboardError;visible:text!==""}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:desktopTokens.text;font.bold:true;text:workspace.app.trx("سجل عمليات الاستخبارات المسموح بعرضه", "Permitted intelligence operation history")}
  Label {visible:workspace.intelligenceHistory.length===0;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:desktopTokens.muted;text:workspace.app.trx("لا يوجد سجل متاح. يشمل السجل اختبارات الاتصال وطلبات البحث؛ معرّف العملية محلي وليس معرّف المزوّد.", "No permitted history available. History includes connection checks and searches; operation IDs are local, not provider IDs.")}
  Repeater {
   model:workspace.intelligenceHistory.slice(0,10)
   delegate:Label {
    required property var modelData
    Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:desktopTokens.muted
    text:workspace.app.trx("عملية محلية: ", "Local operation: ")+modelData.operationId+" · "+modelData.status+"\n"+(modelData.completedUtc||modelData.startedUtc)+"\n"+(modelData.outputSha256?"SHA-256: "+modelData.outputSha256:workspace.app.trx("لا توجد بصمة نتيجة معتمدة محليًا", "No locally retained result digest"))
   }
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:desktopTokens.text;font.bold:true;text:workspace.app.trx("قدرات الاتصال الحالي", "Current connection capabilities")}
  Repeater {
   model:workspace.intelligenceConnection&&workspace.intelligenceConnection.connected?workspace.intelligenceConnection.tools:[]
   delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:desktopTokens.muted;text:workspace.intelligenceConnection.displayText(modelData.title||modelData.name)}
  }
 }

  OsintDesktopCommand {text:workspace.showLocalTools?workspace.app.trx("إخفاء خدمات الاستخبارات","Hide intelligence services"):workspace.app.trx("عرض خدمات الاستخبارات","Show intelligence services");onClicked:workspace.showLocalTools=!workspace.showLocalTools}
 GridLayout {
  visible:workspace.showLocalTools
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
         : workspace.app.trx("افتح قضية لحفظ التقرير أو استخدم البحث المباشر", "Open a case to retain evidence or use direct search")
    },
    {
     title: workspace.app.trx("البحث الموحد عبر نيكسفاري", "NEXVARY unified search"),
     description: workspace.app.trx("ابحث بالبريد الإلكتروني أو الهاتف أو اسم المستخدم عبر المصادر المتاحة. يُظهر سعر €14.00 قبل التنفيذ.",
                                    "Search by email, phone or username across available sources. The €14.00 price is shown before execution."),
     input: workspace.app.trx("الإدخال: بريد إلكتروني، هاتف دولي، أو اسم مستخدم", "Input: email, international phone, or username"),
     price: workspace.app.trx("سعر نيكسفاري الموحد: €14.00 لكل عملية بحث", "NEXVARY fixed price: €14.00 per search"),
     icon: "search_visual",
     status: workspace.intelligenceConnection&&workspace.intelligenceConnection.lookupConnected
         ? workspace.app.trx("البحث الموحد متحقق — اختر فئة لبدء البحث", "Unified search verified — choose a category to start")
         : workspace.intelligenceConnection&&workspace.intelligenceConnection.connected
         ? workspace.app.trx("خدمات متخصصة متاحة؛ البحث الموحد غير متحقق", "Specialized services available; unified search not verified")
         : workspace.app.trx("يلزم إعداد اتصال الاستخبارات", "Intelligence connection setup required")
    }
   ]
   delegate: Pane {
    id: serviceCard
    required property int index
    required property var modelData
    readonly property bool phoneService: index === 0
    readonly property bool cardActive: phoneService ? workspace.caseReady : workspace.intelligenceConnection && workspace.intelligenceConnection.lookupConnected
    objectName: phoneService ? "intelligencePhoneCard" : "intelligenceBillingCard"
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 1
    Layout.minimumWidth: 0
    padding: 16
    background: Rectangle {
     radius: 8
     color: desktopTokens.raised
     border.color: serviceCard.cardActive ? desktopTokens.accent : desktopTokens.border
     border.width: 1
    }
    contentItem: ColumnLayout {
     spacing: 8
     RowLayout {
      Layout.fillWidth: true
      SemanticIcon {
       iconSource: "qrc:/assets/icons-professional/" + serviceCard.modelData.icon + ".svg"
       Layout.preferredWidth: 26
       Layout.preferredHeight: 26
      }
      Label {
       Layout.fillWidth: true
       wrapMode: Text.WordWrap
       color: desktopTokens.text
       font.bold: true
       font.pixelSize: 15
       text: serviceCard.modelData.title
      }
     }
     Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.muted; font.pixelSize: 13; text: serviceCard.modelData.description }
     Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.muted; font.pixelSize: 12; text: serviceCard.modelData.input }
     Item { Layout.fillHeight: true; Layout.minimumHeight: 4 }
     Label {
      objectName: serviceCard.phoneService ? "intelligenceLocalPrice" : "intelligenceDefaultPrice"
      Layout.fillWidth: true; wrapMode: Text.WordWrap; color: desktopTokens.accent; font.bold: true; font.pixelSize: 13
      text: serviceCard.modelData.price
     }
     Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      font.pixelSize: 12
      color: serviceCard.cardActive ? desktopTokens.success : desktopTokens.muted
      text: serviceCard.modelData.status
     }
     OsintDesktopCommand {
      id: searchAction
      objectName: serviceCard.phoneService ? "intelligenceOpenPhoneSearch" : "intelligenceStartPaidSearch"
      primary: serviceCard.cardActive
      enabled: serviceCard.phoneService ? workspace.caseReady : workspace.intelligenceOperator && !!workspace.intelligenceConnection && workspace.intelligenceConnection.lookupConnected
      text: serviceCard.phoneService && workspace.showPhoneSearch
          ? workspace.app.trx("إغلاق نموذج البحث", "Close search form")
          : workspace.app.trx("بدء البحث", "Start search")
      onClicked: {if(serviceCard.phoneService)workspace.showPhoneSearch = !workspace.showPhoneSearch;else workspace.showAdvanced=true}
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
  visible:!identityBridge.directWork
  text: workspace.app.trx("الحسابات والصلاحيات", "Accounts and permissions")
  onClicked: workspace.app.openAccountPermissions()
 }
 OsintDesktopCommand {
  objectName: "intelligenceAdvancedSettings"
  visible: workspace.intelligenceOperator
  text: workspace.showAdvanced
      ? workspace.app.trx("إغلاق إعداد الاستخبارات", "Close Intelligence setup")
      : workspace.app.trx("إعداد الاستخبارات والبحث", "Intelligence setup and search")
  onClicked: workspace.showAdvanced = !workspace.showAdvanced
 }
 Loader {
  active: workspace.intelligenceOperator && workspace.showAdvanced
  visible: active
  Layout.fillWidth: true
  sourceComponent: LampyreConnectionPanel { app: desktopApp }
 }
 NeonCheckBox {
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
