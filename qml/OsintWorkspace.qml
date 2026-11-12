import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:workspace;objectName:"osintWorkspace"
 property var app
 property bool demoMode:false
 onDemoModeChanged:{if(!demoMode&&evaluation)evaluation.clear();activeTab=1}
 property int activeTab:1
 property int selectedProvider:0
 property int ribbonTab:0
 property bool showInspector:true
 property bool showRequests:true
 readonly property var liveConnection:typeof lampyreConnection!=="undefined"?lampyreConnection:null
 readonly property var evaluation:typeof lampyreEvaluation!=="undefined"?lampyreEvaluation:null
 Label {
  Layout.fillWidth:true
  color:workspace.app.design.text;font.pixelSize:22;font.bold:true;wrapMode:Text.WordWrap
  text:workspace.app.trx("مرحبًا بك في قسم الاستخبارات وجمع المعلومات","Welcome to Intelligence and Information Gathering")
 }
 Label {
  Layout.fillWidth:true
  color:workspace.app.design.muted;wrapMode:Text.WordWrap
  text:workspace.app.trx("اختر نوع البحث الذي تريد تنفيذه","Choose the type of search you want to run")
 }
 QtObject {
  id:desktopTokens
  readonly property color text:"#E5EDF3"
  readonly property color muted:"#9EB4C1"
  readonly property color surface:"#08121B"
  readonly property color raised:"#0A2231"
  readonly property color border:"#24556D"
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
 LayoutMirroring.enabled:app.ar;LayoutMirroring.childrenInherit:true;spacing:0
 Rectangle {
  Layout.fillWidth:true;implicitHeight:33;color:"#040507";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.margins:7;spacing:8
   Image {source:"qrc:/assets/icons-professional/search_visual.svg";Layout.preferredWidth:19;Layout.preferredHeight:19;sourceSize:Qt.size(24,24)}
   Label {text:"NEXVARY MR  /  "+workspace.app.trx("مساحة التحقيق", "Investigation workspace");font.pixelSize:13;font.bold:true;color:"#E5EDF3";Layout.fillWidth:true}
   Label {text:workspace.demoMode?workspace.app.trx("وضع عرض تجريبي", "Demo mode"):workspace.liveConnection&&workspace.liveConnection.connected?workspace.app.trx("متصل بخادم MCP", "MCP server connected"):workspace.app.trx("اتصال الحساب من إعدادات المزود", "Configure the provider account below");font.pixelSize:11;color:"#9EB4C1"}
  }
 }
 Rectangle {
  Layout.fillWidth:true;implicitHeight:31;color:"#08121B";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8;spacing:2
   Repeater {model:workspace.app.ar?["الرئيسية", "التحليل", "العرض"]:["Home", "Analysis", "View"]
    delegate:OsintDesktopCommand {required property string modelData;required property int index;text:modelData;primary:workspace.ribbonTab===index;onClicked:workspace.ribbonTab=index}
   }
   Item {Layout.fillWidth:true}
   Label {text:"OSINT";font.pixelSize:11;font.bold:true;color:"#9EB4C1"}
  }
 }
 Rectangle {
  Layout.fillWidth:true;implicitHeight:89;color:"#08121B";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.margins:7;spacing:10
   ColumnLayout {spacing:5
    RowLayout {spacing:5
     OsintDesktopCommand {iconName:"document_text";text:workspace.app.trx("مثال جديد", "New example");visible:workspace.demoMode;enabled:!!workspace.evaluation&&!workspace.evaluation.busy;onClicked:{workspace.evaluation.runSynthetic(0);workspace.activeTab=2}}
     OsintDesktopCommand {iconName:"settings";text:workspace.app.trx("قائمة الطلبات", "Request catalog");primary:workspace.activeTab===1;onClicked:workspace.activeTab=1}
    }
    Label {text:workspace.app.trx("تحقيق محلي", "Local investigation");color:"#9EB4C1";font.pixelSize:10;Layout.alignment:Qt.AlignHCenter}
   }
   Rectangle {Layout.fillHeight:true;width:1;color:"#24556D"}
   ColumnLayout {spacing:5;visible:workspace.ribbonTab===0
    RowLayout {spacing:5
     OsintDesktopCommand {iconName:"database";text:workspace.app.trx("جدول", "Table");onClicked:{workspace.activeTab=2;analysis.viewMode=0}}
     OsintDesktopCommand {iconName:"board";text:workspace.app.trx("مخطط العلاقات", "Relationship schema");onClicked:{workspace.activeTab=2;analysis.viewMode=1}}
    }
    Label {text:workspace.app.trx("عرض البيانات", "Data visualization");color:"#9EB4C1";font.pixelSize:10;Layout.alignment:Qt.AlignHCenter}
   }
   ColumnLayout {spacing:5;visible:workspace.ribbonTab===1
    RowLayout {spacing:5
     OsintDesktopCommand {iconName:"arrow_sync";text:workspace.app.trx("عكس الترتيب", "Reverse sort");onClicked:{workspace.activeTab=2;analysis.viewMode=0;analysis.descending=!analysis.descending}}
     OsintDesktopCommand {iconName:"checkmark_circle";text:workspace.app.trx("إلغاء التحديد", "Clear selection");enabled:analysis.selectedIndex>=0;onClicked:analysis.selectedIndex=-1}
    }
    Label {text:workspace.app.trx("تحليل العناصر", "Entity analysis");color:"#9EB4C1";font.pixelSize:10;Layout.alignment:Qt.AlignHCenter}
   }
   ColumnLayout {spacing:5
    RowLayout {spacing:5
     OsintDesktopCommand {iconName:"history";objectName:"osintToggleRequests";text:workspace.app.trx("الطلبات", "Requests");primary:workspace.showRequests;onClicked:workspace.showRequests=!workspace.showRequests}
     OsintDesktopCommand {iconName:"document_lock";objectName:"osintToggleInspector";text:workspace.app.trx("الخصائص", "Properties");primary:workspace.showInspector;onClicked:workspace.showInspector=!workspace.showInspector}
    }
    Label {text:workspace.app.trx("الألواح", "Panels");color:"#9EB4C1";font.pixelSize:10;Layout.alignment:Qt.AlignHCenter}
   }
   Item {Layout.fillWidth:true}
  }
 }
 Rectangle {
  Layout.fillWidth:true;implicitHeight:34;color:"#0A2231";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.leftMargin:5;anchors.rightMargin:5;spacing:3
   Repeater {
    model:workspace.app.ar?["قائمة الطلبات", "نتائج التحقيق", "الاستخدام"]:["Request catalog", "Investigation results", "Usage"]
    delegate:OsintDesktopCommand {required property string modelData;required property int index;objectName:"osintTab"+(index+1);text:modelData;primary:workspace.activeTab===index+1;onClicked:workspace.activeTab=index+1;implicitHeight:28}
   }
   Item {Layout.fillWidth:true}
  }
 }
 CheckBox {objectName:"osintDemoMode";text:workspace.app.trx("وضع عرض تجريبي منفصل — بيانات اختبار فقط","Separate demonstration mode — test data only");checked:workspace.demoMode;onToggled:workspace.demoMode=checked;Layout.fillWidth:true}
 OsintAnalysisPane {id:analysis;demoMode:workspace.demoMode;Layout.fillWidth:true;app:desktopApp;visible:workspace.demoMode&&(workspace.activeTab===0||workspace.activeTab===2);showInspector:workspace.showInspector;showRequests:workspace.showRequests}
 OsintRequestDesk {Layout.fillWidth:true;app:desktopApp;demoMode:workspace.demoMode;visible:workspace.activeTab===1||!workspace.demoMode&&workspace.activeTab===2;selectedProvider:workspace.selectedProvider;onSelectedProviderChanged:workspace.selectedProvider=selectedProvider}
 OsintDesktopPane {
  Layout.fillWidth:true;visible:workspace.activeTab===3;title:workspace.app.trx("الاستخدام والتفويض", "Usage & authorization");iconName:"history";Layout.preferredHeight:350
  Label {text:workspace.app.trx("الرصيد وتكلفة الطلبات يُراجعان في حساب Lampyre قبل التنفيذ.","Review balance and request prices in the Lampyre account before execution.");color:desktopTokens.text;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Label {text:workspace.demoMode?workspace.app.trx("العرض التجريبي يعمل محليًا دون خصم رصيد.","The demonstration runs locally without credit consumption."):workspace.app.trx("راجع الرصيد والتكلفة في حساب المزود. لا يُعرض رصيد مفترض أو تقدير غير متحقق منه.","Check balance and prices in the provider account. No assumed balance or unverified quote is displayed.");color:desktopTokens.muted;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Item {Layout.fillHeight:true}
 }
 Rectangle {
  Layout.fillWidth:true;implicitHeight:25;color:"#0D2A3D";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8
   Label {text:workspace.demoMode?workspace.app.trx("عرض تجريبي منفصل", "Separate demonstration"):workspace.app.trx("لا يُنفّذ طلب دون اختيار وتأكيد", "No request runs without selection and confirmation");font.pixelSize:10;color:"#9EB4C1";Layout.fillWidth:true}
   Label {text:workspace.app.trx("الرصيد: غير متاح", "Balance: unavailable");font.pixelSize:10;color:"#9EB4C1"}
  }
 }
}
