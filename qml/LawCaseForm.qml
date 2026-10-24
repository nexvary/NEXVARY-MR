import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 property var app
 property var form:({})
 property int step:0
 property bool editing:false
 signal saved()
 readonly property var stages:app.ar?["بيانات القضية","السند والنطاق","الضبط والملاحظات","المراجعة والحفظ"]:["Case header","Authority & scope","Seizure & notes","Review & save"]
 readonly property var definitions:[
  [{key:"caseNumber",ar:"رقم القضية *",en:"Case number *",max:128},{key:"investigatingAgency",ar:"جهة الضبط *",en:"Investigating agency *",max:256},{key:"investigatorName",ar:"المحقق المسؤول *",en:"Responsible investigator *",max:256},{key:"jurisdiction",ar:"الولاية القضائية",en:"Jurisdiction",max:256}],
  [{key:"authorityReference",ar:"رقم الإذن أو السند *",en:"Authority reference *",max:256},{key:"issuingAuthority",ar:"الجهة المصدرة *",en:"Issuing authority *",max:256},{key:"prosecutorName",ar:"اسم وكيل النيابة",en:"Prosecutor name",max:256},{key:"prosecutorOffice",ar:"النيابة المختصة",en:"Prosecutor office",max:256},{key:"courtName",ar:"المحكمة",en:"Court",max:256},{key:"validFromUtc",ar:"بداية السريان UTC — اختياري",en:"Valid from UTC — optional",max:30},{key:"validUntilUtc",ar:"نهاية السريان UTC — اختياري",en:"Valid until UTC — optional",max:30},{key:"scope",ar:"نطاق التفويض المعلن *",en:"Recorded authority scope *",max:8192,multiline:true}],
  [{key:"seizureAtUtc",ar:"وقت الضبط UTC",en:"Seizure time UTC",max:30},{key:"seizurePlace",ar:"مكان الضبط",en:"Seizure place",max:1024},{key:"seizedDeviceDescription",ar:"وصف المضبوطات *",en:"Seized items description *",max:2048,multiline:true},{key:"incidentDescription",ar:"وصف الواقعة",en:"Incident description",max:8192,multiline:true},{key:"notes",ar:"الملاحظات",en:"Notes",max:8192,multiline:true}]
 ]
 function setField(key,value){let next=Object.assign({},form);next[key]=value;form=next}
 function label(key){for(const group of definitions)for(const field of group)if(field.key===key)return app.ar?field.ar:field.en;return key}
 function startNew(){form={};step=0;editing=false}
 function edit(record){form=Object.assign({},record);step=0;editing=true}
 function input(key){for(let i=0;i<fields.count;i++){let item=fields.itemAt(i);if(item&&item.modelData.key===key)return item.editor}return null}
 ColumnLayout {
  anchors.fill:parent;spacing:12
  Label {text:(page.editing?app.trx("تعديل القضية — ","Edit case — "):app.trx("قضية جديدة — ","New case — "))+(step+1)+" / 4 · "+stages[step];Layout.fillWidth:true;color:app.design.text;font.bold:true;font.pixelSize:20;wrapMode:Text.WordWrap}
  Flow {Layout.fillWidth:true;spacing:8
   Repeater {model:page.stages;delegate:ActionButton {required property int index;required property string modelData;text:(index+1)+" · "+modelData;primary:page.step===index;implicitHeight:34;onClicked:page.step=index}}
  }
  ScrollView {
   id:scroll;objectName:"lawFormScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
   ColumnLayout {
    width:scroll.availableWidth;spacing:12
    Panel {Layout.fillWidth:true;title:page.stages[page.step];iconName:page.step===1?"document_lock":"clipboard_task"
     Label {visible:page.step===1;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تُسجل المستندات والنطاق للمراجعة البشرية. اكتمال الحقول ليس اعتمادًا قانونيًا ولا إذنًا آليًا لفحص جهاز. لا يُشترط حكم إدانة نهائي. التاريخ الاختياري بصيغة 2026-10-24T10:00:00Z.","Record declarations and scope for human review. Completeness is not legal approval or automatic device permission. No final conviction is required. Optional time format: 2026-10-24T10:00:00Z.")}
     GridLayout {
      id:formGrid;visible:page.step<3;Layout.fillWidth:true;columns:width>850?2:1;columnSpacing:16;rowSpacing:12
      Repeater {id:fields;model:page.step<3?page.definitions[page.step]:[]
       delegate:ColumnLayout {
        id:field
        required property var modelData
        readonly property var editor:modelData.multiline?multiline:single
        Layout.fillWidth:true;Layout.minimumWidth:0;Layout.columnSpan:modelData.multiline&&formGrid.columns===2?2:1;spacing:4
        Label {text:page.app.ar?modelData.ar:modelData.en;color:page.app.design.muted;Layout.fillWidth:true;wrapMode:Text.WordWrap}
        TextField {id:single;objectName:"lawField-"+modelData.key;visible:!modelData.multiline;Layout.fillWidth:true;maximumLength:modelData.max;text:page.form[modelData.key]||"";onTextEdited:page.setField(modelData.key,text);selectByMouse:true}
        TextArea {id:multiline;objectName:"lawText-"+modelData.key;visible:!!modelData.multiline;Layout.fillWidth:true;Layout.preferredHeight:100;wrapMode:TextEdit.Wrap;text:page.form[modelData.key]||"";selectByMouse:true;onTextChanged:if(activeFocus){if(text.length>modelData.max)text=text.slice(0,modelData.max);page.setField(modelData.key,text)}}
       }
      }
     }
     Repeater {model:page.step===3?Object.keys(page.form):[]
      delegate:Label {required property string modelData;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:page.label(modelData)+": "+page.form[modelData]}
     }
     Label {visible:page.step===3;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("الحفظ محلي ومحمي عند توفر Windows DPAPI. راجع التفاصيل والنطاق قبل الحفظ. لا تبدأ هذه الشاشة أي عملية على الهاتف.","Saving uses local protected storage when Windows DPAPI is available. Review identity and scope before saving. This form starts no phone operation.")}
    }
   }
  }
  RowLayout {
   Layout.fillWidth:true
   ActionButton {objectName:"lawFormBack";text:app.trx("السابق","Previous");enabled:page.step>0;onClicked:page.step--}
   Item {Layout.fillWidth:true}
   ActionButton {objectName:"lawFormNext";visible:page.step<3;text:app.trx("التالي","Next");primary:true;onClicked:page.step++}
   ActionButton {objectName:"lawSaveCase";visible:page.step===3;text:app.trx("حفظ بيانات القضية","Save case record");primary:true;enabled:lawCaseBridge.ready&&(page.editing?lawCaseBridge.permissions.edit:lawCaseBridge.permissions.create);onClicked:{const ok=page.editing?lawCaseBridge.updateCase(page.form):lawCaseBridge.createCase(page.form)!=="";if(ok)page.saved()}}
  }
 }
}
