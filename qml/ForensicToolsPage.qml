import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 property string imagePath:""
 readonly property string mmlsPath:toolBridge.forensicTools.length?toolBridge.forensicTools[0].configuredPath:""
 ScrollView {
  id:scroll;anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:12
   Panel {
    title:app.trx("تحليل الأقسام — The Sleuth Kit","PARTITION ANALYSIS — THE SLEUTH KIT");Layout.fillWidth:true
    Label {text:app.trx("يحلل mmls جدول الأقسام على نسخة مؤقتة تُتحقق بصمتها. لا يُكتب على المصدر. التشغيل عملية مستقلة بصلاحيات حسابك؛ ليس صندوق عزل أمنيًا. اختر نسخة موثوقة من الأداة.","mmls lists partitions on a hash-verified temporary copy. The source is not written. It runs separately with your account permissions, without an OS sandbox. Select a trusted installed tool.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3")}
    Label {text:page.mmlsPath||app.trx("لم تختَر mmls. لا يتم تشغيل أدوات PATH تلقائيًا.","No mmls selected. PATH candidates are never launched automatically.");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#8ac7f5")}
    RowLayout {Layout.fillWidth:true;ActionButton {objectName:"chooseMmls";text:app.trx("اختيار mmls","Choose mmls");enabled:!toolBridge.busy&&!recoveryBridge.busy;onClicked:tool.open()}ActionButton {text:app.trx("المصدر الرسمي","Official source");onClicked:Qt.openUrlExternally("https://github.com/sleuthkit/sleuthkit")}ActionButton {text:app.trx("اختيار الصورة الخام","Choose raw image");enabled:!recoveryBridge.busy;onClicked:input.open()}Item {Layout.fillWidth:true}}
    Label {text:page.imagePath||app.trx("لم تُحدد صورة؛ الحد 2 GiB","No image selected; maximum 2 GiB");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#9bbbd3")}
    CheckBox {id:consent;objectName:"tskConsent";text:app.trx("أُصرّح بتحليل هذه الصورة وتشغيل الأداة المختارة","Authorize this image analysis and selected executable")}
    Label {visible:recoveryBridge.caseOperator==="";text:app.trx("افتح قضية وسجّل هوية المشغّل في مستعرض الأدلة أولًا.","Open a case and record its operator in the Evidence tab first.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#ecb774")}
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
     Label {text:modelData.adapter?app.trx("محول تحليل أقسام mmls؛ توفر الأداة مطلوب","mmls partition adapter; installed tool required"):app.trx("غير مدمج — لا يوجد زر تشغيل","Not integrated — no execution action");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3")}
     Label {text:modelData.license+"\n"+(modelData.available?modelData.path:app.trx("غير موجود في المسارات المفحوصة","Not found in checked paths"));Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#9bbbd3");font.pixelSize:12}
     ActionButton {text:app.trx("عرض المصدر الرسمي","View official source");onClicked:Qt.openUrlExternally(modelData.source)}
    }
   }
  }
 }
 FileDialog {id:tool;nameFilters:["Executable (*.exe)","All files (*)"];onAccepted:{consent.checked=false;const ok=toolBridge.configureTool("mmls",selectedFile.toString());app.notice(ok,toolBridge.status,toolBridge.status)}}
 FileDialog {id:input;nameFilters:["Raw images (*.dd *.img *.raw *.bin)"];onAccepted:{page.imagePath=selectedFile.toString();consent.checked=false}}
}
