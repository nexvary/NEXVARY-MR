import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
 id:window
 objectName:"mainWindow"
 width:1536;height:980;minimumWidth:1180;minimumHeight:760;visible:true
 title:"NEXVARY MR — Mobile Repair Workstation"
 color:"#040f1b"
 property bool ar:true
 property int currentPage:0
 property bool allowClose:false
 property string toastText:""
 property bool toastOk:true
 property var states:["Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"]
 property var statesAr:["تم الاستلام","التشخيص","انتظار القطع","قيد الإصلاح","جاهز","تم التسليم","ملغي"]
 property var titles:ar?["مركز العمليات","الأجهزة والاتصال","تشخيص الجهاز","مخططات اللوحات","فحص ملفات النظام","أوامر الصيانة","قطع الغيار","جاهزية وإعدادات"]:["Operations center","Devices & USB","Diagnostics","Board workspace","Firmware preflight","Repair orders","Parts inventory","Readiness & settings"]
 function trx(a,e){return ar?a:e}
 function orderStatus(s){let i=states.indexOf(s);return ar&&i>=0?statesAr[i]:s}
 function notice(ok,message){toastOk=ok;toastText=ok?message:workshopBridge.error;toastTimer.restart()}
 function navigate(index){currentPage=index}
 function navigationButton(index){return navigation.itemAt(index)}
 palette.window:"#081825";palette.windowText:"#e0ecfa";palette.base:"#0c2131";palette.alternateBase:"#102a3e";palette.text:"#dae8f7";palette.button:"#122d43";palette.buttonText:"#e0ecff";palette.highlight:"#176df0";palette.highlightedText:"white";palette.mid:"#27435b";palette.dark:"#081320";palette.light:"#34536d";palette.placeholderText:"#6f8ca5"
 font.family:"Segoe UI";font.pixelSize:14
 LayoutMirroring.enabled:ar
 LayoutMirroring.childrenInherit:true
 Shortcut {sequence:"Ctrl+1";onActivated:window.navigate(0)}
 Shortcut {sequence:"Ctrl+2";onActivated:window.navigate(1)}
 Shortcut {sequence:"Ctrl+3";onActivated:window.navigate(2)}
 Shortcut {sequence:"Ctrl+4";onActivated:window.navigate(3)}
 Shortcut {sequence:"Ctrl+R";onActivated:if(!toolBridge.busy)toolBridge.scanAdb()}
 Shortcut {sequence:"Ctrl+Q";onActivated:window.close()}
 onClosing:function(close){if(!allowClose){close.accepted=false;exitDialog.open()}}
 Dialog {
  id:exitDialog;width:480;objectName:"exitDialog";anchors.centerIn:parent;modal:true;title:window.trx("إغلاق NEXVARY MR","Close NEXVARY MR");standardButtons:Dialog.Yes|Dialog.No
  Label {width:430;wrapMode:Text.WordWrap;color:"#cfdfed";text:window.trx("هل تريد إغلاق البرنامج؟ ستُلغى الفحوص الجارية. احفظ جلسة المخطط والنماذج المفتوحة أولًا؛ أوامر الصيانة المحفوظة محفوظة محليًا.","Close the application? Running checks will be cancelled. Save board sessions and open forms first; saved repair orders are stored locally.")}
  onAccepted:{toolBridge.cancel();firmwareBridge.cancel();window.allowClose=true;window.close()}
 }
 ColumnLayout {
  anchors.fill:parent;spacing:0
  Rectangle {
   Layout.fillWidth:true;Layout.preferredHeight:80;color:"#071421";border.color:"#193248"
   RowLayout {
    anchors.fill:parent;anchors.margins:18;spacing:18
    Image {source:"qrc:/assets/nexvary-mr.png";Layout.preferredWidth:48;Layout.preferredHeight:48;fillMode:Image.PreserveAspectFit}
    ColumnLayout {spacing:1;Label{text:"Nexvary MR";font.pixelSize:28;font.bold:true;color:"#f2f6ff"}Label{text:"MOBILE REPAIR WORKSTATION";font.pixelSize:10;font.letterSpacing:1.7;color:"#829bb5"}}
    Item {Layout.fillWidth:true}
    Label {text:window.trx("مساحة عمل هندسية محلية","LOCAL ENGINEERING WORKSPACE");color:"#7297b7";font.pixelSize:12}
    Rectangle {Layout.preferredWidth:1;Layout.fillHeight:true;color:"#1b3449"}
    ActionButton {objectName:"languageButton";text:window.ar?"English":"العربية";onClicked:window.ar=!window.ar}
    ActionButton {objectName:"exitButton";text:window.trx("خروج","Exit");onClicked:window.close()}
   }
  }
  Rectangle {
   Layout.fillWidth:true;Layout.preferredHeight:87;color:"#06111d"
   RowLayout {
    anchors.fill:parent;anchors.margins:9;spacing:5
    Repeater {
     id:navigation
     model:window.titles
     delegate:Button {
      required property int index
      required property string modelData
      objectName:"nav"+index
      Layout.fillWidth:true;Layout.fillHeight:true;hoverEnabled:true
      onClicked:window.navigate(index)
      background:Rectangle{radius:6;color:window.currentPage===index?"#0d4dba":parent.hovered?"#122c41":"#0a1c2c";border.color:window.currentPage===index?"#2d7ef0":"#142f46"}
      contentItem:ColumnLayout {spacing:5;NavIcon{Layout.alignment:Qt.AlignHCenter;kind:index;stroke:window.currentPage===index?"#fff":"#8bb9e8"}Label{Layout.alignment:Qt.AlignHCenter;text:modelData;color:"#d6e7fc";font.pixelSize:12;elide:Text.ElideRight;Layout.maximumWidth:parent.width}}
     }
    }
   }
  }
  RowLayout {
   Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:10;spacing:10
   Panel {
    title:window.trx("الجهاز المحدد","Selected device");Layout.preferredWidth:224;Layout.fillHeight:true
    Rectangle {
     Layout.fillWidth:true;Layout.preferredHeight:155;color:"#07131f";radius:7;border.color:"#1b3850"
     Rectangle {anchors.centerIn:parent;width:66;height:116;radius:12;color:"#0b2947";border.width:2;border.color:toolBridge.selectedSerial?"#3989ff":"#31506a";Rectangle{anchors.top:parent.top;anchors.horizontalCenter:parent.horizontalCenter;anchors.topMargin:7;width:25;height:4;radius:2;color:"#5b8bb2"}Label{anchors.centerIn:parent;text:toolBridge.selectedSerial?"USB":"—";color:"#6fa8d9";font.pixelSize:18}}
    }
    Label {text:toolBridge.deviceInfo["ro.product.model"]||window.trx("لا يوجد هاتف محدد","No phone selected");color:"#e0edfc";font.bold:true;wrapMode:Text.WordWrap;Layout.fillWidth:true;textFormat:Text.PlainText}
    Label {text:toolBridge.selectedSerial||window.trx("ابدأ من الأجهزة والاتصال","Start with Devices & USB");color:"#8ba5bb";wrapMode:Text.WrapAnywhere;Layout.fillWidth:true;textFormat:Text.PlainText;font.pixelSize:12}
    Rectangle {Layout.fillWidth:true;height:1;color:"#1c374b"}
    Label {text:window.trx("عمليات سريعة","QUICK ACTIONS");color:"#68a8dd";font.pixelSize:12}
    ActionButton {text:window.trx("فحص الاتصال","Scan connection");Layout.fillWidth:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
    ActionButton {text:window.trx("قراءة المعلومات","Read properties");Layout.fillWidth:true;enabled:!toolBridge.busy&&toolBridge.selectedSerial!=="";onClicked:toolBridge.readDeviceInfo()}
    ActionButton {text:window.trx("عرض الشاشة","Mirror screen");Layout.fillWidth:true;enabled:!toolBridge.busy&&toolBridge.selectedSerial!=="";onClicked:toolBridge.startMirror()}
    ActionButton {text:window.trx("أمر صيانة جديد","Repair orders");Layout.fillWidth:true;onClicked:window.navigate(5)}
    ActionButton {text:window.trx("النسخ الاحتياطي","Backup & restore");Layout.fillWidth:true;onClicked:window.navigate(7)}
    Item {Layout.fillHeight:true}
    Label {text:window.trx("فحوص أولية للقراءة فقط\nلا توجد عمليات فتح حماية أو تعديل هوية","Read-only initial checks\nNo protection bypass or identity changes");color:"#6e8ca5";wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:11}
   }
   ColumnLayout {
    Layout.fillWidth:true;Layout.fillHeight:true;spacing:10
    RowLayout {Layout.fillWidth:true;Label{text:window.titles[window.currentPage];color:"#e6f0ff";font.pixelSize:23;font.bold:true}Item{Layout.fillWidth:true}Label{text:"0.3.0 / "+window.trx("تطوير","DEVELOPMENT");color:"#6d94b3";font.pixelSize:11}}
    StackLayout {
     id:pages;Layout.fillWidth:true;Layout.fillHeight:true;currentIndex:window.currentPage
     ColumnLayout {
      spacing:12
      RowLayout {
       Layout.fillWidth:true;spacing:12
       Repeater {
        model:[{label:window.trx("الأجهزة المكتشفة","Discovered devices"),value:toolBridge.devices.length,color:"#5faaff"},{label:window.trx("أوامر الصيانة","Repair orders"),value:workshopBridge.orders.length,color:"#4ed9b0"},{label:window.trx("أصناف المخزون","Inventory items"),value:workshopBridge.inventory.length,color:"#b29bff"}]
        delegate:Rectangle {required property var modelData;Layout.fillWidth:true;height:108;color:"#0b2133";radius:7;border.color:"#203d54";ColumnLayout{anchors.fill:parent;anchors.margins:16;Label{text:modelData.value;color:modelData.color;font.pixelSize:32;font.bold:true}Label{text:modelData.label;color:"#92aec7";font.pixelSize:13}}}
       }
      }
      Panel {
       title:window.trx("من الاستلام إلى التسليم","FROM INTAKE TO HANDOVER");Layout.fillWidth:true;Layout.preferredHeight:175
       Label {text:window.trx("ابدأ بفحص جاهزية الكمبيوتر، ثم اتصل بالجهاز، واجمع الأدلة قبل اتخاذ قرار الصيانة.","Check workstation readiness, connect the device, then collect evidence before choosing a repair.");color:"#a6bfd6";wrapMode:Text.WordWrap;Layout.fillWidth:true}
       RowLayout {Layout.fillWidth:true;ActionButton{text:window.trx("1  جاهزية الأدوات","1  Tool readiness");Layout.fillWidth:true;onClicked:window.navigate(7)}ActionButton{text:window.trx("2  اكتشاف الهاتف","2  Connect device");Layout.fillWidth:true;primary:true;onClicked:window.navigate(1)}ActionButton{text:window.trx("3  التشخيص","3  Diagnose");Layout.fillWidth:true;onClicked:window.navigate(2)}ActionButton{text:window.trx("4  توثيق الصيانة","4  Repair order");Layout.fillWidth:true;onClicked:window.navigate(5)}}
      }
      RowLayout {
       Layout.fillWidth:true;Layout.fillHeight:true;spacing:12
       Panel {
        title:window.trx("حالة بيئة العمل","WORKSTATION HEALTH");Layout.fillWidth:true;Layout.fillHeight:true
        Repeater {model:toolBridge.tools;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;Rectangle{width:7;height:7;radius:4;color:modelData.available?"#47dba6":"#edb56b"}Label{text:modelData.name;color:"#d2e5f7";Layout.fillWidth:true}Label{text:modelData.available?window.trx("متاح","Ready"):window.trx("غير مثبت","Missing");color:modelData.available?"#47dba6":"#edb56b"}}}
        Label {text:window.trx("SQLite: ","SQLite: ")+(workshopBridge.ready?window.trx("جاهزة","Ready"):workshopBridge.error);color:workshopBridge.ready?"#47dba6":"#edb56b";Layout.fillWidth:true;wrapMode:Text.WordWrap}
        Item{Layout.fillHeight:true}
       }
       Panel {
        title:window.trx("سجل الورشة الفعلي","WORKSHOP ACTIVITY");Layout.fillWidth:true;Layout.fillHeight:true
        ListView {Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:workshopBridge.events;delegate:Label{required property var modelData;width:ListView.view.width;text:modelData.created_at+"  •  "+modelData.action;color:"#9fbdd3";font.pixelSize:12;wrapMode:Text.WordWrap;textFormat:Text.PlainText}}
        Label {visible:workshopBridge.events.length===0;text:window.trx("لا توجد عمليات مسجلة بعد","No recorded operations yet");color:"#708da6"}
       }
      }
     }
     DevicesPage {app:window}
     DiagnosisPage {app:window}
     BoardWorkspace {app:window}
     FirmwarePage {app:window}
     WorkshopPage {app:window}
     InventoryPage {app:window}
     SettingsPage {app:window}
    }
    Rectangle {
     Layout.fillWidth:true;Layout.preferredHeight:66;color:"#061622";radius:6;border.color:"#1a354a"
     RowLayout {anchors.fill:parent;anchors.margins:12;Rectangle{width:8;height:8;radius:4;color:toolBridge.busy?"#ffbe60":"#429dca"}Label{text:toolBridge.status;Layout.fillWidth:true;maximumLineCount:2;elide:Text.ElideRight;wrapMode:Text.WordWrap;color:"#89abc5";font.pixelSize:12;textFormat:Text.PlainText}BusyIndicator{Layout.preferredWidth:28;Layout.preferredHeight:28;running:toolBridge.busy}}
    }
   }
  }
  Rectangle {Layout.fillWidth:true;Layout.preferredHeight:29;color:"#06121f";border.color:"#193245";RowLayout{anchors.fill:parent;anchors.leftMargin:18;anchors.rightMargin:18;Label{text:"NEXVARY MR";color:"#6f9cbe";font.pixelSize:11}Item{Layout.fillWidth:true}Label{text:window.trx("محلي · دون خدمة سحابية · اتصال مصرح به فقط","LOCAL · OFFLINE · AUTHORIZED DEVICES ONLY");color:"#527b9c";font.pixelSize:10}}}
 }
 Rectangle {
  anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:45;width:Math.min(680,parent.width-40);height:70;radius:8;visible:toastText!=="";color:toastOk?"#123a35":"#48252a";border.color:toastOk?"#36bc96":"#e0787b";z:100
  Label{anchors.fill:parent;anchors.margins:12;text:window.toastText;color:"#eff9ff";wrapMode:Text.WordWrap;textFormat:Text.PlainText;verticalAlignment:Text.AlignVCenter}
 }
 Timer{id:toastTimer;interval:6000;onTriggered:window.toastText=""}
}
