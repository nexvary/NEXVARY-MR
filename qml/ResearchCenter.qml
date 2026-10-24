import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"researchCenter"
 property var app
 property int section:0
 property var selected:({})
 property var rows:[]
 property var form:({})
 property string evidencePath:""
 readonly property var categories:["dashboard","atlas","tools","compatibility","experiments","results","validation","reports","training"]
 readonly property var titles:app.ar?["لوحة الأبحاث","أطلس التقنيات","ذكاء الأدوات الجنائية","مصفوفة التوافق","خطط التجارب","نتائج التجارب","موثوقية الأبحاث","تقارير الأبحاث","محاكاة التدريب"]:["Research dashboard","Technology Atlas","Forensic Tool Intelligence","Compatibility Matrix","Research Experiments","Experiment Results","Research Validation","Research Reports","Training Simulations"]
 readonly property var resultFields:[{key:"experimentId",label:"Experiment ID"},{key:"question",label:"Research question"},{key:"hypothesis",label:"Hypothesis"},{key:"model",label:"Device / synthetic model"},{key:"osBuild",label:"OS build"},{key:"firmware",label:"Firmware version"},{key:"environment",label:"Lab environment"},{key:"equipment",label:"Required equipment"},{key:"scope",label:"Authorized scope"},{key:"methodology",label:"Methodology"},{key:"expected",label:"Expected observations"},{key:"actual",label:"Actual observations"},{key:"researcher",label:"Researcher"}]
 readonly property var matrixFields:[{key:"manufacturer",label:"Manufacturer"},{key:"model",label:"Model"},{key:"chipset",label:"Chipset"},{key:"osBuild",label:"OS build"},{key:"firmware",label:"Firmware"},{key:"securityPatch",label:"Security patch"},{key:"connectionMode",label:"Connection mode"},{key:"capability",label:"Documented capability"},{key:"unsupported",label:"Unsupported operations"},{key:"sourceId",label:"Source ID"},{key:"lastTestUtc",label:"Last test UTC / not tested"},{key:"result",label:"Test outcome / not tested"},{key:"confidence",label:"Evidence confidence"}]
 function fieldTitle(key,fallback){const ar={experimentId:"معرّف التجربة",question:"السؤال البحثي",hypothesis:"الفرضية",model:"الموديل أو النموذج الاصطناعي",osBuild:"إصدار نظام التشغيل",firmware:"إصدار Firmware",environment:"بيئة المختبر",equipment:"المعدات المطلوبة",scope:"نطاق التفويض",methodology:"منهجية التجربة",expected:"الملاحظات المتوقعة",actual:"النتيجة الفعلية",researcher:"الباحث",manufacturer:"الشركة المصنعة",chipset:"المعالج",securityPatch:"التصحيح الأمني",connectionMode:"وضع الاتصال",capability:"الوظيفة الموثقة",unsupported:"الوظائف غير المدعومة",sourceId:"معرّف المصدر",lastTestUtc:"آخر اختبار UTC أو لم يُختبر",result:"نتيجة الاختبار أو لم يُختبر",confidence:"درجة الثقة"};return app.ar?(ar[key]||fallback):fallback}
 function refresh(){rows=researchBridge.search(categories[section],query.text);selected={}}
 function setField(key,value){let next=Object.assign({},form);next[key]=value;form=next}
 onSectionChanged:{form={};refresh()}
 Component.onCompleted:refresh()
 Connections {target:researchBridge;function onChanged(){page.refresh()}}
 ColumnLayout {anchors.fill:parent;spacing:10
  RowLayout {Layout.fillWidth:true
   Image {source:"qrc:/assets/icons/book_open_lightbulb.svg";Layout.preferredWidth:32;Layout.preferredHeight:32;sourceSize.width:32;sourceSize.height:32}
   Label {text:app.trx("مركز الأبحاث","Research Center");font.pixelSize:23;font.bold:true;color:app.design.text;Layout.fillWidth:true}
   StateBadge {text:"OFFLINE R&D";tint:app.design.accent}
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("الادعاءات والفرضيات منفصلة عن نتائج المختبر. لا تمنح سجلات الأبحاث صلاحية إجراء أو توافقًا تجاريًا.","Source claims, hypotheses and lab observations stay distinct. Research records grant no operation permission or commercial compatibility.")}
  Label {visible:researchBridge.error!=="";objectName:"researchError";text:researchBridge.error;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.danger;textFormat:Text.PlainText}
  RowLayout {Layout.fillWidth:true
   ComboBox {objectName:"researchSectionSelector";Layout.preferredWidth:Math.min(330,page.width*0.5);model:page.titles;currentIndex:page.section;onActivated:page.section=currentIndex}
   TextField {id:query;objectName:"researchQuery";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("بحث في السجلات والمصادر","Search records and sources");onTextEdited:page.refresh()}
   ActionButton {objectName:"researchSearch";text:app.trx("بحث","Search");onClicked:page.refresh()}
  }
  ScrollView {id:scroll;objectName:"researchScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
   ColumnLayout {width:scroll.availableWidth;spacing:12
    Panel {visible:page.section===0;title:app.trx("المحركات المشتركة وبوابات الإثبات","Shared engines and evidence gates");iconName:"gauge";Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.text;text:researchBridge.catalog.atlas.length+" Atlas · "+researchBridge.catalog.tools.length+" Tools · "+researchBridge.catalog.experiments.length+" Plans · "+researchBridge.results.length+" Results"}
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:researchBridge.engineContext.capabilityGate}
     RowLayout {Layout.fillWidth:true
      ActionButton {text:app.trx("ذكاء الجهاز","Device Intelligence");onClicked:page.app.navigateSection(1)}
      ActionButton {text:app.trx("تشخيص USB","USB Diagnostics");onClicked:page.app.navigateSection(4)}
      ActionButton {text:app.trx("ملفات النظام","Firmware Intelligence");onClicked:page.app.navigateSection(3)}
     }
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.design.muted;textFormat:Text.PlainText;text:JSON.stringify({propertiesCurrent:researchBridge.engineContext.deviceIntelligence.propertiesCurrent,usbScope:researchBridge.engineContext.deviceIntelligence.usbScope,avbStatus:researchBridge.engineContext.firmwareAvb.status||"not inspected",packageStatus:researchBridge.engineContext.firmwarePackage.status||"not inspected",modelAcceptance:researchBridge.engineContext.modelAcceptance},null,2)}
    }
    Repeater {model:page.rows
     delegate:Panel {required property var modelData;Layout.fillWidth:true;title:page.app.ar?(modelData.titleAr||modelData.title||modelData.model||modelData.id):(modelData.title||modelData.model||modelData.id);iconName:page.section===2?"search_visual":"document_text";stateText:modelData.status||"Pending"
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;textFormat:Text.PlainText;text:modelData.claim||modelData.advertised||modelData.question||modelData.actual||modelData.capability||""}
      ActionButton {text:page.app.trx("عرض المصدر والقيود","View provenance and limits");onClicked:{page.selected=modelData;Qt.callLater(function(){scroll.contentItem.contentY=Math.max(0,details.y-8)})}}
     }
    }
    EmptyState {visible:page.rows.length===0&&page.section>0&&page.section<6;Layout.fillWidth:true;app:page.app;iconName:"search_visual";title:app.trx("لا توجد سجلات مطابقة","No matching records");description:app.trx("غيّر البحث أو سجّل نتيجة فعلية. لم تُدرج نتائج مختبرية مفترضة.","Change the search or record an actual result. No lab results have been invented.")}
    Panel {id:details;visible:Object.keys(page.selected).length>0;Layout.fillWidth:true;title:app.trx("المصدر والتفاصيل","Provenance and details");iconName:"document_text"
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.design.text;textFormat:Text.PlainText;text:JSON.stringify(page.selected,null,2)}
     Repeater {model:page.selected.sourceIds||[];delegate:Label {required property string modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:page.app.design.accent;textFormat:Text.PlainText;text:{for(const s of researchBridge.catalog.sources)if(s.id===modelData)return s.publisher+" · "+s.url+" · "+s.reviewedOn+" · "+s.rights;return modelData}}}
     ActionButton {text:app.trx("إغلاق التفاصيل","Close details");onClicked:page.selected={}}
    }
    Panel {visible:page.section===3||page.section===5;title:page.section===3?app.trx("إضافة سجل توافق بحثي","Add research compatibility record"):app.trx("تسجيل نتيجة تجربة","Record experiment result");iconName:"clipboard_task";Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("استخدم بيانات عامة أو اصطناعية فقط. سجلات القضية لا تُنسخ هنا. الحقول الإنجليزية موحدة بين المحركات.","Public or synthetic data only. Case records are not copied here. English field keys are shared across engines.")}
     GridLayout {Layout.fillWidth:true;columns:width>850?2:1;columnSpacing:12;rowSpacing:8
      Repeater {model:page.section===3?page.matrixFields:page.resultFields;delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true;Layout.minimumWidth:0
       Label {text:page.fieldTitle(modelData.key,modelData.label);Layout.fillWidth:true;color:page.app.design.muted}
       TextField {objectName:"researchField-"+modelData.key;Layout.fillWidth:true;maximumLength:4096;text:page.form[modelData.key]||"";onTextEdited:page.setField(modelData.key,text)}
      }}
     }
     ComboBox {id:status;objectName:"researchRecordStatus";Layout.fillWidth:true;model:page.section===3?["Research Hypothesis","Publicly Documented","Vendor Claimed","Unsupported"]:["Successful","Not Reproduced","Research Hypothesis"]}
     RowLayout {visible:page.section===5;Layout.fillWidth:true
      ActionButton {text:app.trx("اختيار دليل اصطناعي","Choose synthetic evidence");onClicked:evidence.open()}
      Label {Layout.fillWidth:true;elide:Text.ElideMiddle;color:app.design.muted;text:page.evidencePath;textFormat:Text.PlainText}
     }
     CheckBox {id:publicConsent;objectName:"researchPublicConsent";text:app.trx("هذه بيانات عامة أو اصطناعية مصرح بحفظها","These are public or synthetic observations authorized for storage")}
     ActionButton {objectName:"researchSaveRecord";text:app.trx("حفظ السجل","Save record");primary:true;enabled:researchBridge.ready&&publicConsent.checked;onClicked:{let fields=Object.assign({},page.form);let ok=false;if(page.section===3){fields.status=status.currentText;ok=researchBridge.addCompatibility(fields)}else{fields.reproducibility=status.currentText;ok=researchBridge.recordResult(fields,page.evidencePath,publicConsent.checked)!==""}if(ok){page.form={};publicConsent.checked=false;page.evidencePath="";page.refresh()}}}
    }
    Panel {visible:page.section===6;Layout.fillWidth:true;title:app.trx("بوابة التحقق","Validation gate");iconName:"shield_checkmark"
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("الموثق علنًا ليس نتيجة مختبرية. نجاح تجربة يحتاج دليلًا قابلًا للتكرار ومراجعة مستقلة وإصدار جهاز محدد قبل أي اعتماد تجاري. الترقية المباشرة إلى Lab Verified مرفوضة.","Public documentation is not a lab result. Successful observations need reproducible evidence, independent review and an exact device build before commercial acceptance. Direct promotion to Lab Verified is rejected.")}
     Repeater {model:researchBridge.results;delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(researchBridge.validation(modelData.id),null,2)}}
    }
    Panel {visible:page.section===7;Layout.fillWidth:true;title:app.trx("تصدير تقرير أبحاث محلي","Export local research report");iconName:"document_text"
     CheckBox {id:exportConsent;objectName:"researchExportConsent";text:app.trx("أوافق على تصدير السجلات البحثية العامة","Approve public research export")}
     ActionButton {objectName:"researchExport";text:app.trx("حفظ JSON جديد","Save new JSON");enabled:researchBridge.ready&&exportConsent.checked;onClicked:report.open()}
    }
    LawTrainingPage {visible:page.section===8;Layout.fillWidth:true;Layout.preferredHeight:600;app:page.app}
   }
  }
 }
 FileDialog {id:evidence;fileMode:FileDialog.OpenFile;onAccepted:page.evidencePath=selectedFile.toString()}
 FileDialog {id:report;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:researchBridge.exportReport(selectedFile.toString(),exportConsent.checked)}
}
