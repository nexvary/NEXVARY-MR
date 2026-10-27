import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 objectName:"deviceManagerPage"
 property var app
 property string reviewedMirrorTarget:""
 property bool showMirror:false
 property bool showDrivers:false
 readonly property bool selected:toolBridge.selectedSerial!==""
 readonly property var selectedConnections:toolBridge.devices.filter(function(device){return device.serial===toolBridge.selectedSerial})
 readonly property bool adbReady:page.selected&&selectedConnections.length===1&&selectedConnections[0].transport==="adb"&&selectedConnections[0].state==="device"
 readonly property string selectionReason:!page.selected?app.trx("حدد جهازًا أولًا","Select a device first"):selectedConnections.length!==1?app.trx("اتصال الجهاز غائب أو ملتبس؛ أعد الاكتشاف","Target missing or ambiguous; rediscover devices"):selectedConnections[0].state==="unauthorized"?app.trx("وافق على تصحيح USB من الهاتف","Authorize USB debugging on the phone"):selectedConnections[0].state==="offline"?app.trx("اتصال ADB غير متاح؛ افحص الكابل والتفويض","ADB is offline; check the cable and authorization"):app.trx("تتطلب هذه العملية اتصال ADB بحالة device","This operation requires an ADB connection in device state")
 readonly property bool identified:!!toolBridge.deviceInfo["ro.build.fingerprint"]
 function screenState(){const state=toolBridge.mirrorState;return state==="running"||state.indexOf("monitor-")===0?app.trx("نافذة العرض قيد التشغيل","Screen window running"):state==="stopping"?app.trx("جارٍ إيقاف العرض","Stopping screen view"):app.trx("جارٍ التحقق من الأداة والهاتف","Checking tool and device")}
 function reviewMirror(){
  reviewedMirrorTarget=toolBridge.selectedSerial
  mirrorOwner.checked=false;mirrorComponent.checked=false;mirrorChallenge.clear();mirrorConfirm.open()
 }
 Item {id:logAnalysis;property string text:""}
 Connections {
  target:toolBridge
  function onDeviceInfoChanged(){resultTabs.currentIndex=0}
  function onLogcatChanged(){logAnalysis.text="";page.showMirror=false;page.showDrivers=false;resultTabs.currentIndex=1}
  function onDriverInfoChanged(){if(toolBridge.activeOperation==="drivers"&&toolBridge.driverInfo!==""){page.showMirror=false;page.showDrivers=true;resultTabs.currentIndex=2}}
 }
 ScrollView {
  id:workspace;objectName:"deviceManagerScroll";anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:workspace.availableWidth;spacing:16
   Panel {
    title:app.trx("1. الاتصال واختيار الجهاز","1. CONNECT AND SELECT");accent:"blue";Layout.fillWidth:true
    RowLayout {
     Layout.fillWidth:true;spacing:12
     Image {source:"qrc:/assets/icons/phone.svg";Layout.preferredWidth:40;Layout.preferredHeight:40;fillMode:Image.PreserveAspectFit}
     ColumnLayout {
      Layout.fillWidth:true;spacing:4
      Label {text:page.selected?app.trx("جهاز محدد وجاهز للفحوص المتاحة","A target is selected for available checks"):app.trx("اختر هاتفًا قبل تشغيل أي عملية","Select a phone before starting an operation");font.bold:true;color:app.tone("#edf5ff");Layout.fillWidth:true;wrapMode:Text.WordWrap}
      Label {text:app.trx("اكشف الأجهزة، اختر الهدف، ثم اقرأ هويته. لا يتغير الجهاز المحدد تلقائيًا.","Discover devices, choose the target, then read its identity. The selected target never changes automatically.");color:app.tone("#9bbbd3");Layout.fillWidth:true;wrapMode:Text.WordWrap}
     }
     Label {text:toolBridge.devices.length+app.trx(" جهاز مكتشف"," discovered");color:app.tone("#64dbb0");font.bold:true}
    }
    Flow {
     Layout.fillWidth:true;spacing:8
     ActionButton {objectName:"scanAdb";text:app.trx("اكتشاف ADB","Discover ADB");primary:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
     ActionButton {objectName:"scanFastboot";text:app.trx("اكتشاف Fastboot","Discover Fastboot");enabled:!toolBridge.busy;onClicked:toolBridge.scanFastboot()}
     ActionButton {objectName:"inspectDeviceDrivers";text:app.trx("فحص USB والتعريفات","Inspect USB / drivers");enabled:!toolBridge.busy;onClicked:{page.showMirror=false;page.showDrivers=true;resultTabs.currentIndex=2;toolBridge.inspectDrivers()}}
     Switch {objectName:"deviceAutoRefresh";text:app.trx("تحديث تلقائي","Auto refresh");checked:toolBridge.autoRefresh;onToggled:toolBridge.autoRefresh=checked}
     ActionButton {objectName:"cancelDeviceOperation";text:app.trx("إلغاء العملية","Cancel operation");enabled:toolBridge.busy;onClicked:toolBridge.cancel()}
    }
    Label {visible:toolBridge.devices.length===0;text:app.trx("لا يوجد جهاز مكتشف. صِل كابل بيانات ووافق على تصحيح USB في هاتفك، ثم اضغط اكتشاف ADB.","No devices discovered. Connect a data cable, authorize USB debugging on your phone, then click Discover ADB.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#9bbbd3")}
    ListView {
     objectName:"deviceSelectionList";visible:toolBridge.devices.length>0
     Layout.fillWidth:true;Layout.preferredHeight:Math.min(240,Math.max(76,toolBridge.devices.length*80));model:toolBridge.devices;spacing:8;clip:true
     ScrollBar.vertical:ScrollBar {}
     delegate:ItemDelegate {
      required property var modelData
      width:ListView.view.width;height:72
      Accessible.name:(modelData.model||modelData.serial)+" "+modelData.state
      onClicked:toolBridge.selectDevice(modelData.serial)
      background:Rectangle {radius:8;color:toolBridge.selectedSerial===modelData.serial?app.tone("#143c64"):app.tone("#0b2131");border.width:toolBridge.selectedSerial===modelData.serial?2:1;border.color:toolBridge.selectedSerial===modelData.serial?"#299bff":app.tone("#234a63")}
      contentItem:RowLayout {
       spacing:12
       Image {source:"qrc:/assets/icons/phone.svg";Layout.preferredWidth:34;Layout.preferredHeight:34;fillMode:Image.PreserveAspectFit}
       ColumnLayout {
        Layout.fillWidth:true;spacing:4
        Label {text:modelData.model||modelData.serial;color:app.tone("#edf4ff");font.bold:true;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
        Label {text:modelData.serial;color:app.tone("#9bbbd3");font.pixelSize:12;Layout.fillWidth:true;elide:Text.ElideMiddle;textFormat:Text.PlainText}
       }
       Label {text:modelData.state;color:modelData.state==="device"?app.tone("#48dea4"):app.tone("#f6bb67");font.bold:true;textFormat:Text.PlainText}
       Label {visible:toolBridge.selectedSerial===modelData.serial;text:app.trx("محدد","Selected");color:"#60bdff"}
      }
     }
    }
   }
   GridLayout {
    Layout.fillWidth:true;columns:page.width>=960?3:2;columnSpacing:12;rowSpacing:12
    ActionCard {
     app:page.app;Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     iconName:"phone";title:app.trx("معلومات الهاتف","Phone information");description:app.trx("قراءة الموديل وإصدار النظام والبناء وحالات الحماية المعلنة من الهاتف.","Read the phone model, OS, build and reported security states.")
     statusText:page.identified?app.trx("تمت قراءة الهوية","Identity read"):app.trx("قراءة فقط","Read only")
     actionText:app.trx("قراءة المعلومات","Read properties");actionName:"readDeviceInfo";available:page.adbReady&&!toolBridge.busy;working:toolBridge.activeOperation==="properties";primary:true
     unavailableReason:!page.adbReady?page.selectionReason:app.trx("انتظر انتهاء العملية الجارية","Wait for the running operation")
     onTriggered:{resultTabs.currentIndex=0;toolBridge.readDeviceInfo()}
    }
    ActionCard {
     app:page.app;Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     iconName:"history";title:app.trx("سجلات النظام","System logs");description:app.trx("قراءة آخر 200 سجل محليًا، ثم تحليل المؤشرات مع الاحتفاظ بالدليل النصي.","Read the last 200 log entries locally, then inspect indicators with their text evidence.")
     statusText:toolBridge.logcat!==""?app.trx("سجل متاح للمراجعة","Log available"):app.trx("قراءة فقط","Read only")
     actionText:app.trx("قراءة آخر 200 سجل","Read last 200 logs");actionName:"readDeviceLogcat";available:page.adbReady&&!toolBridge.busy;working:toolBridge.activeOperation==="logcat"
     unavailableReason:!page.adbReady?page.selectionReason:app.trx("انتظر انتهاء العملية الجارية","Wait for the running operation")
     onTriggered:{page.showMirror=false;page.showDrivers=false;resultTabs.currentIndex=1;toolBridge.readLogcat()}
    }
    ActionCard {
     app:page.app;Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     iconName:"gauge";title:app.trx("عرض شاشة الهاتف","Phone screen view");description:app.trx("عرض الشاشة في نافذة مستقلة بإذن المالك، دون تحكم أو صوت أو مزامنة الحافظة.","Owner-consented screen viewing in a separate window. No remote control, audio or clipboard sync.")
     statusText:toolBridge.mirrorActive?app.trx("جلسة عرض نشطة","Screen session active"):app.trx("بموافقة صريحة","Explicit consent")
     actionText:app.trx("مراجعة عرض الشاشة","Review screen view");actionName:"reviewMirror";available:toolBridge.screenViewAvailable&&!toolBridge.busy&&page.identified&&page.adbReady
     unavailableReason:!toolBridge.screenViewAvailable?app.trx("عرض الشاشة غير متاح","Screen view unavailable"):!page.adbReady?page.selectionReason:!page.identified?app.trx("حدد الهاتف واقرأ معلوماته أولًا","Select the phone and read properties first"):app.trx("انتظر انتهاء العملية الجارية","Wait for the running operation")
     onTriggered:page.reviewMirror()
    }
   }
   DeviceIntelligencePanel {objectName:"deviceIdentitySummary";app:page.app;Layout.fillWidth:true}
   Label {objectName:"mirrorScope";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("عرض بإذن صريح عبر scrcpy 5.0.1 في نافذة مستقلة. قد يلزم فتح شاشة الهاتف لعرض المحتوى؛ لا يتجاوز التطبيق قفل الهاتف. عند غياب الأداة، اختر ملفها الرسمي من الإعدادات.","Explicitly consented scrcpy 5.0.1 view in a separate window. The phone may need to be unlocked to display content; this does not bypass its lock. If the tool is absent, choose its official executable in Settings.")}
   ActionButton {objectName:"mirrorToolSettings";text:app.trx("إعداد أداة عرض الشاشة","Screen-view tool settings");onClicked:app.navigate(7)}
   Panel {
    title:app.trx("3. النتائج والتفاصيل","3. RESULTS AND DETAILS");Layout.fillWidth:true
    TabBar {
     id:resultTabs;objectName:"deviceResultTabs";Layout.fillWidth:true
     TabButton {text:app.trx("الخصائص الخام","Raw properties")}
     TabButton {text:app.trx("Logcat والمؤشرات","Logcat / indicators")}
     TabButton {text:app.trx("USB والتعريفات","USB / drivers")}
     TabButton {text:app.trx("جلسة الشاشة","Screen session")}
    }
    Flow {
     Layout.fillWidth:true;spacing:8
     ActionButton {objectName:"analyzeDeviceLog";visible:resultTabs.currentIndex===1;text:app.trx("تحليل مؤشرات السجل","Analyze log indicators");enabled:toolBridge.logcat!=="";onClicked:logAnalysis.text=repairBridge.analyzeLogReport(toolBridge.logcat,app.ar)}
     ActionButton {objectName:"stopMirror";visible:resultTabs.currentIndex===3||toolBridge.mirrorActive;text:app.trx("إيقاف عرض الشاشة","Stop screen view");enabled:toolBridge.mirrorActive;onClicked:toolBridge.stopMirror()}
     Label {objectName:"mirrorState";visible:resultTabs.currentIndex===3&&toolBridge.mirrorActive;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#8eb6d6");text:page.screenState()}
    }
    ScrollView {
     Layout.fillWidth:true;Layout.preferredHeight:page.height>850?300:220;contentWidth:availableWidth;clip:true
     TextArea {
      objectName:"deviceResultText";readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.Wrap;horizontalAlignment:Text.AlignLeft;LayoutMirroring.enabled:false;font.family:"Consolas";font.pixelSize:12;color:app.tone("#c7deef");background:null
      text:resultTabs.currentIndex===0?(page.identified?JSON.stringify(toolBridge.deviceInfo,null,2):app.trx("اقرأ معلومات الهاتف لعرض الخصائص هنا.","Read phone properties to display them here.")):resultTabs.currentIndex===1?(logAnalysis.text!==""?logAnalysis.text+"\n\n──── LOGCAT ────\n":"")+(toolBridge.logcat||app.trx("لم تتم قراءة السجل بعد.","No log has been read yet.")):resultTabs.currentIndex===2?(toolBridge.driverInfo||app.trx("شغّل فحص USB والتعريفات لعرض النتائج هنا.","Run USB / driver inspection to display the results here.")):(toolBridge.mirrorStatus!==""||toolBridge.mirrorOutput!==""?toolBridge.mirrorStatus+"\n\n"+toolBridge.mirrorOutput:app.trx("لم تبدأ جلسة عرض بعد.","No screen session has started."))
     }
    }
    Label {text:app.trx("الخصائص والسجلات تصريحات مقروءة من الجهاز وليست اعتمادًا شاملًا لسلامته. راجع البيانات الشخصية قبل مشاركة النتائج.","Properties and logs are device observations, not complete health certification. Review personal data before sharing results.");color:app.tone("#9bbbd3");font.pixelSize:11;Layout.fillWidth:true;wrapMode:Text.WordWrap}
   }
  }
 }
 Dialog {id:mirrorConfirm;objectName:"mirrorConfirm";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;width:Math.min(620,app.width-48);title:app.trx("موافقة عرض شاشة الهاتف","Phone screen-view consent");standardButtons:Dialog.Ok|Dialog.Cancel
  onOpened:standardButton(Dialog.Ok).enabled=Qt.binding(function(){return toolBridge.screenViewAvailable&&!toolBridge.busy&&page.reviewedMirrorTarget!==""&&page.reviewedMirrorTarget===toolBridge.selectedSerial&&mirrorOwner.checked&&mirrorComponent.checked&&mirrorChallenge.text===page.reviewedMirrorTarget})
  onAccepted:{page.showMirror=true;resultTabs.currentIndex=3;app.notice(toolBridge.startMirror(page.reviewedMirrorTarget,{authorized:mirrorOwner.checked,component:mirrorComponent.checked,challenge:mirrorChallenge.text}),app.trx("بدأ التحقق من أداة العرض والهدف","Screen-view tool and target checks started"),toolBridge.status)}
  ColumnLayout {width:mirrorConfirm.availableWidth;spacing:12
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:app.trx("الجهاز المحدد: ","Reviewed target: ")+page.reviewedMirrorTarget;color:app.tone("#edf4ff")}
   Label {objectName:"mirrorComponentNotice";Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("scrcpy يرفع مكونًا مؤقتًا ويشغله على الهاتف؛ هذه ليست عملية قراءة خالصة. قد تظهر بيانات الشاشة الخاصة في نافذة خارجية. إيقاف البرنامج لا يثبت تنظيف الهاتف أو سلامة الأدلة. اختبار الهاتف الفعلي معلق.","scrcpy uploads and runs a temporary device component; this is not a purely passive read. Private screen data may appear in an external window. Stopping the host does not prove phone cleanup or forensic integrity. Physical acceptance is pending.");color:app.tone("#eab775")}
   NeonCheckBox {id:mirrorOwner;objectName:"mirrorOwner";text:app.trx("لدي تفويض صريح من مالك الجهاز لعرض الشاشة","I have explicit owner authorization to view this screen")}
   NeonCheckBox {id:mirrorComponent;objectName:"mirrorComponent";text:app.trx("أوافق على تشغيل المكون المؤقت وكشف محتوى الشاشة","I approve the temporary component and screen-content exposure")}
   TextField {id:mirrorChallenge;objectName:"mirrorChallenge";Layout.fillWidth:true;placeholderText:app.trx("اكتب الرقم التسلسلي المحدد للتأكيد","Type the reviewed serial to confirm")}
  }
 }

}
