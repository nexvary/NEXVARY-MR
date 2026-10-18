import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ScrollView {
 id:page
 property var app
 property var guide:({})
 property var node:({})
 property var answers:[]
 property int boundOrder:-1
 property string snapshot:""
 objectName:"faultTreePage"
 contentWidth:availableWidth;clip:true
 function start(){const order=workshopBridge.invoice(orderChoice.currentValue||-1);boundOrder=order.id||-1;guide=serviceCatalog.tree(order.brand||"",order.model||"",symptom.currentText,app.ar);answers=[];snapshot="";chooseNode(guide.root||"")}
 function chooseNode(id){node=({});for(const n of (guide.nodes||[]))if(n.id===id){node=n;return}}
 function answer(yes){const a=answers.slice();a.push({id:node.id,question:app.ar?node.ar:node.en,answer:yes});answers=a;chooseNode(yes?node.yes:node.no)}
 function historyText(){let s=(guide.brand||"")+" "+(guide.model||"")+" / "+(guide.symptom||"")+"\n"+(guide.scope||"")+"\n"+(guide.sourceUrl||"")+"\n";for(const a of answers)s+=a.id+": "+a.question+" => "+(a.answer?"YES":"NO")+"\n";return s+(node.ar?(app.ar?node.ar:node.en):"")}
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;Layout.preferredHeight:245;title:app.trx("شجرة التشخيص المرتبطة بالهاتف","Phone-bound diagnostic tree")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("الأجوبة نتائج يدخلها الفني، وليست قياسات تلقائية. الدليل الخاص لا يُستخدم إلا إذا طابق الشركة والموديل والعَرَض تمامًا؛ وإلا يظهر مسار عام بحدوده. لا توجد مكتبة موثقة لكل الموديلات بعد.","Answers are technician observations, not automatic measurements. An imported guide is used only for an exact brand/model/symptom match; otherwise a general triage path is explicitly labeled. A verified library for all models is not yet supplied.")}
   ComboBox {id:orderChoice;objectName:"faultTreeOrder";Layout.fillWidth:true;model:workshopBridge.orders;textRole:"id";valueRole:"id";onCurrentValueChanged:{page.guide=({});page.node=({});page.boundOrder=-1}}
   RowLayout {Layout.fillWidth:true;ComboBox {id:symptom;Layout.fillWidth:true;model:repairBridge.supportedSymptoms();onCurrentTextChanged:{page.guide=({});page.node=({})}}ActionButton {objectName:"startFaultTree";text:app.trx("بدء شجرة الفحص","Start fault tree");primary:true;enabled:orderChoice.currentValue>0;onClicked:page.start()}}
  }
  Panel {Layout.fillWidth:true;Layout.preferredHeight:310;title:app.trx("الخطوة الحالية","Current step")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:(page.guide.brand||"")+" "+(page.guide.model||"")+"\n"+(page.guide.scope||"");color:app.tone("#8eb6d6")}
   Label {objectName:"faultTreeQuestion";Layout.fillWidth:true;Layout.fillHeight:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:(app.ar?page.node.ar:page.node.en)||app.trx("اختر أمر صيانة ثم ابدأ","Choose a repair order and start");color:app.tone("#e0ecfa");font.pixelSize:18}
   RowLayout {ActionButton {objectName:"faultTreeYes";text:app.trx("نعم","Yes");enabled:!!page.node.yes;onClicked:page.answer(true)}ActionButton {objectName:"faultTreeNo";text:app.trx("لا","No");enabled:!!page.node.no;onClicked:page.answer(false)}ActionButton {text:app.trx("إعادة البدء","Restart");enabled:page.boundOrder>0;onClicked:page.start()}}
  }
  Panel {Layout.fillWidth:true;Layout.preferredHeight:380;title:app.trx("توثيق النتيجة الفعلية","Record actual findings")
   TextField {id:technician;objectName:"treeTechnician";Layout.fillWidth:true;maximumLength:160;placeholderText:app.trx("الفني المسؤول *","Technician *")}
   TextArea {id:finding;objectName:"treeFinding";Layout.fillWidth:true;Layout.preferredHeight:110;wrapMode:TextEdit.Wrap;placeholderText:app.trx("ما الذي قسته أو لاحظته فعليًا؟ *","What did you actually measure or observe? *");onTextChanged:if(text.length>8000)text=text.slice(0,8000)}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:page.answers.length+app.trx(" إجابات مسجلة؛ يُحفظ المسار مع أمر الهاتف المحدد."," recorded answers; the path is saved with the selected phone's repair order.");color:app.tone("#a8c1d7")}
   ActionButton {objectName:"saveFaultTree";text:app.trx("حفظ مسار الفحص والنتيجة","Save path and findings");enabled:page.boundOrder>0&&page.node.terminal===true&&technician.text.trim()!==""&&finding.text.trim()!=="";onClicked:app.notice(workshopBridge.saveDiagnosis(page.boundOrder,page.guide.symptom,page.historyText(),finding.text,page.guide.sourceUrl||"General triage; no exact-model guide",technician.text),app.trx("حُفظ التشخيص في سجل الهاتف","Saved in the phone's repair history"))}
  }
  Panel {Layout.fillWidth:true;Layout.preferredHeight:200;title:app.trx("مكتبة أدلة الموديلات","Model guide library")
   CheckBox {id:rights;Layout.fillWidth:true;text:app.trx("راجعت مصدر الدليل وترخيص استخدامه ودقة الموديل","I reviewed guide source, usage rights and exact model")}
   ActionButton {objectName:"importModelGuide";text:app.trx("استيراد شجرة موديل JSON","Import model tree JSON");enabled:rights.checked;onClicked:guidePicker.open()}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:serviceCatalog.guides.length+app.trx(" أدلة موديلات محفوظة. "," saved model guides. ")+serviceCatalog.error;color:app.tone("#eab775")}
  }
 }
 FileDialog {id:guidePicker;fileMode:FileDialog.OpenFile;nameFilters:["Model guide (*.json)"];onAccepted:serviceCatalog.importGuide(selectedFile.toString(),rights.checked)}
}
