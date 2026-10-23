import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 property string reviewedMirrorTarget:""
 property bool showMirror:false
 property var app
 property bool showDrivers:false
 ColumnLayout {
  anchors.fill:parent; spacing:12
  RowLayout {
   Layout.fillWidth:true
   ActionButton {objectName:"scanAdb";text:app.trx("فحص ADB","Scan ADB");primary:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
   ActionButton {text:"Fastboot";enabled:!toolBridge.busy;onClicked:toolBridge.scanFastboot()}
   ActionButton {text:app.trx("فحص التعريفات","Inspect drivers");enabled:!toolBridge.busy;onClicked:{page.showMirror=false;showDrivers=true;toolBridge.inspectDrivers()}}
   Switch {text:app.trx("تحديث تلقائي","Auto refresh");checked:toolBridge.autoRefresh;onToggled:toolBridge.autoRefresh=checked}
   Item {Layout.fillWidth:true}
   BusyIndicator {running:toolBridge.busy;Layout.preferredWidth:32;Layout.preferredHeight:32}
   ActionButton {text:app.trx("إلغاء","Cancel");enabled:toolBridge.busy;onClicked:toolBridge.cancel()}
  }
  Item {id:logAnalysis;property string text:""}
  Connections {target:toolBridge;function onLogcatChanged(){logAnalysis.text=""}}
  RowLayout {
   Layout.fillWidth:true; Layout.fillHeight:true;spacing:12
   Panel {
    accent:"blue";title:app.trx("الأجهزة المكتشفة","Discovered devices");Layout.preferredWidth:290;Layout.fillHeight:true
    Label {visible:toolBridge.devices.length===0;text:app.trx("لم يتم اكتشاف جهاز. صِل هاتفًا بكابل بيانات، ثم اسمح بتصحيح USB من الهاتف.","No device discovered. Connect a data cable, then authorize USB debugging on the phone.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#91a6bc")}
    ListView {
     Layout.fillWidth:true;Layout.fillHeight:true;model:toolBridge.devices;spacing:8;clip:true
     delegate:ItemDelegate {
      required property var modelData
      width:ListView.view.width;height:76
      onClicked:toolBridge.selectDevice(modelData.serial)
      background:Rectangle {radius:6;color:toolBridge.selectedSerial===modelData.serial?app.tone("#143c64"):app.tone("#0b2131");border.color:app.tone("#234a63")}
      contentItem:Column {spacing:6;Label {text:modelData.model||modelData.serial;color:app.tone("#edf4ff");font.bold:true;textFormat:Text.PlainText} Label{text:modelData.serial+" • "+modelData.state;color:modelData.state==="device"?app.tone("#48dea4"):app.tone("#f6bb67");font.pixelSize:12;textFormat:Text.PlainText}}
     }
    }
    Label {objectName:"mirrorScope";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:toolBridge.laboratoryBuild?app.trx("عرض تجريبي عبر scrcpy 5.0.1: اقرأ معلومات الهاتف ثم راجع الموافقة. دون تحكم أو صوت أو مزامنة الحافظة.","Experimental scrcpy 5.0.1 view: read phone information, then review consent. No control, audio or clipboard sync."):app.trx("عرض الشاشة يحتاج نسخة Laboratory؛ الإنتاج للقراءة والتشخيص فقط.","Screen view requires Laboratory; production is read-only diagnostics.")}
    ActionButton {objectName:"reviewMirror";text:app.trx("مراجعة عرض الشاشة","Review screen view");Layout.fillWidth:true;enabled:toolBridge.laboratoryBuild&&!toolBridge.busy&&!!toolBridge.deviceInfo["ro.build.fingerprint"]&&toolBridge.selectedSerial!=="";onClicked:{page.reviewedMirrorTarget=toolBridge.selectedSerial;mirrorOwner.checked=false;mirrorComponent.checked=false;mirrorChallenge.clear();mirrorConfirm.open()}}
    ActionButton {objectName:"stopMirror";text:app.trx("إيقاف عرض الشاشة","Stop screen view");Layout.fillWidth:true;enabled:toolBridge.mirrorActive;onClicked:toolBridge.stopMirror()}
    ActionButton {text:app.trx("سجل عرض الشاشة","Screen-view log");Layout.fillWidth:true;enabled:toolBridge.mirrorStatus!=="";onClicked:{page.showMirror=true;showDrivers=false}}
    Label {objectName:"mirrorState";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#8eb6d6");text:toolBridge.mirrorActive?app.trx("حالة جلسة العرض: ","Screen-view state: ")+toolBridge.mirrorState:""}

   }
   ColumnLayout {
    Layout.fillWidth:true;Layout.fillHeight:true;spacing:12
    Panel {
     title:app.trx("خصائص الجهاز · قراءة فقط","Device properties · read only");Layout.fillWidth:true;Layout.fillHeight:true
     RowLayout {ActionButton {text:app.trx("قراءة المعلومات","Read properties");enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:toolBridge.readDeviceInfo()} ActionButton{text:app.trx("قراءة آخر 200 سجل","Read last 200 logs");enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:{page.showMirror=false;showDrivers=false;toolBridge.readLogcat()}}}
     ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;text:JSON.stringify(toolBridge.deviceInfo,null,2);color:app.tone("#c7deef");font.family:"Consolas";background:null;wrapMode:TextEdit.Wrap}}
    }
    Panel {
     title:app.trx("السجلات والتعريفات","Logs and drivers");Layout.fillWidth:true;Layout.fillHeight:true
     ActionButton {text:app.trx("تحليل مؤشرات السجل","Analyze log indicators");enabled:toolBridge.logcat!=="";onClicked:logAnalysis.text=repairBridge.analyzeLogReport(toolBridge.logcat,app.ar)}
     ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;text:page.showMirror?toolBridge.mirrorStatus+"\n\n"+toolBridge.mirrorOutput:showDrivers?toolBridge.driverInfo:(logAnalysis.text!==""?logAnalysis.text+"\n\n──── LOGCAT ────\n":"")+(toolBridge.logcat||toolBridge.driverInfo||app.trx("نتائج الفحص ستظهر هنا. قد تحتوي السجلات على بيانات شخصية؛ شاركها بعد المراجعة.","Read-only results appear here. Logs may contain personal data; review before sharing."));color:app.tone("#a8bfd4");font.family:"Consolas";wrapMode:TextEdit.Wrap;background:null}}
    }
   }
  }
 }
 Dialog {id:mirrorConfirm;objectName:"mirrorConfirm";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;width:Math.min(620,app.width-48);title:app.trx("موافقة عرض الشاشة التجريبي","Experimental screen-view consent");standardButtons:Dialog.Ok|Dialog.Cancel
  onOpened:standardButton(Dialog.Ok).enabled=Qt.binding(function(){return toolBridge.laboratoryBuild&&!toolBridge.busy&&page.reviewedMirrorTarget!==""&&page.reviewedMirrorTarget===toolBridge.selectedSerial&&mirrorOwner.checked&&mirrorComponent.checked&&mirrorChallenge.text===page.reviewedMirrorTarget})
  onAccepted:{page.showMirror=true;app.notice(toolBridge.startMirror(page.reviewedMirrorTarget,{authorized:mirrorOwner.checked,component:mirrorComponent.checked,challenge:mirrorChallenge.text}),app.trx("بدأ التحقق من أداة العرض والهدف","Screen-view tool and target checks started"),toolBridge.status)}
  ColumnLayout {width:mirrorConfirm.availableWidth;spacing:12
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:app.trx("الجهاز المحدد: ","Reviewed target: ")+page.reviewedMirrorTarget;color:app.tone("#edf4ff")}
   Label {objectName:"mirrorComponentNotice";Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("scrcpy يرفع مكونًا مؤقتًا ويشغله على الهاتف؛ هذه ليست عملية قراءة خالصة. قد تظهر بيانات الشاشة الخاصة في نافذة خارجية. إيقاف البرنامج لا يثبت تنظيف الهاتف أو سلامة الأدلة. اختبار الهاتف الفعلي معلق.","scrcpy uploads and runs a temporary device component; this is not a purely passive read. Private screen data may appear in an external window. Stopping the host does not prove phone cleanup or forensic integrity. Physical acceptance is pending.");color:app.tone("#eab775")}
   NeonCheckBox {id:mirrorOwner;objectName:"mirrorOwner";text:app.trx("لدي تفويض صريح من مالك الجهاز لعرض الشاشة","I have explicit owner authorization to view this screen")}
   NeonCheckBox {id:mirrorComponent;objectName:"mirrorComponent";text:app.trx("أوافق على تشغيل المكون المؤقت وكشف محتوى الشاشة","I approve the temporary component and screen-content exposure")}
   TextField {id:mirrorChallenge;objectName:"mirrorChallenge";Layout.fillWidth:true;placeholderText:app.trx("اكتب الرقم التسلسلي المحدد للتأكيد","Type the reviewed serial to confirm")}
  }
 }

}

