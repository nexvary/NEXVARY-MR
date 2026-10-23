import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"deviceServicesPage"
 property var app
 property var selectedAction:({})
 property string reviewedTarget:""
 property var reviewedRecord:({})
 readonly property var diagnostics:toolBridge.serviceProfile.diagnostics||({})
 readonly property var battery:diagnostics.battery||({})
 readonly property var storage:diagnostics.storage||({})
 readonly property var partitions:diagnostics.partitions||({})
 function reviewRecord(record){reviewedRecord=record;historyDetail.open()}
 readonly property var brands:serviceCatalog.deviceCatalog.brands||[]
 readonly property var driverSource:serviceCatalog.drivers.find(function(r){return r.brand.toLowerCase()===brand.currentText.split(" / ")[0].toLowerCase()})
 readonly property var phases:({preparing:["تحضير نسخة الملف والتحقق من البصمة","Preparing and hashing the private copy"],connection:["التحقق من الاتصال الحالي","Checking the current connection"],identity:["مطابقة هوية الهاتف وإصداره","Verifying device identity and build"],capability:["قراءة القدرات الفعلية","Reading actual capabilities"],applying:["تنفيذ الأمر على الجهاز","Applying the selected command"],verifying:["التحقق من النتيجة","Verifying the result"],complete:["اكتملت العملية","Operation completed"],failed:["توقفت العملية؛ راجع النتيجة","Operation stopped; review the result"]})
 function explain(row){
  const unavailable={download:"يحتاج Download Mode إلى إجراء موثق ومختبر للموديل نفسه.",edl:"وضع EDL وأدوات DA وFirehose تعتمد على المعالج والتوقيع والتفويض الحقيقي.",frp:"استعادة الحساب تتم من صاحب الحساب عبر Google أو الشركة المصنعة؛ لا توجد أداة لتجاوز الحماية.",lockscreen:"استخدم الاسترداد الرسمي بصلاحيات المالك؛ لا نحذف بيانات القفل أو نتجاوز الأمان.",modem:"إصلاح المودم يحتاج Firmware مطابقًا وإجراء OEM مختبرًا؛ إعدادات APN متاحة منفصلة.",efs:"لا يوجد أمر عام آمن لمسح EFS أو تعديل SPC أو هوية الشبكة.",sim_unlock:"فتح الشبكة يتبع الإجراء الرسمي لشركة الاتصالات.",factory_reset:"نفّذ الاستعادة الرسمية من إعدادات الهاتف أو Recovery؛ مسح Userdata منفصل ولا يزيل FRP.",f2fs:"تحويل نظام الملفات يحتاج مخطط أقسام وإجراء موثقًا للموديل.",bootloader_unlock:"سنة التصنيع لا تثبت قابلية الفتح؛ استخدم إجراء الشركة الرسمي.",bootloader_relock:"إعادة القفل تتطلب Firmware رسميًا موقعًا ومطابقًا؛ الإجراء العام قد يمنع الإقلاع.",carrier_apps:"إزالة تطبيقات شركات الاتصالات تحتاج مراجعة الحزم واعتمادياتها لكل إصدار."}
  if(!app.ar)return row.reason||""
  if(row.implemented&&!toolBridge.laboratoryBuild&&row.id!=="probe"&&row.id!=="unlock_ability")return "غير متاح في نسخة الإنتاج المخصصة للقراءة والتشخيص فقط. يحتاج نسخة المختبر التجريبية واختبارًا فعليًا للموديل."
  if(!row.implemented)return unavailable[row.id]||row.reason
  return row.available?"":app.trx("تحقق من الجهاز والوضع وقراءة القدرات الحديثة.","")+"\n"+row.reason
 }
 function review(row){selectedAction=row;reviewedTarget=toolBridge.selectedSerial;owner.checked=false;loss.checked=false;compatible.checked=false;replace.checked=false;challenge.text="";packagePath.text="";packageHash.text="";confirm.open()}
 ColumnLayout {anchors.fill:parent;spacing:10
  RowLayout {Layout.fillWidth:true
   NavIcon {kind:1;Layout.preferredWidth:32;Layout.preferredHeight:32}
   Label {text:app.trx("خدمات الجهاز حسب قدراته","Capability-based device services");font.bold:true;font.pixelSize:19;color:app.tone("#edf4ff");Layout.fillWidth:true;wrapMode:Text.WordWrap}
   NeonComboBox {id:brand;objectName:"serviceBrand";model:page.brands;Layout.preferredWidth:190}
   ActionButton {objectName:"serviceOfficialDrivers";text:app.trx("التعريفات الرسمية","Official drivers");enabled:!!page.driverSource;onClicked:Qt.openUrlExternally(page.driverSource.url)}
  }
  Label {objectName:"serviceBuildProfile";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:toolBridge.laboratoryBuild?"#FF465C":"#39FF14";text:toolBridge.laboratoryBuild?app.trx("نسخة المختبر التجريبية — عمليات تغيير الجهاز تحتاج تفويضًا واختبارًا فعليًا لكل موديل.","Experimental Laboratory build — device changes require authorization and per-model physical testing."):app.trx("نسخة الإنتاج: قراءة وتشخيص فقط. عمليات تغيير الجهاز غير متاحة.","Production build: read-only diagnostics. Device-changing operations are unavailable.")}
  Label {objectName:"serviceWorkflow";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#C8D2DD";text:app.trx("توصيل الجهاز ← اكتشافه ← قراءة المعلومات ← التشخيص ← اختيار العملية ← مراجعة المخاطر ← التنفيذ ← التقرير","Connect → Discover → Read information → Diagnose → Select operation → Review risks → Run → Report")}
  Label {objectName:"serviceDriverUnavailableReason";Layout.fillWidth:true;visible:!page.driverSource;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("لم يُوثّق رابط تعريف رسمي لهذه الشركة بعد. طابق الموديل ووضع الاتصال ومصدر التعريف قبل التثبيت.","No official driver source is recorded for this brand yet. Verify the exact model, connection mode and driver source before installation.")}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8c1d7");text:app.trx("اختيار الشركة يرشّح المراجع فقط. تُقرأ هوية الجهاز وقدراته من الاتصال الحالي. العربية والإنجليزية والفرنسية والتركية تتبع اللغات الموجودة في Firmware؛ لن تُضاف ترجمة غير موجودة.","The brand filter selects references only. Device identity/capabilities come from the current connection. Arabic, English, French and Turkish depend on the languages present in the installed firmware.")}
  RowLayout {Layout.fillWidth:true
   NeonComboBox {id:device;objectName:"serviceDevice";Layout.fillWidth:true;model:toolBridge.devices;textRole:"serial";valueRole:"serial";currentIndex:toolBridge.devices.findIndex(function(row){return row.serial===toolBridge.selectedSerial});displayText:currentIndex<0?app.trx("اختر جهازًا بعد الفحص","Select a device after discovery"):currentText;enabled:!toolBridge.busy;onActivated:toolBridge.selectDevice(currentValue)}
   ActionButton {text:"ADB";enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
   ActionButton {text:"Fastboot";enabled:!toolBridge.busy;onClicked:toolBridge.scanFastboot()}
   ActionButton {objectName:"cancelDeviceService";text:app.trx("إيقاف","Cancel");enabled:toolBridge.busy;onClicked:toolBridge.cancel()}
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#51d6a2");text:toolBridge.selectedSerial+" · "+(toolBridge.serviceProfile.vendor||toolBridge.serviceProfile.product||"")+" · "+(toolBridge.serviceProfile["ro.product.model"]||"")+" · "+(toolBridge.serviceProfile.verifiedAtUtc||app.trx("لم تُقرأ القدرات بعد","Capabilities not read yet"))}
  Label {Layout.fillWidth:true;visible:!!toolBridge.serviceProfile.serial;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8bfd4");text:"Android: "+(toolBridge.serviceProfile["ro.build.version.release"]||"?")+" · SoC: "+(toolBridge.serviceProfile["ro.soc.manufacturer"]||"?")+" / "+(toolBridge.serviceProfile["ro.soc.model"]||"?")+" · Bootloader: "+(toolBridge.serviceProfile.unlocked||toolBridge.serviceProfile["ro.boot.flash.locked"]||"?")+" · "+app.trx("القيم كما أبلغ عنها الجهاز؛ تعريف USB غير مرتبط بهذه القراءة","Device-reported values; USB driver binding is not correlated")}
  RowLayout {Layout.fillWidth:true
   Label {Layout.fillWidth:true;text:page.phases[toolBridge.servicePhase]?(app.ar?page.phases[toolBridge.servicePhase][0]:page.phases[toolBridge.servicePhase][1]):"";color:app.tone("#eab775")}
   NeonComboBox {id:group;objectName:"serviceGroup";model:["All","ADB","Fastboot","Recovery","OEM / Restricted"];Layout.preferredWidth:175}
  }
  ProgressBar {objectName:"deviceServiceProgress";Layout.fillWidth:true;indeterminate:toolBridge.busy;visible:toolBridge.busy}
  RowLayout {Layout.fillWidth:true;Layout.fillHeight:true;spacing:10
   ListView {id:list;objectName:"deviceServiceList";Layout.preferredWidth:Math.min(410,page.width*0.48);Layout.fillHeight:true;clip:true;spacing:8
    model:toolBridge.serviceActions.filter(function(row){return group.currentIndex===0||(group.currentIndex===1&&row.group==="adb")||(group.currentIndex===2&&row.group==="fastboot")||(group.currentIndex===3&&row.group==="recovery")||(group.currentIndex===4&&(row.group==="oem"||row.group==="restricted"))})
    ScrollBar.vertical:ScrollBar {}
    delegate:GlossSurface {required property var modelData;width:list.width;implicitHeight:content.implicitHeight+24;color:app.tone("#0b2131");radius:8;border.color:modelData.available?app.tone("#356756"):app.tone("#234a63");reflection:0.06
     ColumnLayout {id:content;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;spacing:7
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:app.ar?modelData.ar:modelData.en;color:app.tone("#edf4ff");font.bold:true}
      Label {Layout.fillWidth:true;visible:!modelData.available;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:page.explain(modelData);color:app.tone("#a8bfd4");font.pixelSize:12}
      Label {visible:modelData.risk==="destructive";Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("قد تُحذف البيانات أو يتعطل الإقلاع؛ لا يمكن التراجع عن كتابة بدأت على الهاتف.","Data loss or boot failure is possible; changes already started on the phone cannot be undone.");color:app.tone("#e0787b");font.pixelSize:12}
      ActionButton {objectName:"serviceAction_"+modelData.id;Layout.fillWidth:true;visible:modelData.implemented;text:app.trx("مراجعة وتنفيذ","Review and run");enabled:modelData.available;onClicked:page.review(modelData)}
     }
    }
   }
   ScrollView {id:detailsScroll;objectName:"deviceServiceDetailsScroll";implicitWidth:0;implicitHeight:0;Layout.minimumWidth:0;Layout.minimumHeight:0;Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
    ColumnLayout {width:detailsScroll.availableWidth;spacing:10
    DeviceCompatibilityPanel {app:page.app;profile:toolBridge.serviceProfile.compatibility||({});Layout.fillWidth:true}
    Panel {objectName:"deviceHealthPanel";title:app.trx("قراءات تشخيص الجهاز","Device diagnostic observations");Layout.fillWidth:true;visible:!!page.diagnostics.schemaVersion
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8bfd4");text:app.trx("قراءات لحظية وليست شهادة سلامة للبطارية أو الذاكرة. البيانات غير المتاحة لا تعني أن الجهاز سليم.","Point-in-time observations, not battery or storage health certification. Missing data does not mean the device is healthy.")}
     Label {objectName:"deviceBatterySummary";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#39ff14";text:app.trx("شحن البطارية: ","Battery charge: ")+(page.battery.chargePercent!==undefined?Number(page.battery.chargePercent).toFixed(1)+"%":app.trx("غير متاح","Unavailable"))+" · "+app.trx("الحرارة: ","Temperature: ")+(page.battery.temperatureC!==undefined?Number(page.battery.temperatureC).toFixed(1)+" °C":app.trx("غير متاحة","Unavailable"))}
     Label {Layout.fillWidth:true;visible:!!page.battery.updatesStopped;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("أبلغ Android عن إيقاف تحديثات البطارية؛ قد تكون القيم محاكاة وليست قراءة حالية.","Android reports stopped battery updates; values may be simulated rather than current readings.")}
     Label {objectName:"deviceStorageSummary";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:"#39ff14";text:app.trx("مساحة /data المتاحة: ","Available /data storage: ")+(page.storage.state==="observed"?(Number(page.storage.availableKiB)/1048576).toFixed(2)+" GiB / "+(Number(page.storage.totalKiB)/1048576).toFixed(2)+" GiB":app.trx("غير متاحة في وضع الاتصال والصلاحيات الحالية","Unavailable in the current mode or permissions"))}
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8bfd4");text:app.trx("أجهزة التخزين الظاهرة للنظام: ","Kernel-visible block devices: ")+(page.partitions.entries?page.partitions.entries.length:app.trx("غير متاحة","Unavailable"))+" · "+app.trx("ليست خريطة أقسام معتمدة للتفليش","Not a validated firmware partition map")}
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8bfd4");text:app.trx("وقت القراءة UTC: ","Observed UTC: ")+(page.diagnostics.observedAtUtc||"");textFormat:Text.PlainText}
    }
    Panel {objectName:"deviceServiceResultPanel";title:app.trx("نتيجة العملية — بيانات محلية","Operation result — local data");Layout.fillWidth:true;Layout.minimumHeight:200;Layout.preferredHeight:Math.max(200,detailsScroll.availableHeight-references.implicitHeight-history.implicitHeight-20)
     ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {objectName:"deviceServiceOutput";readOnly:true;selectByMouse:true;text:toolBridge.serviceOutput;wrapMode:TextEdit.Wrap;color:app.tone("#c7deef");background:null;textFormat:TextEdit.PlainText}}
    }
    Panel {id:references;title:app.trx("الموديلات المطلوبة — مراجع غير معتمدة بعد","Requested models — not yet validated");Layout.fillWidth:true
     NeonComboBox {objectName:"requestedServiceModel";Layout.fillWidth:true;model:toolBridge.requestedModels.filter(function(row){return row.brand===brand.currentText});textRole:"model";enabled:count>0;displayText:count>0?currentText:app.trx("لا توجد مراجع موديلات مسجلة لهذه الشركة","No model references recorded for this brand")}
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("إضافة اسم موديل ليست اعتمادًا لعمليات FRP أو Format أو Firmware الخاصة به.","A model reference does not validate its FRP, format or firmware operations.");color:app.tone("#eab775");font.pixelSize:12}
    }
    Panel {id:history;title:app.trx("سجل خدمات الجلسة · UTC","Session service history · UTC");Layout.fillWidth:true
     ActionButton {objectName:"exportDeviceServiceReport";text:app.trx("تصدير تقرير محلي","Export local report");enabled:!toolBridge.busy&&toolBridge.serviceHistory.length>0;onClicked:reportConsent.open()}
     ListView {objectName:"deviceServiceHistory";Layout.fillWidth:true;Layout.preferredHeight:80;clip:true;model:toolBridge.serviceHistory;ScrollBar.vertical:ScrollBar {}delegate:ItemDelegate {required property var modelData;width:ListView.view.width;onClicked:page.reviewRecord(modelData);contentItem:Label {text:modelData.atUtc+" · "+modelData.serial+" · "+modelData.transport+" · "+modelData.action+" · "+(modelData.success?app.trx("اكتملت","Completed"):app.trx("توقفت","Stopped"));textFormat:Text.PlainText;wrapMode:Text.WrapAnywhere;color:app.tone("#a8bfd4");font.pixelSize:11}}}
    }
    }
   }
  }
 }
 Dialog {id:historyDetail;objectName:"deviceServiceHistoryDetail";parent:Overlay.overlay;anchors.centerIn:parent;width:Math.min(670,app.width-48);height:Math.min(480,app.height-96);modal:true;title:app.trx("تفاصيل العملية المسجلة","Recorded operation details");standardButtons:Dialog.Close
  contentItem:ScrollView {clip:true;contentWidth:availableWidth
   TextArea {objectName:"deviceServiceHistoryText";readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;color:app.tone("#c7deef");background:null;text:(page.reviewedRecord.serial||"")+" · "+(page.reviewedRecord.transport||"")+"\n"+(page.reviewedRecord.action||"")+" · "+(page.reviewedRecord.atUtc||"")+"\n\n"+(page.reviewedRecord.result||"")}
  }
 }
 FileDialog {id:packageFile;fileMode:FileDialog.OpenFile;nameFilters:page.selectedAction.id==="sideload_zip"?["Recovery package (*.zip)"]:["Android package (*.apk)"];onAccepted:packagePath.text=selectedFile.toString()}
 Dialog {id:reportConsent;parent:Overlay.overlay;anchors.centerIn:parent;width:Math.min(550,app.width-48);modal:true;title:app.trx("تقرير غير مشفر","Plaintext report");standardButtons:Dialog.Ok|Dialog.Cancel;onAccepted:reportFile.open()
  Label {width:reportConsent.availableWidth;wrapMode:Text.WordWrap;color:app.tone("#edf4ff");text:app.trx("يتضمن التقرير معرف الجهاز ونتائج الفحوص المحلية. أوافق على حفظه في ملف جديد ومراجعة البيانات قبل مشاركته.","This report contains device identifiers and local diagnostics. I approve saving a new plaintext file and reviewing it before sharing.")}
 }
 FileDialog {id:reportFile;fileMode:FileDialog.SaveFile;nameFilters:["Service report (*.json)"];onAccepted:toolBridge.exportServiceReport(selectedFile.toString(),true)}
 Dialog {id:confirm;objectName:"deviceServiceConfirm";parent:Overlay.overlay;anchors.centerIn:parent;width:Math.min(670,app.width-48);modal:true;title:app.ar?page.selectedAction.ar||"":page.selectedAction.en||"";standardButtons:Dialog.Ok|Dialog.Cancel
  readonly property bool fileAction:page.selectedAction.id==="install_apk"||page.selectedAction.id==="sideload_zip"
  readonly property bool destructive:page.selectedAction.risk==="destructive"
  onOpened:standardButton(Dialog.Ok).enabled=Qt.binding(function(){return owner.checked&&(page.selectedAction.id==="probe"||challenge.text===page.reviewedTarget)&&(!confirm.destructive||loss.checked)&&(!confirm.fileAction||packagePath.text!=="")&&(page.selectedAction.id!=="sideload_zip"||(compatible.checked&&/^[a-fA-F0-9]{64}$/.test(packageHash.text.trim())))})
  onAccepted:toolBridge.startService(page.selectedAction.id,page.reviewedTarget,{authorized:owner.checked,destructive:loss.checked,challenge:challenge.text,file:packagePath.text,sha256:packageHash.text,compatible:compatible.checked,replace:replace.checked})
  contentItem:ScrollView {id:confirmScroll;contentWidth:availableWidth;implicitHeight:Math.min(520,app.height-200);clip:true
   ColumnLayout {width:confirmScroll.availableWidth;spacing:12
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:app.trx("الجهاز المستهدف: ","Target device: ")+page.reviewedTarget;color:app.tone("#edf4ff");font.bold:true}
    NeonCheckBox {id:owner;objectName:"serviceOwnerConsent";Layout.fillWidth:true;text:app.trx("أقر بتفويض مالك الجهاز لتنفيذ هذه العملية","I have the device owner's authorization for this operation")}
    Label {Layout.fillWidth:true;visible:page.selectedAction.id!=="probe";wrapMode:Text.WordWrap;text:app.trx("اكتب معرف الجهاز كما يظهر أعلاه لتأكيد الاختيار","Type the exact device serial shown above to confirm the target");color:app.tone("#a8bfd4")}
    TextField {id:challenge;objectName:"serviceTargetChallenge";Layout.fillWidth:true;visible:page.selectedAction.id!=="probe";placeholderText:page.reviewedTarget}
    NeonCheckBox {id:loss;objectName:"serviceDataLossConsent";visible:confirm.destructive;Layout.fillWidth:true;text:app.trx("أوافق على فقدان البيانات ومخاطر التوقف؛ لا يوجد تراجع","I accept data loss and interruption risks; there is no rollback")}
    Label {Layout.fillWidth:true;visible:confirm.destructive;wrapMode:Text.WordWrap;color:app.tone("#e0787b");text:app.trx("قد يستمر الجهاز في عملية بدأت حتى بعد إلغاء النقل من الكمبيوتر. مسح Userdata لا يزيل FRP ولا يستعيد البيانات المشفرة.","The device may continue a started operation after host cancellation. Userdata erasure does not remove FRP or recover encrypted data.")}
    RowLayout {visible:confirm.fileAction;Layout.fillWidth:true;TextField {id:packagePath;objectName:"servicePackagePath";Layout.fillWidth:true;readOnly:true;placeholderText:app.trx("ملف محلي موثوق ومصرح به","Authorized trusted local package")}ActionButton {text:app.trx("اختيار ملف","Choose file");onClicked:packageFile.open()}}
    TextField {id:packageHash;objectName:"servicePackageHash";visible:confirm.fileAction;Layout.fillWidth:true;placeholderText:app.trx("SHA-256 من مصدر موثوق — إلزامي لملف ZIP","Reviewed source SHA-256 — required for ZIP")}
    NeonCheckBox {id:compatible;objectName:"serviceCompatibilityConsent";visible:page.selectedAction.id==="sideload_zip";Layout.fillWidth:true;text:app.trx("راجعت تطابق ZIP مع الموديل وإصداره وRecovery","I checked ZIP compatibility with the exact model, build and recovery")}
    NeonCheckBox {id:replace;visible:page.selectedAction.id==="install_apk";Layout.fillWidth:true;text:app.trx("السماح باستبدال التطبيق الموجود مع الاحتفاظ ببياناته","Allow updating the existing application while retaining its data")}
    Label {Layout.fillWidth:true;visible:page.selectedAction.id==="languages";wrapMode:Text.WordWrap;text:app.trx("ستفتح إعدادات اللغة؛ اختر العربية أو English أو Français أو Türkçe على الهاتف إن كانت متاحة. لا تُمنح صلاحيات Root ولا تُضاف لغات غير موجودة في Firmware.","Language settings will open. Choose العربية, English, Français or Türkçe on the device if available. No root is granted and no missing firmware translations are installed.");color:app.tone("#eab775")}
    Label {Layout.fillWidth:true;visible:page.selectedAction.id==="sideload_zip";wrapMode:Text.WordWrap;text:app.trx("اختر ADB Sideload من Recovery/TWRP على الهاتف أولًا ثم أعد فحص ADB. لا يثبّت البرنامج TWRP ولا يثبت نجاح التفليش من انتهاء النقل وحده.","Select ADB Sideload in recovery/TWRP on the device first, then rescan ADB. This does not install TWRP; transfer completion alone does not prove a successful flash.");color:app.tone("#eab775")}
   }
  }
 }
}

