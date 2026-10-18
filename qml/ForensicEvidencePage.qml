import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 objectName:"forensicEvidenceWorkspace"
 property string selectedId:""
 property bool editingOperator:false
 property var filteredEvidence: {
  const items=recoveryBridge.evidence;
  const query=search.text.toLowerCase();
  let rows=[];
  for(let i=0;i<items.length;i++) {
   const e=items[i];
   if((kind.currentIndex===0||e.kind===kind.model[kind.currentIndex])&&
      (e.name+" "+e.sha256+" "+e.id).toLowerCase().indexOf(query)>=0)rows.push({sourceIndex:i,entry:e});
  }
  return rows;
 }
 readonly property int selectedIndex: {
  const entries=recoveryBridge.evidence;
  for(let i=0;i<entries.length;i++)if(entries[i].id===selectedId)return i;
  return -1;
 }
 function notify(ok){app.notice(ok,app.trx("اكتملت العملية","Operation completed"),recoveryBridge.error)}
 ColumnLayout {
  anchors.fill:parent;spacing:10
  Label {text:recoveryBridge.active?recoveryBridge.caseTitle+" · "+recoveryBridge.caseId:app.trx("أنشئ أو افتح قضية من تبويب القضايا","Create or open a case in the Cases tab");Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#8ac7f5");font.bold:true}
  RowLayout {
   Layout.fillWidth:true;Layout.minimumWidth:0
   TextField {id:search;objectName:"forensicSearch";placeholderText:app.trx("بحث بالاسم أو المعرّف أو البصمة","Search name, ID or SHA256");Layout.fillWidth:true;Layout.minimumWidth:0}
   ComboBox {id:kind;objectName:"forensicKind";model:[app.trx("كل الأنواع","All types"),"logical-copy","carved-candidate","agent-metadata","external-analysis"];Layout.preferredWidth:180}
   ActionButton {objectName:"forensicVerify";text:app.trx("تحقق SHA-256","Verify SHA-256");enabled:recoveryBridge.active&&!recoveryBridge.busy;onClicked:notify(recoveryBridge.verifyEvidence())}
  }
  RowLayout {
   Layout.fillWidth:true;Layout.minimumWidth:0;Layout.fillHeight:true;spacing:10
   ListView {
    id:files;objectName:"forensicEvidenceList";Layout.preferredWidth:Math.max(220,page.width*0.38);Layout.fillHeight:true;clip:true;spacing:8;model:page.filteredEvidence
    ScrollBar.vertical:ScrollBar {}
    delegate:GlossSurface {
     required property var modelData
     width:ListView.view.width;height:96;radius:8;color:page.selectedId===modelData.entry.id?app.tone("#173b57"):app.tone("#0d2538");border.color:app.tone("#284b65")
     ColumnLayout {
      anchors.fill:parent;anchors.margins:10;spacing:5
      Label {text:modelData.entry.name;Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideMiddle;textFormat:Text.PlainText;color:app.tone("#d3e8fa");font.bold:true}
      Label {text:modelData.entry.kind+" · "+modelData.entry.size+" B";Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideRight;color:app.tone("#8cb2ce");font.pixelSize:12}
      Label {text:modelData.entry.id;Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideMiddle;color:app.tone("#77b7e5");font.pixelSize:11}
     }
     MouseArea {anchors.fill:parent;onClicked:page.selectedId=modelData.entry.id}
    }
    Label {anchors.centerIn:parent;width:parent.width-20;visible:files.count===0;text:app.trx("لا توجد أدلة مطابقة","No matching evidence");wrapMode:Text.WordWrap;color:app.tone("#9bbbd3");horizontalAlignment:Text.AlignHCenter}
   }
   ScrollView {
    id:detail;objectName:"forensicDetailScroll";Layout.preferredWidth:400;Layout.fillWidth:true;Layout.minimumWidth:0;Layout.fillHeight:true;clip:true;contentWidth:availableWidth
    ColumnLayout {
     width:detail.availableWidth;spacing:10
     Panel {
      title:app.trx("هوية المشغّل والتفويض","OPERATOR AND AUTHORIZATION");Layout.fillWidth:true;Layout.minimumWidth:0
      RowLayout {
       Layout.fillWidth:true
      Label {text:app.trx("المشغّل الحالي: ","Current operator: ")+(recoveryBridge.caseOperator||app.trx("لم يُسجّل","Not recorded"));Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#d3e8fa")}
       ActionButton {text:app.trx("تحرير","Edit");visible:recoveryBridge.caseOperator!=="";onClicked:page.editingOperator=!page.editingOperator}
      }
      ColumnLayout {
       visible:page.editingOperator||recoveryBridge.caseOperator==="";Layout.fillWidth:true
      TextField {id:operatorName;objectName:"forensicOperator";placeholderText:app.trx("اسم أو معرف المشغّل","Operator name or identifier");maximumLength:160;Layout.fillWidth:true;Layout.minimumWidth:0}
      TextField {id:authorization;objectName:"forensicAuthorization";placeholderText:app.trx("مرجع التفويض ونطاقه","Authorization reference and scope");maximumLength:500;Layout.fillWidth:true;Layout.minimumWidth:0}
      CheckBox {id:operatorConsent;objectName:"forensicOperatorConsent";text:app.trx("أؤكد صحة البيانات المسجلة","I confirm these recorded details")}
      ActionButton {objectName:"forensicRecordOperator";text:app.trx("حفظ هوية المشغّل","Record operator");enabled:recoveryBridge.active&&!recoveryBridge.busy&&operatorConsent.checked&&operatorName.text.trim()!==""&&authorization.text.trim()!=="";onClicked:{const ok=recoveryBridge.recordOperator(operatorName.text,authorization.text,true);notify(ok);if(ok)page.editingOperator=false;operatorConsent.checked=false}}
      }
     }
     Panel {
      title:app.trx("الدليل المحدد","SELECTED EVIDENCE");Layout.fillWidth:true;Layout.minimumWidth:0
      Label {text:page.selectedIndex>=0?JSON.stringify(recoveryBridge.evidence[page.selectedIndex],null,2):app.trx("اختر دليلًا لعرض معلوماته","Select evidence to inspect metadata");textFormat:Text.PlainText;Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WrapAnywhere;color:app.tone("#9bbbd3");font.pixelSize:12}
      TextArea {objectName:"forensicPreview";Layout.fillWidth:true;Layout.minimumWidth:0;Layout.preferredHeight:150;readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;textFormat:TextEdit.PlainText;text:page.selectedIndex>=0?recoveryBridge.previewEvidence(page.selectedIndex):""}
      Label {text:app.trx("الحائز المسجل: ","Recorded custodian: ")+(page.selectedIndex>=0?recoveryBridge.evidenceCustodian(page.selectedIndex):"");Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#63d7b1")}
      ActionButton {objectName:"forensicCustody";text:app.trx("تسجيل نقل الحيازة","Record custody transfer");enabled:page.selectedIndex>=0&&!recoveryBridge.busy&&recoveryBridge.caseOperator!=="";onClicked:custody.open()}
      Label {text:app.trx("سجل محلي مترابط بالبصمات؛ لا يثبت التسليم المادي أو اعتمادًا قضائيًا. سجلات القضايا القديمة تُميّز بأنها غير متحقق منها تاريخيًا.","Local hash-linked declarations do not prove physical handover or court accreditation. Migrated historical entries are marked unverified.");Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3");font.pixelSize:12}
     }
    }
   }
  }
  RowLayout {
   Layout.fillWidth:true;Layout.minimumWidth:0
   CheckBox {id:plaintext;objectName:"forensicPlaintext";text:app.trx("أوافق على تصدير غير مشفر","Authorize plaintext export")}
   ComboBox {id:format;objectName:"forensicFormat";model:["PDF","HTML","JSON"];Layout.preferredWidth:100}
   ActionButton {objectName:"forensicReport";text:app.trx("تصدير تقرير","Export report");enabled:recoveryBridge.active&&!recoveryBridge.busy&&plaintext.checked;onClicked:report.open()}
   ActionButton {text:app.trx("تصدير الدليل","Export evidence");enabled:page.selectedIndex>=0&&!recoveryBridge.busy&&plaintext.checked;onClicked:evidence.open()}
   Item {Layout.fillWidth:true;Layout.minimumWidth:0}
  }
  ProgressBar {Layout.fillWidth:true;Layout.minimumWidth:0;visible:recoveryBridge.busy;indeterminate:recoveryBridge.progress===0;value:recoveryBridge.progress}
  RowLayout {Layout.fillWidth:true;Layout.minimumWidth:0;Label {text:recoveryBridge.status;Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:recoveryBridge.error?app.tone("#ecb774"):app.tone("#9bbbd3");font.pixelSize:12}ActionButton {text:app.trx("إلغاء","Cancel");visible:recoveryBridge.busy;onClicked:recoveryBridge.cancel()}}
 }
 FileDialog {id:report;fileMode:FileDialog.SaveFile;defaultSuffix:format.currentText.toLowerCase();nameFilters:[format.currentText+" (*."+format.currentText.toLowerCase()+")"];onAccepted:notify(format.currentText==="JSON"?recoveryBridge.exportReport(selectedFile.toString(),plaintext.checked):recoveryBridge.exportFormattedReport(selectedFile.toString(),format.currentText.toLowerCase(),app.ar,plaintext.checked))}
 FileDialog {id:evidence;fileMode:FileDialog.SaveFile;onAccepted:notify(recoveryBridge.exportEvidence(page.selectedIndex,selectedFile.toString(),plaintext.checked))}
 Dialog {
  id:custody;objectName:"forensicCustodyDialog";title:app.trx("إقرار نقل الحيازة","Custody transfer declaration");anchors.centerIn:parent;modal:true;width:Math.min(500,page.width-20);standardButtons:Dialog.Ok|Dialog.Cancel
  onOpened:{recipient.text="";reason.text="";standardButton(Dialog.Ok).enabled=false}
  ColumnLayout {
   width:parent.width;spacing:12
   Label {text:app.trx("يسجل المشغّل إقرارًا بالتسليم؛ لا يرسل البرنامج ملفات إلى الشخص المذكور.","The operator records a handover declaration; this does not send any files.");Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap}
   TextField {id:recipient;objectName:"forensicCustodyRecipient";placeholderText:app.trx("المستلم أو جهة الحيازة","Recipient or custody location");maximumLength:160;Layout.fillWidth:true;Layout.minimumWidth:0;onTextChanged:if(custody.opened)custody.standardButton(Dialog.Ok).enabled=text.trim()!==""&&reason.text.trim()!==""}
   TextField {id:reason;objectName:"forensicCustodyReason";placeholderText:app.trx("سبب النقل","Transfer reason");maximumLength:500;Layout.fillWidth:true;Layout.minimumWidth:0;onTextChanged:if(custody.opened)custody.standardButton(Dialog.Ok).enabled=text.trim()!==""&&recipient.text.trim()!==""}
  }
  onAccepted:notify(recoveryBridge.transferCustody(page.selectedIndex,recipient.text,reason.text,true))
 }
}
