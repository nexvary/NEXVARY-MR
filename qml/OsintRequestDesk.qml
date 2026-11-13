import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
 id:desk
 property var app
 property int selectedProvider:0
 property bool demoMode:false
 property int scenario:0
 readonly property var bridge:typeof lampyreEvaluation!=="undefined"?lampyreEvaluation:null
 padding:8
 background:OsintBackdrop {dimming:0.24}
 contentItem:ColumnLayout {spacing:8
 RowLayout {visible:!desk.demoMode;Layout.fillWidth:true;Layout.margins:6
  Label {text:desk.app.trx("المزوّد", "Provider");color:"#9EB4C1";font.pixelSize:12}
  ComboBox {objectName:"osintProviderSelector";model:["NEXVARY Intelligence", "PhoneInfoga", "OSINT Industries"];currentIndex:desk.selectedProvider;onActivated:desk.selectedProvider=currentIndex;Layout.preferredWidth:240;palette.base:"#08121B";palette.text:"#E5EDF3";palette.button:"#0D2A3D";palette.buttonText:"#E5EDF3"}
  Item {Layout.fillWidth:true}
  Label {text:desk.app.trx("التجربة غير مؤكدة • لا اتصال API", "Trial unconfirmed • No API connection");color:"#9EB4C1";font.pixelSize:11;visible:desk.selectedProvider===0&&desk.demoMode}
 }
 LampyreConnectionPanel {app:desk.app;Layout.fillWidth:true;visible:desk.selectedProvider===0&&!desk.demoMode}
 GridLayout {
  objectName:"lampyrePanel";visible:desk.selectedProvider===0&&desk.demoMode
  Layout.fillWidth:true;columns:width>900?3:1;columnSpacing:4;rowSpacing:4
  OsintDesktopPane {
   Layout.preferredWidth:220;Layout.fillWidth:desk.width<=900;Layout.fillHeight:true
   title:desk.app.trx("معيار الإدخال", "Criteria");iconName:"search_visual"
   Rectangle {Layout.fillWidth:true;implicitHeight:28;color:"#0C435C"
    Label {anchors.fill:parent;anchors.margins:6;text:desk.app.trx("مثال محلي", "Local example");color:"#E5EDF3";font.pixelSize:12;font.bold:true}
   }
   RowLayout {SemanticIcon {iconSource:"qrc:/assets/icons-professional/document_text.svg";Layout.preferredWidth:17;Layout.preferredHeight:17}Label {text:"Email";color:"#E5EDF3";font.pixelSize:12}}
   Label {text:desk.app.trx("هذا معيار لبيانات الاختبار.", "This input belongs to the test example.");color:"#9EB4C1";Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:11}
   Item {Layout.fillHeight:true}
  }
  OsintDesktopPane {
   Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0
   title:desk.app.trx("المهام", "Tasks");iconName:"clipboard_task"
   Label {text:desk.app.trx("اختبارات محلية", "Local validation tasks");font.bold:true;font.pixelSize:12;color:"#9EB4C1"}
   Repeater {model:desk.app.ar?["نجاح وبيانات اصطناعية", "فشل المصادقة", "انتهاء المهلة", "رصيد غير كافٍ", "حد الطلبات"]:["Success with synthetic data", "Authentication failure", "Timeout", "Insufficient credits", "Rate limited"]
    delegate:Rectangle {required property int index;required property string modelData;Layout.fillWidth:true;implicitHeight:34;color:desk.scenario===index?"#0C435C":index%2?"#0A2231":"#08121B";border.color:desk.scenario===index?"#23B9E7":"transparent"
     RowLayout {anchors.fill:parent;anchors.margins:6;spacing:7
      SemanticIcon {iconSource:"qrc:/assets/icons-professional/"+(desk.scenario===index?"checkmark_circle":"document_text")+".svg";Layout.preferredWidth:17;Layout.preferredHeight:17}
      Label {text:modelData;color:"#E5EDF3";font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideRight}
     }
     MouseArea {anchors.fill:parent;onClicked:desk.scenario=parent.index}
     activeFocusOnTab:true;Keys.onReturnPressed:desk.scenario=index;Accessible.role:Accessible.Button;Accessible.name:modelData
    }
   }
   Item {Layout.fillHeight:true}
  }
  OsintDesktopPane {
   Layout.preferredWidth:285;Layout.fillWidth:desk.width<=900;Layout.fillHeight:true
   title:desk.app.trx("المعاملات والوصف", "Parameters & description");iconName:"settings"
   Label {text:desk.app.trx("عام", "General");font.bold:true;color:"#E5EDF3";font.pixelSize:12}
   Label {text:"Email";color:"#9EB4C1";font.pixelSize:11}
   EntryField {Layout.fillWidth:true;text:"research@example.invalid";readOnly:true;selectByMouse:true;color:"#E5EDF3";palette.base:"#08121B";font.pixelSize:12}
   Label {text:desk.app.trx("المصدر: مثال محلي ثابت. لا يُرسل أي عنوان إلى المزوّد. البيانات ليست دليلًا.", "Source: fixed local example. No address is sent to the provider. Data is not evidence.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#9EB4C1";font.pixelSize:12}
   Label {text:desk.app.trx("التكلفة الفعلية: غير متاحة", "Live cost: unavailable");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#9EB4C1";font.pixelSize:12}
   Item {Layout.fillHeight:true}
  }
  Layout.preferredHeight:440
 }
 RowLayout {visible:desk.selectedProvider===0&&desk.demoMode;Layout.fillWidth:true;Layout.margins:6
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#9EB4C1";font.pixelSize:11;text:!desk.bridge?desk.app.trx("المختبر غير متاح", "Laboratory unavailable"):desk.bridge.busy?desk.app.trx("جارٍ تشغيل المثال…", "Running fixture…"):desk.app.trx("المثال دون تكلفة • النتائج في تبويب التحقيق", "No-cost example • Results in the investigation tab")}
  OsintDesktopCommand {objectName:"lampyreCancelSynthetic";text:desk.app.trx("إلغاء", "Cancel");enabled:!!desk.bridge&&desk.bridge.busy;onClicked:desk.bridge.cancel()}
  OsintDesktopCommand {objectName:"lampyreRunSynthetic";iconName:"checkmark_circle";primary:true;text:desk.app.trx("تنفيذ المثال", "Execute example");enabled:!!desk.bridge&&!desk.bridge.busy;onClicked:desk.bridge.runSynthetic(desk.scenario)}
 }
 PhoneOsintPanel {app:desk.app;Layout.fillWidth:true;visible:desk.selectedProvider===1}
 OsintDesktopPane {visible:desk.selectedProvider===2;Layout.fillWidth:true;Layout.preferredHeight:250;title:"OSINT Industries";iconName:"document_lock"
  Label {text:desk.app.trx("المزوّد غير مفعّل. يلزم التحقق من الترخيص والتوثيق والتكلفة قبل إتاحة البحث.", "Provider disabled pending licensing, documentation and price verification.");color:"#9EB4C1";Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Item {Layout.fillHeight:true}
 }
 }
}
