import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 property string analyzedSymptom:""
 property string guidanceSnapshot:""
 property var savedEntries:workshopBridge.diagnoses(orderChooser.currentValue||-1)
 function refreshOrders() {
  const previous=orderChooser.currentValue||-1
  const choices=[{id:-1,label:app.trx("اختر أمر الصيانة المرتبط صراحةً","Explicitly select the repair order")}].concat(workshopBridge.orders.map(function(o){return {id:o.id,label:"#"+o.id+" · "+o.customer+" · "+o.brand+" "+o.model}}))
  orderChooser.model=choices
  let selected=0
  for(let i=0;i<choices.length;i++)if(choices[i].id===previous)selected=i
  orderChooser.currentIndex=selected
  page.savedEntries=workshopBridge.diagnoses(orderChooser.currentValue||-1)
 }
 Component.onCompleted:refreshOrders()
 Connections {target:workshopBridge;function onChanged(){page.refreshOrders()}}
 Connections {target:app;function onArChanged(){page.refreshOrders()}}
 ScrollView {
  id:scroll;objectName:"diagnosisScroll";anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:14
   Panel {
    iconName:"gauge";title:app.trx("مسار التشخيص الإرشادي","Guided diagnostic workflow");Layout.fillWidth:true
    Label {text:app.trx("1  حدد العَرَض   ←   2  راجع الإرشادات والدليل   ←   3  احفظ ملاحظات الفني لأمر محدد","1  Select symptom   →   2  Review guidance and evidence   →   3  Save findings to a specific order");color:app.tone("#69b7ff");Layout.fillWidth:true;wrapMode:Text.WordWrap}
    RowLayout {
     Layout.fillWidth:true
     NeonComboBox {id:symptom;objectName:"symptomChoice";Layout.fillWidth:true;model:repairBridge.supportedSymptoms();onCurrentTextChanged:{page.analyzedSymptom="";page.guidanceSnapshot=""}}
     ActionButton {objectName:"diagnoseButton";text:app.trx("تحليل العَرَض","Analyze symptom");primary:true;onClicked:{page.analyzedSymptom=symptom.currentText;page.guidanceSnapshot=repairBridge.diagnoseLocalized(page.analyzedSymptom,app.ar)}}
    }
    Flickable {id:guidanceScroll;Layout.fillWidth:true;Layout.preferredHeight:page.guidanceSnapshot!==""?220:96;clip:true;contentWidth:width;contentHeight:result.height;boundsBehavior:Flickable.StopAtBounds;ScrollBar.vertical:ScrollBar {}
    TextArea {id:result;objectName:"diagnosisResult";width:guidanceScroll.width;height:Math.max(guidanceScroll.height,contentHeight+topPadding+bottomPadding);readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;text:page.guidanceSnapshot||app.trx("اختر العَرَض ثم اضغط تحليل لتوليد إرشادات قابلة للحفظ.","Select a symptom, then Analyze to generate guidance for saving.");color:app.tone("#d0e0ef");font.pixelSize:16;background:Rectangle{color:app.tone("#0c2232");radius:6}}
    }
    Label {text:app.trx("إرشادات قائمة على قواعد وليست تشخيصًا مؤكدًا. الدليل والملاحظات بالأسفل تُدخل يدويًا؛ لا تُنسخ سجلات الهاتف تلقائيًا.","Rule-based guidance, not a confirmed diagnosis. Findings and reviewed evidence below are entered manually; phone logs are never copied automatically.");color:app.tone("#eab775");wrapMode:Text.WordWrap;Layout.fillWidth:true}
   }
   Panel {
    iconName:"clipboard_task";title:app.trx("توثيق نتائج التشخيص","Save diagnostic findings");Layout.fillWidth:true
    NeonComboBox {
     id:orderChooser;objectName:"diagnosisOrder";Layout.fillWidth:true;textRole:"label";valueRole:"id"
     model:[]
     onCurrentValueChanged:page.savedEntries=workshopBridge.diagnoses(currentValue||-1)
    }
    Label {Layout.fillWidth:true;text:app.trx("العَرَض الذي ستُحفظ إرشاداته: ","Guidance will be saved for: ")+page.analyzedSymptom;color:app.tone("#8eb6d6");textFormat:Text.PlainText}
    TextField {id:technician;objectName:"diagnosisTechnician";Layout.fillWidth:true;maximumLength:160;placeholderText:app.trx("اسم الفني *","Technician name *")}
    TextArea {id:findings;objectName:"diagnosisFindings";Layout.fillWidth:true;Layout.preferredHeight:100;wrapMode:TextEdit.Wrap;placeholderText:app.trx("ملاحظات الفني ونتائج الفحص الفعلية * (حتى 8000 حرف)","Technician observations and actual findings * (up to 8000 characters)");onTextChanged:if(text.length>8000)text=text.slice(0,8000)}
    TextArea {id:evidence;objectName:"diagnosisEvidence";Layout.fillWidth:true;Layout.preferredHeight:85;wrapMode:TextEdit.Wrap;placeholderText:app.trx("الدليل الذي راجعته يدويًا، وظروف الاختبار والقيود (حتى 4000 حرف)","Manually reviewed evidence, test conditions and limitations (up to 4000 characters)");onTextChanged:if(text.length>4000)text=text.slice(0,4000)}
    RowLayout {
     Layout.fillWidth:true
     ActionButton {objectName:"saveDiagnosis";text:app.trx("حفظ النتائج لأمر الصيانة","Save findings to repair order");primary:true;enabled:page.analyzedSymptom!==""&&page.guidanceSnapshot!==""&&orderChooser.currentValue>0&&findings.text.trim()!==""&&technician.text.trim()!=="";onClicked:{const ok=workshopBridge.saveDiagnosis(orderChooser.currentValue,page.analyzedSymptom,page.guidanceSnapshot,findings.text,evidence.text,technician.text);app.notice(ok,app.trx("حُفظت النتائج والإرشادات مع أمر الصيانة","Findings and guidance saved with repair order"));if(ok){findings.clear();evidence.clear()}}}
     ActionButton {objectName:"exportDiagnosis";text:app.trx("تصدير التقرير JSON","Export JSON report");enabled:page.savedEntries.length>0;onClicked:reportPicker.open()}
    }
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("يتضمن التقرير بيانات العميل والملاحظات المحفوظة. راجعه قبل المشاركة. تظهر النتائج أيضًا في ملف PDF الخاص بالأمر.","Reports contain customer data and saved notes. Review before sharing. Saved findings also appear in the order PDF.");color:app.tone("#c9aa78");font.pixelSize:12}
   }
   Panel {
    iconName:"document_lock";title:app.trx("النتائج المحفوظة للأمر المختار","Saved findings for selected order");Layout.fillWidth:true
    ListView {
     Layout.fillWidth:true;Layout.preferredHeight:page.savedEntries.length?Math.min(240,contentHeight):0;visible:page.savedEntries.length>0;clip:true;spacing:8;model:page.savedEntries
     delegate:Label {required property var modelData;width:ListView.view.width;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8c1d7");text:modelData.created_at+" · "+modelData.technician+" · "+modelData.symptom+"\n"+modelData.findings}
    }
    Label {visible:page.savedEntries.length===0;text:app.trx("لا توجد نتائج محفوظة لهذا الأمر بعد","No saved findings for this order yet");color:app.tone("#7e9ab1")}
   }
   Panel {
    iconName:"phone";title:app.trx("مكتبة الشركات والعائلات","Brands and device families");Layout.fillWidth:true
    TextField {id:catalogSearch;placeholderText:app.trx("البحث عن شركة أو عائلة أجهزة","Search brand or device family");Layout.fillWidth:true}
    ListView {
     Layout.fillWidth:true;Layout.preferredHeight:160;clip:true;model:repairBridge.brands()
     delegate:Label {required property var modelData;width:ListView.view.width;visible:(modelData.brand+modelData.families.join(" ")).toLowerCase().indexOf(catalogSearch.text.toLowerCase())>=0;height:visible?32:0;text:modelData.brand+"   /   "+modelData.families.join(" · ");color:app.tone("#a8bed3");textFormat:Text.PlainText;elide:Text.ElideRight}
    }
   }
  }
 }
 FileDialog {id:reportPicker;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["Diagnostic report (*.json)"];onAccepted:app.notice(workshopBridge.exportDiagnosisReport(orderChooser.currentValue,selectedFile.toString()),app.trx("حُفظ تقرير التشخيص المحلي","Local diagnostic report saved"))}
}
