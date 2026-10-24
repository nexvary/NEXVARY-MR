import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ScrollView {
 id:page
 property var app
 property string path:""
 property string selectedId:""
 property bool verified:false
 contentWidth:availableWidth;clip:true
 Connections {target:lawCaseBridge;function onChanged(){page.verified=false}}
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {title:app.trx("تسجيل دليل محلي محمي","Register protected local evidence");iconName:"document_lock";Layout.fillWidth:true
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
  Panel {title:app.trx("التحقق ونقل الحيازة","Verification and custody transfer");iconName:"history";Layout.fillWidth:true
   ComboBox {id:evidence;objectName:"lawEvidenceSelector";Layout.fillWidth:true;model:lawCaseBridge.activeCase.evidence||[];textRole:"name";onActivated:{page.selectedId=model[currentIndex].id;page.verified=false}}
   ActionButton {objectName:"lawVerifyEvidence";text:app.trx("التحقق من النسخة المحفوظة","Verify stored copy");enabled:evidence.currentIndex>=0;onClicked:page.verified=lawCaseBridge.verifyEvidence(evidence.model[evidence.currentIndex].id)}
   Label {objectName:"lawEvidenceVerified";visible:page.verified;color:app.design.success;text:app.trx("تطابق الحجم وSHA-256 للنسخة المحفوظة","Stored-copy size and SHA-256 match")}
   TextField {id:from;objectName:"lawCustodyFrom";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("من: المسؤول المسجل حاليًا","From: recorded current custodian")}
   TextField {id:to;objectName:"lawCustodyTo";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("إلى: المسؤول المستلم","To: receiving custodian")}
   TextField {id:reason;objectName:"lawCustodyReason";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("سبب النقل","Transfer reason")}
   ActionButton {objectName:"lawTransferCustody";text:app.trx("تسجيل نقل الحيازة","Record custody transfer");enabled:lawCaseBridge.permissions.custody&&evidence.currentIndex>=0;onClicked:if(lawCaseBridge.transferCustody(evidence.model[evidence.currentIndex].id,from.text,to.text,reason.text)){from.text="";to.text="";reason.text=""}}
   Repeater {model:lawCaseBridge.activeCase.evidence||[];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(modelData,null,2)}}
   Repeater {model:lawCaseBridge.activeCase.custody||[];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:JSON.stringify(modelData)}}
  }
 }
 FileDialog {id:chooser;fileMode:FileDialog.OpenFile;onAccepted:page.path=selectedFile.toString()}
}
