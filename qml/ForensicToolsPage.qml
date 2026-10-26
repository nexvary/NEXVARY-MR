import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"forensicToolsPage"
 property var app
 property string imagePath:""
 property string capturePath:""
 property string exifPath:""
 signal evidenceRequested(string evidenceId)
 readonly property string lastCaptureId: {
  const entries=recoveryBridge.evidence
  for(let i=entries.length-1;i>=0;i--)if(entries[i].kind==="native-analysis"&&entries[i].name==="pcap-summary.json")return entries[i].id
  return ""
 }
 onCapturePathChanged:pcapConsent.checked=false
 onExifPathChanged:exifConsent.checked=false
 readonly property string lastExifId: {
  const entries=recoveryBridge.evidence
  for(let i=entries.length-1;i>=0;i--)if(entries[i].kind==="native-analysis"&&entries[i].name==="exif-summary.json")return entries[i].id
  return ""
 }
 readonly property string mmlsPath:toolBridge.forensicTools.length?toolBridge.forensicTools[0].configuredPath:""
 ScrollView {
  id:scroll;objectName:"forensicToolsScroll";anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:12
   Panel {
    title:app.trx("تحليل PCAP محلي — إحصاءات فقط","OFFLINE PCAP — STATISTICS ONLY");Layout.fillWidth:true
    Label {objectName:"pcapScope";text:app.trx("ملف PCAP كلاسيكي حتى 8 MiB. إحصاءات الحزم والتوقيت وحقول IP على Ethernet فقط؛ لا تُحفظ عناوين أو محتويات الحزم. PCAPNG والتقاط الشبكة وفك TLS غير مدعومة.","Classic PCAP up to 8 MiB. Packet/time statistics and Ethernet IP header declarations only; packet addresses and payloads are omitted. No PCAPNG, live capture or TLS decryption.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#9bbbd3")}
    Label {text:page.capturePath||app.trx("افتح قضية وسجّل المشغّل ثم اختر الملف المصرح به.","Open a case, record its operator, then choose the authorized file.");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#8ac7f5")}
    ActionButton {objectName:"choosePcap";text:app.trx("اختيار PCAP","Choose PCAP");enabled:!recoveryBridge.busy;onClicked:capture.open()}
    NeonCheckBox {id:pcapConsent;objectName:"pcapConsent";text:app.trx("أصرح بتحليل هذا الملف محليًا وإضافة إحصاءاته للقضية","Authorize local analysis of this file and case summary storage")}
    RowLayout {
     ActionButton {objectName:"runPcapAnalysis";text:app.trx("تحليل الملف","Analyze capture");primary:true;enabled:recoveryBridge.active&&recoveryBridge.caseOperator!==""&&!recoveryBridge.busy&&page.capturePath!==""&&pcapConsent.checked;onClicked:if(!recoveryBridge.analyzeCapture(page.capturePath,pcapConsent.checked))app.notice(false,"",recoveryBridge.error)}
     ActionButton {text:app.trx("إلغاء","Cancel");enabled:recoveryBridge.busy;onClicked:recoveryBridge.cancel()}
     ActionButton {objectName:"showPcapResult";text:app.trx("عرض النتيجة","View summary");enabled:page.lastCaptureId!==""&&!recoveryBridge.busy;onClicked:page.evidenceRequested(page.lastCaptureId)}
    }
    Label {text:app.trx("تُضاف النتيجة إلى مستعرض الأدلة بنفس البصمة وسجل التدقيق وضوابط التصدير. التوقيت والأطوال تصريحات غير موثقة من الملف.","Results use the existing evidence browser, hashes, audit and export controls. Timestamps and lengths are untrusted file declarations.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#ecb774")}
    Label {text:recoveryBridge.status;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#9bbbd3")}
   }
   Panel {
    objectName:"exifPanel"
    title:app.trx("بيانات الصور EXIF — تحليل محلي","PHOTO EXIF — LOCAL METADATA");Layout.fillWidth:true
    Label {objectName:"exifScope";text:app.trx("JPEG حتى 8 MiB. يعرض الشركة والموديل والاتجاه والأبعاد والتواريخ الخام المتاحة. بيانات الصورة قابلة للتعديل ولا تثبت هوية المصوّر أو وقت التصوير. لا تُستخرج قيم GPS أو الأرقام التسلسلية أو ملاحظات الشركة.","JPEG up to 8 MiB. Shows available make, model, orientation, declared dimensions and raw dates. Metadata is editable and does not prove camera identity or capture time. GPS values, serials and MakerNotes are omitted.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#9bbbd3")}
    Label {text:page.exifPath||app.trx("اختر صورة مصرحًا بتحليل بياناتها.","Choose a photo authorized for metadata analysis.");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#8ac7f5")}
    ActionButton {objectName:"chooseExif";text:app.trx("اختيار JPEG","Choose JPEG");enabled:!recoveryBridge.busy;onClicked:exifInput.open()}
    NeonCheckBox {id:exifConsent;objectName:"exifConsent";text:app.trx("أصرح بتحليل بيانات هذه الصورة وحفظ الملخص في القضية","Authorize this photo's metadata analysis and case summary storage")}
    Flow {
     Layout.fillWidth:true;spacing:8
     ActionButton {objectName:"runExifAnalysis";text:app.trx("تحليل EXIF","Analyze EXIF");primary:true;enabled:recoveryBridge.active&&recoveryBridge.caseOperator!==""&&!recoveryBridge.busy&&page.exifPath!==""&&exifConsent.checked;onClicked:if(!recoveryBridge.analyzeExif(page.exifPath,exifConsent.checked))app.notice(false,"",recoveryBridge.error)}
     ActionButton {text:app.trx("إلغاء","Cancel");enabled:recoveryBridge.busy;onClicked:recoveryBridge.cancel()}
     ActionButton {objectName:"showExifResult";text:app.trx("عرض الملخص","View summary");enabled:page.lastExifId!==""&&!recoveryBridge.busy;onClicked:page.evidenceRequested(page.lastExifId)}
    }
    Label {text:app.trx("تُراجع بصمة المصدر مرتين. تبقى التواريخ دون استنتاج منطقة زمنية أو إضافتها كأحداث تصوير مؤكدة. التقرير قد يحتوي بيانات تعريفية؛ تصديره يتطلب الموافقة المعتادة.","Source hash is checked twice. Dates remain raw without inferred timezone or confirmed capture events. Summary fields can be identifying; export uses the existing explicit approval.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#ecb774")}
   }
   Panel {
    title:app.trx("تحليل الأقسام — The Sleuth Kit","PARTITION ANALYSIS — THE SLEUTH KIT");Layout.fillWidth:true
    Label {text:app.trx("يحلل mmls جدول الأقسام على نسخة مؤقتة تُتحقق بصمتها. لا يُكتب على المصدر. التشغيل عملية مستقلة بصلاحيات حسابك؛ ليس صندوق عزل أمنيًا. اختر نسخة موثوقة من الأداة.","mmls lists partitions on a hash-verified temporary copy. The source is not written. It runs separately with your account permissions, without an OS sandbox. Select a trusted installed tool.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#9bbbd3")}
    Label {text:page.mmlsPath||app.trx("لم تختَر mmls. لا يتم تشغيل أدوات PATH تلقائيًا.","No mmls selected. PATH candidates are never launched automatically.");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#8ac7f5")}
    RowLayout {Layout.fillWidth:true;ActionButton {objectName:"chooseMmls";text:app.trx("اختيار mmls","Choose mmls");enabled:!toolBridge.busy&&!recoveryBridge.busy;onClicked:tool.open()}ActionButton {text:app.trx("المصدر الرسمي","Official source");onClicked:app.openExternal("https://github.com/sleuthkit/sleuthkit")}ActionButton {text:app.trx("اختيار الصورة الخام","Choose raw image");enabled:!recoveryBridge.busy;onClicked:input.open()}Item {Layout.fillWidth:true}}
    Label {text:page.imagePath||app.trx("لم تُحدد صورة؛ الحد 2 GiB","No image selected; maximum 2 GiB");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#9bbbd3")}
    NeonCheckBox {id:consent;objectName:"tskConsent";text:app.trx("أُصرّح بتحليل هذه الصورة وتشغيل الأداة المختارة","Authorize this image analysis and selected executable")}
    Label {visible:recoveryBridge.caseOperator==="";text:app.trx("افتح قضية وسجّل هوية المشغّل في مستعرض الأدلة أولًا.","Open a case and record its operator in the Evidence tab first.");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#ecb774")}
    RowLayout {Layout.fillWidth:true;ActionButton {objectName:"runTskAnalysis";text:app.trx("تحليل جدول الأقسام","Analyze partitions");primary:true;enabled:recoveryBridge.active&&recoveryBridge.caseOperator!==""&&!recoveryBridge.busy&&page.imagePath!==""&&page.mmlsPath!==""&&consent.checked;onClicked:if(!recoveryBridge.analyzePartitions(page.imagePath,page.mmlsPath,consent.checked))app.notice(false,"",recoveryBridge.error)}ActionButton {text:app.trx("إلغاء","Cancel");enabled:recoveryBridge.busy;onClicked:recoveryBridge.cancel()}Item {Layout.fillWidth:true}}
    ProgressBar {Layout.fillWidth:true;visible:recoveryBridge.busy;value:recoveryBridge.progress;indeterminate:value===0}
    Label {text:recoveryBridge.status;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#9bbbd3")}
   }
   Label {text:app.trx("مرشحو الأدوات — اكتشاف المسار لا يعني اكتمال التكامل","TOOL CANDIDATES — PATH DISCOVERY DOES NOT MEAN INTEGRATION");Layout.fillWidth:true;wrapMode:Text.WordWrap;font.bold:true;color:app.tone("#8ac7f5")}
   Repeater {
    model:toolBridge.forensicTools
    delegate:Panel {
     required property var modelData
     title:modelData.name;Layout.fillWidth:true
     Label {text:modelData.adapter?app.trx("محول تحليل أقسام mmls؛ توفر الأداة مطلوب","mmls partition adapter; installed tool required"):app.trx("غير مدمج — لا يوجد زر تشغيل","Not integrated — no execution action");Layout.fillWidth:true;wrapMode:Text.WordWrap;Layout.minimumHeight:implicitHeight;color:app.tone("#9bbbd3")}
     Label {text:modelData.license+"\n"+(modelData.available?modelData.path:app.trx("غير موجود في المسارات المفحوصة","Not found in checked paths"));Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#9bbbd3");font.pixelSize:12}
     ActionButton {text:app.trx("عرض المصدر الرسمي","View official source");onClicked:app.openExternal(modelData.source)}
    }
   }
  }
 }
 FileDialog {id:capture;nameFilters:["Classic PCAP (*.pcap *.cap)","All files (*)"];onAccepted:page.capturePath=selectedFile.toString()}
 FileDialog {id:exifInput;nameFilters:["JPEG (*.jpg *.jpeg)","All files (*)"];onAccepted:page.exifPath=selectedFile.toString()}
 FileDialog {id:tool;nameFilters:["Executable (*.exe)","All files (*)"];onAccepted:{consent.checked=false;const ok=toolBridge.configureTool("mmls",selectedFile.toString());app.notice(ok,toolBridge.status,toolBridge.status)}}
 FileDialog {id:input;nameFilters:["Raw images (*.dd *.img *.raw *.bin)"];onAccepted:{page.imagePath=selectedFile.toString();consent.checked=false}}
}
