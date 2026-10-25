import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ScrollView {
 id:page
 objectName:"lawEvidenceWorkspace"
 property var app
 property bool custodyOnly:false
 property string path:""
 property string selectedId:""
 property bool verified:false
 property var lastExport:({})
 contentWidth:availableWidth;clip:true
 Connections {target:lawCaseBridge;function onChanged(){page.verified=false;copyConsent.checked=false;page.lastExport=({})}}
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {visible:!page.custodyOnly;title:app.trx("تسجيل دليل محلي محمي","Register protected local evidence");iconName:"document_lock";Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تُحفظ نسخة محمية وبصمة SHA-256، دون فتح الملف أو تشغيله. الحد 16 MiB. الحيازة إقرارات مسجلة وليست توقيعات تسليم مستقلة.","Stores a protected copy and SHA-256 without opening or executing the file. Limit 16 MiB. Custody entries are declarations, not independent handover signatures.")}
   ComboBox {id:device;objectName:"lawEvidenceDevice";Layout.fillWidth:true;model:lawCaseBridge.devices;textRole:"model"}
   RowLayout {Layout.fillWidth:true
    ActionButton {objectName:"lawChooseEvidence";text:app.trx("اختيار ملف","Choose file");onClicked:chooser.open()}
    TextField {objectName:"lawEvidencePath";Layout.fillWidth:true;text:page.path;readOnly:true;placeholderText:app.trx("ملف محلي مصرح بفحصه","Authorized local file")}
   }
   TextField {id:description;objectName:"lawEvidenceDescription";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("وصف الدليل","Evidence description")}
   TextField {id:custodian;objectName:"lawEvidenceCustodian";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("المسؤول الحالي","Current custodian")}
   CheckBox {id:consent;objectName:"lawEvidenceConsent";text:app.trx("أقر بنطاق التفويض ونسخ الملف إلى القضية","Confirm scope and copy file into case")}
   ActionButton {objectName:"lawRegisterEvidence";text:app.trx("حفظ الدليل وبصمته","Store evidence and digest");primary:true;enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.addEvidence&&device.currentIndex>=0&&page.path!==""&&consent.checked;onClicked:{const id=lawCaseBridge.addEvidence(lawCaseBridge.devices[device.currentIndex].id,page.path,description.text,custodian.text,consent.checked);if(id!==""){page.selectedId=id;page.path="";consent.checked=false}}}
  }
  Panel {title:page.custodyOnly?app.trx("سلسلة الحيازة","Chain of custody"):app.trx("التحقق ونسخ العمل","Verification and working copies");iconName:"history";Layout.fillWidth:true
   ComboBox {id:evidence;objectName:"lawEvidenceSelector";Layout.fillWidth:true;model:lawCaseBridge.activeCase.evidence||[];textRole:"name";onActivated:{page.selectedId=model[currentIndex].id;page.verified=false}}
   ActionButton {objectName:"lawVerifyEvidence";visible:!page.custodyOnly;text:app.trx("التحقق من النسخة المحفوظة","Verify stored copy");enabled:evidence.currentIndex>=0;onClicked:page.verified=lawCaseBridge.verifyEvidence(evidence.model[evidence.currentIndex].id)}
   Label {objectName:"lawEvidenceVerified";visible:page.verified&&!page.custodyOnly;color:app.design.success;text:app.trx("تطابق الحجم وSHA-256 للنسخة المحفوظة","Stored-copy size and SHA-256 match")}
   TextField {id:copyPurpose;visible:!page.custodyOnly;objectName:"lawCopyPurpose";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("الغرض من نسخة العمل","Working-copy purpose")}
   TextField {id:copyDirectory;visible:!page.custodyOnly;objectName:"lawCopyDirectory";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("مسار مجلد حزمة جديد داخل مجلد موجود","New package directory path inside an existing folder")}
   CheckBox {id:copyConsent;visible:!page.custodyOnly;objectName:"lawCopyConsent";text:app.trx("أوافق على تصدير نسخة عمل غير مشفرة","Approve unencrypted working-copy export")}
   ActionButton {objectName:"lawExportWorkingCopy";visible:!page.custodyOnly;text:app.trx("تصدير نسخة عمل وManifest","Export working copy & manifest");enabled:lawCaseBridge.permissions.exportEvidence&&evidence.currentIndex>=0&&copyPurpose.text.trim()!==""&&copyDirectory.text.trim()!==""&&copyConsent.checked;onClicked:{const result=lawCaseBridge.exportWorkingCopy(evidence.model[evidence.currentIndex].id,copyDirectory.text,copyPurpose.text,copyConsent.checked);page.lastExport=result}}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تبقى النسخة الأصلية المحمية دون تغيير. احتفظ ببصمة Manifest منفصلة، وتحقق بالأداة verify_evidence_package.py دون تشغيل الدليل.","The protected original remains unchanged. Retain the manifest digest separately; verify with verify_evidence_package.py without executing evidence.")}
   TextArea {objectName:"lawCopyManifestDigest";visible:!!page.lastExport.manifestSha256;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;text:page.lastExport.manifestSha256||"";color:app.design.success}
   Repeater {model:page.custodyOnly?[]:(lawCaseBridge.activeCase.workingCopies||[]);delegate:TextArea {required property var modelData;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;text:modelData.purpose+"\nManifest SHA-256: "+modelData.manifestSha256}}
   TextField {id:from;visible:page.custodyOnly;objectName:"lawCustodyFrom";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("من: المسؤول المسجل حاليًا","From: recorded current custodian")}
   TextField {id:to;visible:page.custodyOnly;objectName:"lawCustodyTo";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("إلى: المسؤول المستلم","To: receiving custodian")}
   TextField {id:reason;visible:page.custodyOnly;objectName:"lawCustodyReason";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("سبب النقل","Transfer reason")}
   ActionButton {objectName:"lawTransferCustody";visible:page.custodyOnly;text:app.trx("تسجيل نقل الحيازة","Record custody transfer");enabled:lawCaseBridge.permissions.custody&&evidence.currentIndex>=0;onClicked:if(lawCaseBridge.transferCustody(evidence.model[evidence.currentIndex].id,from.text,to.text,reason.text)){from.text="";to.text="";reason.text=""}}
   Repeater {model:lawCaseBridge.activeCase.evidence||[];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(modelData,null,2)}}
   Repeater {model:page.custodyOnly?(lawCaseBridge.activeCase.custody||[]):[];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:JSON.stringify(modelData)}}
  }
 }
 FileDialog {id:chooser;fileMode:FileDialog.OpenFile;onAccepted:page.path=selectedFile.toString()}
}
