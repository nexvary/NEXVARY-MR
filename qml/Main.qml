import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
 id:window
 objectName:"mainWindow"
 width:1536;height:980;minimumWidth:1180;minimumHeight:760;visible:true
 title:"NEXVARY MR — Mobile Repair Workstation"
 color:window.tone("#040f1b")
 property bool ar:true
 property bool darkTheme:true
 readonly property var lightColors:{"#040f1b":"#f0f3f7","#050e17":"#f0f4f7","#06121f":"#eff2f6","#061622":"#eef3f6","#07131f":"#eff2f6","#071421":"#eef2f6","#071623":"#eef2f6","#081320":"#eef2f6","#081825":"#edf2f6","#091b29":"#edf1f5","#0a1c2c":"#ecf1f5","#0b1e2d":"#ecf1f5","#0b2030":"#ebf0f4","#0b2131":"#ebf0f4","#0b2133":"#ebf0f4","#0b2234":"#eaf0f4","#0b2947":"#e7edf2","#0c2130":"#ebf0f4","#0c2131":"#ebf0f4","#0c2232":"#ebf0f4","#0c2a25":"#ecf5f3","#0d2538":"#e9eff3","#0d4dba":"#b4c4df","#101d29":"#ebf0f4","#10283a":"#e9eef3","#10293e":"#e8eef3","#102a3e":"#e8eef3","#102b3d":"#e8eef3","#122c41":"#e7edf2","#122d43":"#e7edf2","#123453":"#e4ebf0","#123a35":"#e8f3f1","#124b94":"#b4c7df","#125c4f":"#e2f0ed","#142f46":"#e6ecf1","#143c64":"#b4c9df","#154b7a":"#b4cbdf","#162434":"#e9edf3","#173b57":"#e2eaf0","#1767dc":"#0e418b","#176df0":"#093f90","#193245":"#e5ecf1","#193248":"#e5ebf1","#1a354a":"#e4ebf1","#1b3449":"#e4ebf1","#1b3850":"#e3eaf0","#1c374b":"#e4ebf0","#1d3b55":"#b4cbdf","#203d52":"#b4cddf","#203d54":"#b4ccdf","#233b4c":"#e2eaf0","#234a63":"#b4cedf","#244358":"#b4cedf","#24445a":"#b4cddf","#252514":"#f4f4eb","#27435b":"#b4cbdf","#282615":"#f4f3eb","#284b65":"#b4cddf","#29485f":"#b4cddf","#2a786f":"#b4dfda","#2d7ef0":"#0a418f","#307ef3":"#083f91","#31506a":"#b6cbdd","#31516a":"#b6ccdd","#34536d":"#b6cbdc","#356756":"#b8dbcf","#36bc96":"#22775f","#392329":"#f0e7e9","#3989ff":"#083f91","#40b9e6":"#126787","#4284ac":"#2f5d7a","#429dca":"#225b77","#465160":"#c1c8d2","#46d7a5":"#1b7e5c","#47dba6":"#19805b","#48252a":"#f0e3e5","#48dba5":"#19805a","#48dea4":"#178259","#4dd9a8":"#1b7e5b","#4de4bd":"#148568","#4ed9b0":"#1b7e61","#519cff":"#084391","#51d6a2":"#1d7c57","#527b9c":"#3a576e","#58d7ae":"#1e7b5d","#58dcb7":"#1b7e62","#5b8bb2":"#365773","#5e542c":"#ddd5b6","#5faaff":"#084891","#63afff":"#084b91","#63d7b1":"#1f7a5c","#64acff":"#084891","#64dbb0":"#1d7c5a","#66869d":"#425767","#68a8dd":"#1c517d","#68afff":"#084891","#69b7ff":"#084f91","#6bb8ec":"#115888","#6bb8ed":"#115888","#6d94b3":"#3a576f","#6e8ca5":"#405668","#6eb6ff":"#084c91","#6f8ca5":"#415668","#6f9cbe":"#345974","#6fa8d9":"#205079","#6fd9b5":"#20795b","#706032":"#ded3b5","#708ca2":"#425666","#708da6":"#415668","#70b7f0":"#0f538a","#71b8ee":"#105589","#75b7ff":"#084a91","#7696ad":"#3f5869","#7797ae":"#3f5869","#77b7e5":"#195580","#789ab4":"#3c576c","#78b7ff":"#1e2d3e","#7e9ab1":"#3f5669","#8192a5":"#465362","#819cb4":"#3f556a","#829bb5":"#3f546a","#839fb6":"#3e566a","#879fb7":"#3f5469","#87a5ba":"#3d586b","#89a3bb":"#3e556b","#89abc5":"#375871","#8ac7f5":"#1e303e","#8ba5bb":"#3e566a","#8bd6c0":"#28715c","#8cb2ce":"#325976","#8da6bc":"#3e566a","#8da8bd":"#3e576b","#8ea9bf":"#3d576b","#8eb6d6":"#295070","#8fbddd":"#245475","#90abc0":"#3d576b","#91a6bc":"#405469","#91b5d1":"#325877","#91bfff":"#1e2b3e","#92adc3":"#3c576d","#92aec7":"#39566f","#93a9bb":"#415667","#93adc4":"#3b566d","#93b8d7":"#295070","#99aec0":"#405668","#9ab8cd":"#385971","#9bbbd3":"#335975","#9ebbd3":"#355774","#9fb8ce":"#39566f","#9fbbcd":"#3a5a6f","#9fbdd3":"#355973","#a2bdd4":"#1e2f3e","#a5bbce":"#3b566d","#a5bfd3":"#1e303e","#a6bfd6":"#1e2f3e","#a6d9ff":"#1e303e","#a7bed0":"#20303c","#a8bed3":"#1f2e3d","#a8bfd4":"#1e2f3d","#a8c1d7":"#1e2f3e","#a9bfd2":"#202f3c","#adc1d3":"#202f3c","#b29bff":"#251e3e","#b3c8dc":"#1e2e3e","#b5cee4":"#1e2f3e","#b8d1e6":"#1e2f3e","#b9d4e8":"#1e303e","#bbd7ea":"#1e313e","#bd985b":"#785d30","#c6daeb":"#1e2f3e","#c7deef":"#1e303e","#c9aa78":"#785d30","#cfdfed":"#1e2f3e","#d0e0ef":"#1e2e3e","#d0e2f3":"#1e2e3e","#d2e5f7":"#1e2e3e","#d3e8fa":"#1e2f3e","#d4e5f5":"#1e2e3e","#d6e7fc":"#1e2c3e","#d8e8f7":"#1e2e3e","#d9e6f5":"#1e2d3e","#d9e8f2":"#1e313e","#dae8f7":"#1e2d3e","#dbe8f4":"#1e2f3e","#dbeaff":"#1e2b3e","#e0787b":"#7c1d1f","#e0b677":"#7d561c","#e0ecfa":"#1e2d3e","#e0ecff":"#1e2a3e","#e0edfc":"#1e2d3e","#e1c87b":"#7d651c","#e1ebf9":"#1e2b3e","#e3effa":"#1e2f3e","#e4efff":"#1e2b3e","#e6eef8":"#1e2c3e","#e6f0ff":"#1e2b3e","#e7f1ff":"#1e2b3e","#e8af68":"#855314","#e8f2ff":"#1e2c3e","#e9cb7d":"#836516","#eab775":"#855414","#ecb774":"#875312","#edb266":"#895410","#edb56b":"#885511","#edf4ff":"#1e2a3e","#edf5ff":"#1e2c3e","#eef5ff":"#1e2b3e","#eff6ff":"#1e2c3e","#eff9ff":"#1e323e","#f0b770":"#8a530f","#f0bb78":"#8a540f","#f0bc75":"#8a560f","#f1bd81":"#8a510f","#f2f6ff":"#1e283e","#f4b56f":"#8e500b","#f6bb67":"#905809","#ffae8c":"#3e271e","#ffbe60":"#915908"}
 function tone(value){return darkTheme?value:(lightColors[value]||value)}
 property bool showDevicePanel:false
 property var pageHistory:[]
 property int previousPage:0
 property bool goingBack:false
 onCurrentPageChanged:{if(!goingBack&&currentPage!==previousPage){let history=pageHistory.slice();history.push(previousPage);pageHistory=history.slice(-50)}previousPage=currentPage}
 function goBack(){if(pageHistory.length===0)return;let history=pageHistory.slice();let target=history.pop();pageHistory=history;goingBack=true;currentPage=target;goingBack=false}
 property int currentPage:0
 property bool allowClose:false
 property string toastText:""
 property bool toastOk:true
 property var states:["Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"]
 property var statesAr:["تم الاستلام","التشخيص","انتظار القطع","قيد الإصلاح","جاهز","تم التسليم","ملغي"]
 property var titles:ar?["مركز العمليات","الأجهزة والاتصال","تشخيص الجهاز","مخططات اللوحات","فحص ملفات النظام","أوامر الصيانة","قطع الغيار","جاهزية وإعدادات","استعادة البيانات","مختبر الأدلة الرقمية"]:["Operations center","Devices & USB","Diagnostics","Board workspace","Firmware preflight","Repair orders","Parts inventory","Readiness & settings","Data recovery","Forensic Lab"]
 function trx(a,e){return ar?a:e}
 function orderStatus(s){let i=states.indexOf(s);return ar&&i>=0?statesAr[i]:s}
 function notice(ok,message,errorMessage){toastOk=ok;toastText=ok?message:(errorMessage===undefined?workshopBridge.error:errorMessage);toastTimer.restart()}
 function navigate(index){currentPage=index}
 function navigationButton(index){return navigation.itemAt(index)}
 palette.window:window.tone("#081825");palette.windowText:window.tone("#e0ecfa");palette.base:window.tone("#0c2131");palette.alternateBase:window.tone("#102a3e");palette.text:window.tone("#dae8f7");palette.button:window.tone("#122d43");palette.buttonText:window.tone("#e0ecff");palette.highlight:window.tone("#176df0");palette.highlightedText:"white";palette.mid:window.tone("#27435b");palette.dark:window.tone("#081320");palette.light:window.tone("#34536d");palette.placeholderText:window.tone("#6f8ca5")
 font.family:"Segoe UI";font.pixelSize:14
 LayoutMirroring.enabled:ar
 LayoutMirroring.childrenInherit:true
 Shortcut {sequence:"Ctrl+1";onActivated:window.navigate(0)}
 Shortcut {sequence:"Ctrl+2";onActivated:window.navigate(1)}
 Shortcut {sequence:"Ctrl+3";onActivated:window.navigate(2)}
 Shortcut {sequence:"Ctrl+4";onActivated:window.navigate(3)}
 Shortcut {sequence:"Ctrl+5";onActivated:window.navigate(4)}
 Shortcut {sequence:"Ctrl+6";onActivated:window.navigate(5)}
 Shortcut {sequence:"Ctrl+7";onActivated:window.navigate(6)}
 Shortcut {sequence:"Ctrl+8";onActivated:window.navigate(7)}
 Shortcut {sequence:"Ctrl+9";onActivated:window.navigate(8)}
 Shortcut {sequence:"Ctrl+R";onActivated:if(!toolBridge.busy)toolBridge.scanAdb()}
 Shortcut {sequence:"Ctrl+Q";onActivated:window.close()}
 onClosing:function(close){if(!allowClose){close.accepted=false;exitDialog.open()}}
 Dialog {
  id:exitDialog;width:480;objectName:"exitDialog";anchors.centerIn:parent;modal:true;title:window.trx("إغلاق NEXVARY MR","Close NEXVARY MR");standardButtons:Dialog.Yes|Dialog.No
  Label {width:430;wrapMode:Text.WordWrap;color:window.tone("#cfdfed");text:window.trx("هل تريد إغلاق البرنامج؟ ستُلغى الفحوص الجارية. احفظ جلسة المخطط والنماذج المفتوحة أولًا؛ أوامر الصيانة المحفوظة محفوظة محليًا.","Close the application? Running checks will be cancelled. Save board sessions and open forms first; saved repair orders are stored locally.")}
  onAccepted:{toolBridge.cancel();firmwareBridge.cancel();recoveryBridge.cancel();window.allowClose=true;window.close()}
 }
 ColumnLayout {
  anchors.fill:parent;spacing:0
  Rectangle {
   Layout.fillWidth:true;Layout.preferredHeight:80;color:window.tone("#071421");border.color:window.tone("#193248")
   RowLayout {
    anchors.fill:parent;anchors.margins:18;spacing:18
    Image {source:"qrc:/assets/nexvary-mr.png";Layout.preferredWidth:48;Layout.preferredHeight:48;fillMode:Image.PreserveAspectFit}
    ColumnLayout {spacing:1;Label{text:"Nexvary MR";font.pixelSize:28;font.bold:true;color:window.tone("#f2f6ff")}Label{text:"MOBILE REPAIR WORKSTATION";font.pixelSize:10;font.letterSpacing:1.7;color:window.tone("#829bb5")}}
    Item {Layout.fillWidth:true}
    ActionButton {objectName:"devicePanelButton";text:window.trx("لوحة الجهاز","Device panel");onClicked:window.showDevicePanel=!window.showDevicePanel}
    Rectangle {Layout.preferredWidth:1;Layout.fillHeight:true;color:window.tone("#1b3449")}
    ActionButton {objectName:"themeButton";text:window.darkTheme?window.trx("فاتح","Light"):window.trx("داكن","Dark");onClicked:window.darkTheme=!window.darkTheme}
    ActionButton {objectName:"languageButton";text:window.ar?"English":"العربية";onClicked:window.ar=!window.ar}
    ActionButton {objectName:"exitButton";text:window.trx("خروج","Exit");onClicked:window.close()}
   }
  }
  RowLayout {
   Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:10;spacing:10
   Rectangle {
    Layout.preferredWidth:202;Layout.fillHeight:true;color:window.tone("#071421");radius:10;border.color:window.tone("#193248")
    ScrollView {
     id:navigationScroll;anchors.fill:parent;anchors.margins:8;contentWidth:availableWidth;clip:true
     ColumnLayout {
      width:navigationScroll.availableWidth;spacing:5
      Repeater {
       id:navigation;model:window.titles
       delegate:Button {
        required property int index
        required property string modelData
        objectName:"nav"+index;Layout.fillWidth:true;Layout.preferredHeight:52;hoverEnabled:true
        Accessible.name:modelData
        onClicked:window.navigate(index)
        background:Rectangle {radius:8;color:window.currentPage===index?window.tone("#0d4dba"):parent.hovered?window.tone("#122c41"):window.tone("#0a1c2c");border.color:window.currentPage===index?window.tone("#2d7ef0"):window.tone("#142f46")}
        contentItem:RowLayout {spacing:10;NavIcon {kind:index;Layout.preferredWidth:32;Layout.preferredHeight:32}Label {text:modelData;Layout.fillWidth:true;Layout.minimumWidth:0;color:window.tone("#d6e7fc");font.pixelSize:13;wrapMode:Text.WordWrap;maximumLineCount:2;elide:Text.ElideRight}}
       }
      }
     }
    }
   }
   Panel {
    title:window.trx("الجهاز المحدد","Selected device");visible:window.showDevicePanel;Layout.preferredWidth:224;Layout.fillHeight:true
    ScrollView {
     id:deviceScroll;objectName:"deviceScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
     ColumnLayout {
      width:deviceScroll.availableWidth;spacing:12
    Rectangle {
     Layout.fillWidth:true;Layout.preferredHeight:155;color:window.tone("#07131f");radius:7;border.color:window.tone("#1b3850")
     Rectangle {anchors.centerIn:parent;width:66;height:116;radius:12;color:window.tone("#0b2947");border.width:2;border.color:toolBridge.selectedSerial?window.tone("#3989ff"):window.tone("#31506a");Rectangle{anchors.top:parent.top;anchors.horizontalCenter:parent.horizontalCenter;anchors.topMargin:7;width:25;height:4;radius:2;color:window.tone("#5b8bb2")}Label{anchors.centerIn:parent;text:toolBridge.selectedSerial?"USB":"—";color:window.tone("#6fa8d9");font.pixelSize:18}}
    }
    Label {text:toolBridge.deviceInfo["ro.product.model"]||window.trx("لا يوجد هاتف محدد","No phone selected");color:window.tone("#e0edfc");font.bold:true;wrapMode:Text.WordWrap;Layout.fillWidth:true;textFormat:Text.PlainText}
    Label {text:toolBridge.selectedSerial||window.trx("ابدأ من الأجهزة والاتصال","Start with Devices & USB");color:window.tone("#8ba5bb");wrapMode:Text.WrapAnywhere;Layout.fillWidth:true;textFormat:Text.PlainText;font.pixelSize:12}
    Rectangle {Layout.fillWidth:true;height:1;color:window.tone("#1c374b")}
    Label {text:window.trx("عمليات سريعة","QUICK ACTIONS");color:window.tone("#68a8dd");font.pixelSize:12}
    ActionButton {text:window.trx("فحص الاتصال","Scan connection");Layout.fillWidth:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
    ActionButton {text:window.trx("قراءة المعلومات","Read properties");Layout.fillWidth:true;enabled:!toolBridge.busy&&toolBridge.selectedSerial!=="";onClicked:toolBridge.readDeviceInfo()}
    ActionButton {text:window.trx("عرض الشاشة","Mirror screen");Layout.fillWidth:true;enabled:!toolBridge.busy&&toolBridge.selectedSerial!=="";onClicked:toolBridge.startMirror()}
    ActionButton {text:window.trx("أمر صيانة جديد","Repair orders");Layout.fillWidth:true;onClicked:window.navigate(5)}
    ActionButton {objectName:"quickBackup";text:window.trx("النسخ الاحتياطي","Backup & restore");Layout.fillWidth:true;onClicked:window.navigate(7)}
    Label {text:window.trx("فحوص أولية للقراءة فقط\nلا توجد عمليات فتح حماية أو تعديل هوية","Read-only initial checks\nNo protection bypass or identity changes");color:window.tone("#6e8ca5");wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:11}
     }
    }
   }
   ColumnLayout {
    Layout.fillWidth:true;Layout.fillHeight:true;spacing:10
    RowLayout {Layout.fillWidth:true;ActionButton {objectName:"backButton";text:window.trx("رجوع","Back");enabled:window.pageHistory.length>0;onClicked:window.goBack()}Label{text:window.titles[window.currentPage];color:window.tone("#e6f0ff");font.pixelSize:23;font.bold:true;Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideRight}Item{Layout.fillWidth:true}Label{text:"0.4.0 / "+window.trx("تطوير","DEVELOPMENT");color:window.tone("#6d94b3");font.pixelSize:11}}
    StackLayout {
     id:pages;Layout.fillWidth:true;Layout.fillHeight:true;currentIndex:window.currentPage
     ColumnLayout {
      spacing:12
      RowLayout {
       Layout.fillWidth:true;spacing:12
       Repeater {
        model:[{label:window.trx("الأجهزة المكتشفة","Discovered devices"),value:toolBridge.devices.length,color:window.tone("#5faaff")},{label:window.trx("أوامر الصيانة","Repair orders"),value:workshopBridge.orders.length,color:window.tone("#4ed9b0")},{label:window.trx("أصناف المخزون","Inventory items"),value:workshopBridge.inventory.length,color:window.tone("#b29bff")}]
        delegate:Rectangle {required property var modelData;Layout.fillWidth:true;height:108;color:window.tone("#0b2133");radius:7;border.color:window.tone("#203d54");ColumnLayout{anchors.fill:parent;anchors.margins:16;Label{text:modelData.value;color:modelData.color;font.pixelSize:32;font.bold:true}Label{text:modelData.label;color:window.tone("#92aec7");font.pixelSize:13}}}
       }
      }
      Panel {
       title:window.trx("من الاستلام إلى التسليم","FROM INTAKE TO HANDOVER");Layout.fillWidth:true;Layout.preferredHeight:175
       Label {text:window.trx("ابدأ بفحص جاهزية الكمبيوتر، ثم اتصل بالجهاز، واجمع الأدلة قبل اتخاذ قرار الصيانة.","Check workstation readiness, connect the device, then collect evidence before choosing a repair.");color:window.tone("#a6bfd6");wrapMode:Text.WordWrap;Layout.fillWidth:true}
       RowLayout {Layout.fillWidth:true;ActionButton{text:window.trx("1  جاهزية الأدوات","1  Tool readiness");Layout.fillWidth:true;onClicked:window.navigate(7)}ActionButton{text:window.trx("2  اكتشاف الهاتف","2  Connect device");Layout.fillWidth:true;primary:true;onClicked:window.navigate(1)}ActionButton{text:window.trx("3  التشخيص","3  Diagnose");Layout.fillWidth:true;onClicked:window.navigate(2)}ActionButton{text:window.trx("4  توثيق الصيانة","4  Repair order");Layout.fillWidth:true;onClicked:window.navigate(5)}}
      }
      RowLayout {
       Layout.fillWidth:true;Layout.fillHeight:true;spacing:12
       Panel {
        title:window.trx("حالة بيئة العمل","WORKSTATION HEALTH");Layout.fillWidth:true;Layout.fillHeight:true
        Repeater {model:toolBridge.tools;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;Rectangle{width:7;height:7;radius:4;color:modelData.available?window.tone("#47dba6"):window.tone("#edb56b")}Label{text:modelData.name;color:window.tone("#d2e5f7");Layout.fillWidth:true}Label{text:modelData.available?window.trx("متاح","Ready"):window.trx("غير مثبت","Missing");color:modelData.available?window.tone("#47dba6"):window.tone("#edb56b")}}}
        Label {text:window.trx("SQLite: ","SQLite: ")+(workshopBridge.ready?window.trx("جاهزة","Ready"):workshopBridge.error);color:workshopBridge.ready?window.tone("#47dba6"):window.tone("#edb56b");Layout.fillWidth:true;wrapMode:Text.WordWrap}
        Item{Layout.fillHeight:true}
       }
       Panel {
        title:window.trx("سجل الورشة الفعلي","WORKSHOP ACTIVITY");Layout.fillWidth:true;Layout.fillHeight:true
        ListView {Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:workshopBridge.events;delegate:Label{required property var modelData;width:ListView.view.width;text:modelData.created_at+"  •  "+modelData.action;color:window.tone("#9fbdd3");font.pixelSize:12;wrapMode:Text.WordWrap;textFormat:Text.PlainText}}
        Label {visible:workshopBridge.events.length===0;text:window.trx("لا توجد عمليات مسجلة بعد","No recorded operations yet");color:window.tone("#708da6")}
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
     RecoveryPage {app:window}
     ForensicLabPage {app:window}
    }
    Rectangle {
     Layout.fillWidth:true;Layout.preferredHeight:66;color:window.tone("#061622");radius:6;border.color:window.tone("#1a354a")
     RowLayout {anchors.fill:parent;anchors.margins:12;Rectangle{width:8;height:8;radius:4;color:toolBridge.busy?window.tone("#ffbe60"):window.tone("#429dca")}Label{text:toolBridge.status;Layout.fillWidth:true;maximumLineCount:2;elide:Text.ElideRight;wrapMode:Text.WordWrap;color:window.tone("#89abc5");font.pixelSize:12;textFormat:Text.PlainText}BusyIndicator{Layout.preferredWidth:28;Layout.preferredHeight:28;running:toolBridge.busy}}
    }
   }
  }
  Rectangle {Layout.fillWidth:true;Layout.preferredHeight:29;color:window.tone("#06121f");border.color:window.tone("#193245");RowLayout{anchors.fill:parent;anchors.leftMargin:18;anchors.rightMargin:18;Label{text:"NEXVARY MR";color:window.tone("#6f9cbe");font.pixelSize:11}Item{Layout.fillWidth:true}Label{text:window.trx("محلي · دون خدمة سحابية · اتصال مصرح به فقط","LOCAL · OFFLINE · AUTHORIZED DEVICES ONLY");color:window.tone("#527b9c");font.pixelSize:10}}}
 }
 Rectangle {
  anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:45;width:Math.min(680,parent.width-40);height:70;radius:8;visible:toastText!=="";color:toastOk?window.tone("#123a35"):window.tone("#48252a");border.color:toastOk?window.tone("#36bc96"):window.tone("#e0787b");z:100
  Label{anchors.fill:parent;anchors.margins:12;text:window.toastText;color:window.tone("#eff9ff");wrapMode:Text.WordWrap;textFormat:Text.PlainText;verticalAlignment:Text.AlignVCenter}
 }
 Timer{id:toastTimer;interval:6000;onTriggered:window.toastText=""}
}
