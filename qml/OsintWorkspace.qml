import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:workspace;objectName:"osintWorkspace"
 property var app
 property int activeTab:2
 property int selectedProvider:0
 property int ribbonTab:0
 property bool showInspector:true
 property bool showRequests:true
 readonly property var evaluation:typeof lampyreEvaluation!=="undefined"?lampyreEvaluation:null
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
 }
 LayoutMirroring.enabled:app.ar;LayoutMirroring.childrenInherit:true;spacing:0
 Rectangle {
  Layout.fillWidth:true;implicitHeight:33;color:"#040507";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.margins:7;spacing:8
   Image {source:"qrc:/assets/icons/search_visual.svg";Layout.preferredWidth:19;Layout.preferredHeight:19;sourceSize:Qt.size(24,24)}
   Label {text:"NEXVARY MR  /  "+workspace.app.trx("مساحة التحقيق", "Investigation workspace");font.pixelSize:13;font.bold:true;color:"#E5EDF3";Layout.fillWidth:true}
   Label {text:workspace.app.trx("محلي • غير متصل", "Local • Disconnected");font.pixelSize:11;color:"#9EB4C1"}
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
     OsintDesktopCommand {iconName:"document_text";text:workspace.app.trx("مثال جديد", "New example");enabled:!!workspace.evaluation&&!workspace.evaluation.busy;onClicked:{workspace.evaluation.runSynthetic(0);workspace.activeTab=2}}
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
 OsintAnalysisPane {id:analysis;Layout.fillWidth:true;app:desktopApp;visible:workspace.activeTab===0||workspace.activeTab===2;showInspector:workspace.showInspector;showRequests:workspace.showRequests}
 OsintRequestDesk {Layout.fillWidth:true;app:desktopApp;visible:workspace.activeTab===1;selectedProvider:workspace.selectedProvider;onSelectedProviderChanged:workspace.selectedProvider=selectedProvider}
 OsintDesktopPane {
  Layout.fillWidth:true;visible:workspace.activeTab===3;title:workspace.app.trx("الاستخدام والتفويض", "Usage & authorization");iconName:"history";Layout.preferredHeight:350
  Label {text:workspace.app.trx("حساب Lampyre غير متصل. الرصيد والتكلفة الفعلية غير متاحين.", "Lampyre account disconnected. Actual balance and pricing are unavailable.");color:desktopTokens.text;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Label {text:workspace.app.trx("المثال المحلي: 0 Photon. لا توجد طلبات مدفوعة أو سجل حساب متزامن.", "Local example: 0 Photons. No paid requests or synchronized account history.");color:desktopTokens.muted;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Item {Layout.fillHeight:true}
 }
 Rectangle {
  Layout.fillWidth:true;implicitHeight:25;color:"#0D2A3D";border.color:"#24556D"
  RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8
   Label {text:workspace.app.trx("مثال اصطناعي • ليس دليلًا", "Synthetic example • Not evidence");font.pixelSize:10;color:"#9EB4C1";Layout.fillWidth:true}
   Label {text:workspace.app.trx("الرصيد: غير متاح", "Balance: unavailable");font.pixelSize:10;color:"#9EB4C1"}
  }
 }
}
