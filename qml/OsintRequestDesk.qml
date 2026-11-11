import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
 id:desk
 property var app
 property int selectedProvider:0
 property int scenario:0
 readonly property var bridge:typeof lampyreEvaluation!=="undefined"?lampyreEvaluation:null
 padding:8
 background:Rectangle {color:"#F0F2F4"}
 contentItem:ColumnLayout {spacing:8
 RowLayout {Layout.fillWidth:true;Layout.margins:6
  Label {text:desk.app.trx("المزوّد", "Provider");color:"#536470";font.pixelSize:12}
  ComboBox {objectName:"osintProviderSelector";model:["Lampyre", "PhoneInfoga", "OSINT Industries"];currentIndex:desk.selectedProvider;onActivated:desk.selectedProvider=currentIndex;Layout.preferredWidth:240;palette.base:"#FFFFFF";palette.text:"#25333E";palette.button:"#EEF1F4";palette.buttonText:"#25333E"}
  Item {Layout.fillWidth:true}
  Label {text:desk.app.trx("التجربة غير مؤكدة • لا اتصال API", "Trial unconfirmed • No API connection");color:"#71808A";font.pixelSize:11;visible:desk.selectedProvider===0}
 }
 GridLayout {
  objectName:"lampyrePanel";visible:desk.selectedProvider===0
  Layout.fillWidth:true;columns:width>900?3:1;columnSpacing:4;rowSpacing:4
  OsintDesktopPane {
   Layout.preferredWidth:220;Layout.fillWidth:desk.width<=900;Layout.fillHeight:true
   title:desk.app.trx("معيار الإدخال", "Criteria");iconName:"search_visual"
   Rectangle {Layout.fillWidth:true;implicitHeight:28;color:"#DCEBD5"
    Label {anchors.fill:parent;anchors.margins:6;text:desk.app.trx("▾  مثال محلي", "▾  Local example");color:"#2E4430";font.pixelSize:12;font.bold:true}
   }
   RowLayout {Image {source:"qrc:/assets/icons/document_text.svg";Layout.preferredWidth:17;Layout.preferredHeight:17}Label {text:"Email";color:"#25333E";font.pixelSize:12}}
   Label {text:desk.app.trx("هذا معيار المثال الاصطناعي؛ لا يمثّل كتالوج Lampyre الفعلي.", "This is the synthetic example criterion, not Lampyre's live catalog.");color:"#74818B";Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:11}
   Item {Layout.fillHeight:true}
  }
  OsintDesktopPane {
   Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0
   title:desk.app.trx("المهام", "Tasks");iconName:"clipboard_task"
   Label {text:desk.app.trx("اختبارات محلية", "Local validation tasks");font.bold:true;font.pixelSize:12;color:"#536470"}
   Repeater {model:desk.app.ar?["نجاح وبيانات اصطناعية", "فشل المصادقة", "انتهاء المهلة", "رصيد غير كافٍ", "حد الطلبات"]:["Success with synthetic data", "Authentication failure", "Timeout", "Insufficient credits", "Rate limited"]
    delegate:Rectangle {required property int index;required property string modelData;Layout.fillWidth:true;implicitHeight:34;color:desk.scenario===index?"#DCEBD5":index%2?"#F5F7F8":"#FFFFFF";border.color:desk.scenario===index?"#9BBC8B":"transparent"
     RowLayout {anchors.fill:parent;anchors.margins:6;spacing:7
      Image {source:"qrc:/assets/icons/"+(desk.scenario===index?"checkmark_circle":"document_text")+".svg";Layout.preferredWidth:17;Layout.preferredHeight:17}
      Label {text:modelData;color:"#25333E";font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideRight}
      Label {text:"0 Ph";color:"#4A7E3E";font.pixelSize:11}
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
   Label {text:desk.app.trx("عام", "General");font.bold:true;color:"#25333E";font.pixelSize:12}
   Label {text:"Email";color:"#697984";font.pixelSize:11}
   TextField {Layout.fillWidth:true;text:"research@example.invalid";readOnly:true;selectByMouse:true;color:"#25333E";palette.base:"#FFFFFF";font.pixelSize:12}
   Label {text:desk.app.trx("المصدر: مثال محلي ثابت. لا يُرسل أي عنوان إلى المزوّد. البيانات ليست دليلًا.", "Source: fixed local example. No address is sent to the provider. Data is not evidence.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#687983";font.pixelSize:12}
   Label {text:desk.app.trx("التكلفة الفعلية: غير متاحة", "Live cost: unavailable");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#687983";font.pixelSize:12}
   Item {Layout.fillHeight:true}
  }
  Layout.preferredHeight:440
 }
 RowLayout {visible:desk.selectedProvider===0;Layout.fillWidth:true;Layout.margins:6
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#687983";font.pixelSize:11;text:!desk.bridge?desk.app.trx("المختبر غير متاح", "Laboratory unavailable"):desk.bridge.busy?desk.app.trx("جارٍ تشغيل المثال…", "Running fixture…"):desk.app.trx("المثال دون تكلفة • النتائج في تبويب التحقيق", "No-cost example • Results in the investigation tab")}
  OsintDesktopCommand {objectName:"lampyreCancelSynthetic";text:desk.app.trx("إلغاء", "Cancel");enabled:!!desk.bridge&&desk.bridge.busy;onClicked:desk.bridge.cancel()}
  OsintDesktopCommand {objectName:"lampyreRunSynthetic";iconName:"checkmark_circle";primary:true;text:desk.app.trx("تنفيذ المثال", "Execute example");enabled:!!desk.bridge&&!desk.bridge.busy;onClicked:desk.bridge.runSynthetic(desk.scenario)}
 }
 PhoneOsintPanel {app:desk.app;Layout.fillWidth:true;visible:desk.selectedProvider===1}
 OsintDesktopPane {visible:desk.selectedProvider===2;Layout.fillWidth:true;Layout.preferredHeight:250;title:"OSINT Industries";iconName:"document_lock"
  Label {text:desk.app.trx("المزوّد غير مفعّل. يلزم التحقق من الترخيص والتوثيق والتكلفة قبل إتاحة البحث.", "Provider disabled pending licensing, documentation and price verification.");color:"#536470";Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Item {Layout.fillHeight:true}
 }
 }
}
