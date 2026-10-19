import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ScrollView {
 id:page
 property var app
 objectName:"securityPage"
 contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:165;title:app.trx("فحص ملفات الهاتف وAPK — ClamAV","Phone file & APK scanning — ClamAV")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("فحص ملف محلي مصرح به، أو نسخة APK حصلت عليها بحق الوصول. لا يفحص الهاتف كله أو ذاكرته أو بياناته المشفرة. لا توجد حماية لحظية أو إزالة تلقائية. عدم العثور على توقيع لا يثبت سلامة الهاتف.","Scan one authorized local file or lawfully obtained APK copy. This does not scan the entire phone, memory or encrypted data. No real-time protection or automatic removal. No signature match does not prove the phone is clean.")}
   ActionButton {text:app.trx("تنزيل ClamAV من موقعه الرسمي","Official ClamAV downloads");onClicked:Qt.openUrlExternally("https://www.clamav.net/downloads")}
  }
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:240;title:app.trx("إعداد المحرك وقاعدة التواقيع","Configure engine and signatures")
   RowLayout {Layout.fillWidth:true;TextField {id:engine;objectName:"scannerExecutable";Layout.fillWidth:true;text:securityBridge.executable;placeholderText:app.trx("مسار clamscan المثبت","Installed clamscan path")}ActionButton {text:app.trx("اختيار المحرك","Choose engine");enabled:!securityBridge.busy;onClicked:enginePicker.open()}}
   RowLayout {Layout.fillWidth:true;TextField {id:database;objectName:"scannerDatabase";Layout.fillWidth:true;text:securityBridge.database;placeholderText:app.trx("مجلد التواقيع المحدّث بواسطة FreshClam","Signature directory updated using FreshClam")}ActionButton {text:app.trx("قاعدة التواقيع","Signatures");enabled:!securityBridge.busy;onClicked:databasePicker.open()}}
   RowLayout {ActionButton {objectName:"configureScanner";text:app.trx("حفظ إعداد الفحص","Save scanner settings");enabled:!securityBridge.busy;onClicked:securityBridge.configure(engine.text,database.text)}ActionButton {text:app.trx("شرح تحديث FreshClam الرسمي","Official FreshClam update guide");onClicked:Qt.openUrlExternally("https://docs.clamav.net/manual/Usage/SignatureManagement.html")}}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8c1d7");text:app.trx("تُرفض حاويات CVD الأقدم من 7 أيام من خلال المحرك. القواعد الخاصة تُعرض بتغطية خاصة فقط. قد تستهلك القاعدة الرسمية ذاكرة كبيرة؛ لا يُرفق المحرك أو التواقيع مع البرنامج.","The engine rejects CVD containers older than 7 days. Custom rules are labeled as custom coverage. Official databases can use substantial memory; engine and signatures are separately installed.")}
  }
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:270;title:app.trx("تحديث قاعدة الفيروسات — FreshClam","Update virus signatures — FreshClam")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8c1d7");text:app.trx("ثبّت FreshClam بجوار clamscan واختر مجلد تواقيع قابلًا للكتابة. ينزّل التواقيع الرسمية فقط؛ لا يرسل ملفات العميل. قد يكون التنزيل الأول كبيرًا. الإلغاء لا يتراجع عن قواعد اكتمل استبدالها.","Install FreshClam beside clamscan and select a writable database directory. Downloads official signatures only; customer files are not sent. Initial downloads can be large. Cancellation does not roll back databases already replaced.")}
   Label {objectName:"signatureUpdaterPath";Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#a8c1d7");text:securityBridge.updater||app.trx("FreshClam غير موجود بجوار المحرك المحدد","FreshClam not found beside the selected engine")}
   CheckBox {id:updateConsent;objectName:"signatureUpdateConsent";text:app.trx("أوافق على تنزيل وتحديث التواقيع الرسمية","Approve official signature download and update");enabled:!securityBridge.busy}
   RowLayout {
    ActionButton {objectName:"updateSignatures";text:app.trx("تحديث التواقيع الآن","Update signatures now");enabled:!securityBridge.busy&&updateConsent.checked&&securityBridge.updater!=="";onClicked:securityBridge.updateDatabase(updateConsent.checked)}
    ActionButton {objectName:"cancelSignatureUpdate";text:app.trx("إلغاء التحديث","Cancel update");enabled:securityBridge.updating;onClicked:securityBridge.cancel()}
   }
   Label {objectName:"signatureUpdateStatus";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#eab775");text:securityBridge.updating?app.trx("جارٍ تنزيل التواقيع والتحقق منها…","Downloading and checking signatures…"):securityBridge.updateResult.error||((securityBridge.updateResult.success===true)?app.trx("اكتمل التحديث — افحص الملف للحصول على نتيجة","Update completed — scan the file for a verdict"):app.trx("لم يُجرَ تحديث في هذه الجلسة","No update in this session"))}
   ScrollView {Layout.fillWidth:true;Layout.preferredHeight:80;visible:!!securityBridge.updateResult.output;TextArea {objectName:"signatureUpdateLog";readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;textFormat:TextEdit.PlainText;text:securityBridge.updateResult.output||"";color:app.tone("#c6daeb");background:null}}
  }
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:230;title:app.trx("بدء الفحص المصرح به","Start authorized scan")
   RowLayout {Layout.fillWidth:true;TextField {id:source;objectName:"securitySource";Layout.fillWidth:true;placeholderText:app.trx("ملف أو APK محلي — حتى 128 MiB","Local file or APK — up to 128 MiB")}ActionButton {text:app.trx("اختيار الملف","Choose file");enabled:!securityBridge.busy;onClicked:sourcePicker.open()}}
   CheckBox {id:consent;objectName:"securityConsent";Layout.fillWidth:true;text:app.trx("أملك تفويضًا لفحص هذا الملف أو نسخة الهاتف","I am authorized to scan this file or phone copy")}
   RowLayout {ActionButton {objectName:"startSecurityScan";text:app.trx("فحص الفيروسات","Scan for malware");primary:true;enabled:!securityBridge.busy&&consent.checked&&source.text!==""&&securityBridge.executable!=="";onClicked:securityBridge.scan(source.text,consent.checked)}ActionButton {objectName:"cancelSecurityScan";text:app.trx("إلغاء العملية","Cancel operation");enabled:securityBridge.busy;onClicked:securityBridge.cancel()}}
   ProgressBar {Layout.fillWidth:true;indeterminate:securityBridge.busy;visible:securityBridge.busy}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:securityBridge.error;color:app.tone("#eab775")}
  }
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:360;title:app.trx("نتيجة الفحص وحدوده","Scan result and scope")
   Label {objectName:"securityVerdict";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:securityBridge.result.verdict||app.trx("لم يُجرَ فحص بعد","No scan performed yet");color:app.tone("#e0ecfa");font.bold:true}
   ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;text:JSON.stringify(securityBridge.result,null,2);wrapMode:TextEdit.Wrap;color:app.tone("#c6daeb");background:null}}
   CheckBox {id:exportConsent;text:app.trx("أوافق على تصدير التقرير الذي يتضمن مسار الملف","Approve exporting this report including the file path")}
   ActionButton {objectName:"exportSecurityReport";text:app.trx("تصدير التقرير JSON","Export JSON report");enabled:!securityBridge.busy&&!!securityBridge.result.verdict&&exportConsent.checked;onClicked:reportPicker.open()}
  }
 }
 FileDialog {id:enginePicker;fileMode:FileDialog.OpenFile;onAccepted:engine.text=selectedFile.toString()}
 FolderDialog {id:databasePicker;onAccepted:database.text=selectedFolder.toString()}
 FileDialog {id:sourcePicker;fileMode:FileDialog.OpenFile;onAccepted:source.text=selectedFile.toString()}
 FileDialog {id:reportPicker;fileMode:FileDialog.SaveFile;defaultSuffix:"json";onAccepted:securityBridge.exportReport(selectedFile.toString(),exportConsent.checked)}
}
