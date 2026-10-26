import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:panel
 objectName:"governmentArtifactPanel"
 property var app
 property int resultPage:0
 property var selected:({})
 property string lastCaseId:""
 readonly property var result:lawCaseBridge.artifactsResult
 readonly property var rows:result.rows||[]
 readonly property var groups:result.digestGroups||[]
 title:app.trx("فهرس النتائج وروابط المصدر","Finding index and provenance");iconName:"search_visual"
 function technical(value){return "\u2066"+String(value||"")+"\u2069"}
 function nodeType(type){
  const names={"case":["القضية","Case"],"device":["الجهاز المسجل","Registered device"],"evidence":["الدليل المصدر","Source evidence"],"analysis":["التقرير المحفوظ","Retained report"],"finding":["النتيجة المرصودة","Observed finding"]}
  const name=names[type];return name?app.trx(name[0],name[1]):type
 }
 function valueAt(index){const item=entries.itemAt(index);return item?item.observedValue:""}
 function selectAt(index){const item=entries.itemAt(index);return item?item.selectButton:null}
 Connections {target:lawCaseBridge;function onArtifactsChanged(){panel.resultPage=0;panel.selected=({})}}
 Connections {target:lawCaseBridge;function onChanged(){const id=lawCaseBridge.activeCase.id||"";if(id!==panel.lastCaseId){query.text="";panel.lastCaseId=id}}}
 Label {objectName:"governmentArtifactScope";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("ابحث حرفيًا في النتائج المحفوظة ومراجعها. تُفحص بصمات التقارير وروابط القضية؛ لا تُقرأ ملفات الأصل من جديد. الروابط تخص المصدر المسجل فقط، ولا تثبت هوية أشخاص أو وقوع أحداث. النتائج والرسم محدودان بالمطابقات المحفوظة.","Search retained findings and references literally. Report hashes and case links are checked; original files are not reread. Links describe recorded provenance, not people or real-world events. Rows and graph are limited to retained matches.")}
 TextField {id:query;objectName:"governmentArtifactQuery";Layout.fillWidth:true;maximumLength:256;enabled:!lawCaseBridge.artifactsBusy;placeholderText:panel.app.trx("بحث عربي أو إنجليزي في القيمة أو المرجع أو البصمة","Arabic or English search in value, reference or digest")}
 Flow {Layout.fillWidth:true;spacing:8
  ActionButton {objectName:"governmentArtifactBuild";text:panel.app.trx("بناء الفهرس وروابطه","Build index and links");primary:true;enabled:lawCaseBridge.permissions.analyze&&!!lawCaseBridge.activeCase.id&&!lawCaseBridge.artifactsBusy&&!lawCaseBridge.importBusy&&!lawCaseBridge.timelineBusy;onClicked:lawCaseBridge.buildArtifacts(query.text)}
  ActionButton {objectName:"governmentArtifactCancel";text:panel.app.trx("إلغاء","Cancel");enabled:lawCaseBridge.artifactsBusy;onClicked:lawCaseBridge.cancelArtifacts()}
 }
 Label {objectName:"governmentArtifactCounts";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:panel.app.design.text;text:panel.result.schema?panel.app.trx("نتائج مرصودة: ","Observed findings: ")+panel.result.observedCount+panel.app.trx(" | مطابقات: "," | Matches: ")+panel.result.matchingCount+panel.app.trx(" | محفوظة: "," | Retained: ")+panel.rows.length+panel.app.trx(" | مطابقات مستبعدة بالحد: "," | Matches omitted by cap: ")+panel.result.omittedMatchingFindings+"\n"+panel.app.trx("عقد المصدر: ","Provenance nodes: ")+(panel.result.nodes||[]).length+panel.app.trx(" | روابط: "," | Edges: ")+(panel.result.edges||[]).length+panel.app.trx(" | تقارير استيراد مختصرة: "," | Truncated import reports: ")+panel.result.truncatedImportReports+panel.app.trx(" | تقارير مرفوضة: "," | Rejected reports: ")+panel.result.rejectedImportReports+panel.app.trx(" | تحليلات خارج هذا الفهرس: "," | Analyses outside this index: ")+panel.result.unsupportedAnalysisReports:panel.app.trx("ابنِ الفهرس بعد استيراد دليل مصرح به.","Build the index after an authorized evidence import.")}
 Label {visible:!!panel.result.schema;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("بحث حرفي دون تجذير أو تسوية الحروف والتشكيل. الحد 2048 نتيجة بعد البحث؛ التقارير قد تكون مختصرة أصلًا. لا توجد فهرسة كاملة للهاتف أو فك لمحتوى التطبيقات.","Literal search without stemming or letter/diacritic normalization. Up to 2,048 findings after search; source reports may already be truncated. No full-device index or application decoding.")}
 Label {visible:!!panel.result.schema;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:panel.app.trx("الاستعلام المنفّذ: ","Executed query: ")+(panel.result.query||panel.app.trx("كل النتائج المحتفظ بها","All retained findings"))}
 Repeater {id:entries;model:panel.rows.slice(panel.resultPage*12,(panel.resultPage+1)*12)
  delegate:Panel {required property var modelData;property alias observedValue:valueLabel.text;property alias selectButton:select;Layout.fillWidth:true;title:panel.app.trx("نتيجة مرصودة — ","Observed finding — ")+modelData.kind;iconName:"document_text"
   Label {id:valueLabel;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.text;text:modelData.value+(modelData.valueTruncated?panel.app.trx(" [قيمة مختصرة]"," [shortened value]"):"")}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:modelData.sourceName+"\n"+panel.technical(modelData.reference);LayoutMirroring.enabled:false}
   ActionButton {id:select;text:panel.app.trx("عرض مسار المصدر","Show provenance path");onClicked:panel.selected=modelData}
  }
 }
 Flow {Layout.fillWidth:true;spacing:8;visible:panel.rows.length>12
  ActionButton {objectName:"governmentArtifactPrevious";text:panel.app.trx("السابق","Previous");enabled:panel.resultPage>0;onClicked:{panel.resultPage--;panel.selected=({})}}
  ActionButton {objectName:"governmentArtifactNext";text:panel.app.trx("التالي","Next");enabled:(panel.resultPage+1)*12<panel.rows.length;onClicked:{panel.resultPage++;panel.selected=({})}}
 }
 Panel {objectName:"governmentArtifactPath";visible:!!panel.selected.id;Layout.fillWidth:true;title:panel.app.trx("مسار المصدر المسجل","Recorded provenance path");iconName:"shield_checkmark"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("القضية تسجل الجهاز، والجهاز يرتبط بالدليل؛ الدليل دخل التقرير، والتقرير يحوي النتيجة. هذا مسار بيانات، وليس علاقة بين أشخاص.","The case registers a device and its evidence; the evidence is input to the report, which contains the finding. This is a data path, not a relationship between people.")}
  Repeater {model:panel.selected.provenancePath||[];delegate:Label {required property var modelData;required property int index;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.text;text:(index+1)+". "+panel.nodeType(modelData.type)+"\n"+panel.technical(modelData.label)+"\nID: "+panel.technical(modelData.id);LayoutMirroring.enabled:false}}
  Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:"Source SHA-256: "+(panel.selected.sourceSha256||"")+"\nReport SHA-256: "+(panel.selected.reportSha256||"");LayoutMirroring.enabled:false}
 }
 Panel {objectName:"governmentArtifactDigestGroups";visible:!!panel.result.schema;Layout.fillWidth:true;title:panel.app.trx("تطابق بصمات مسجلة","Equal recorded digests");iconName:"document_lock"
  Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("تطابق البصمة المسجلة لا يثبت تطابق الملفات الحالية أو هوية صاحبها، ولا يعد تأييدًا مستقلًا. تعرض هذه البطاقة أول 8 مجموعات وأول 4 مراجع لكل مجموعة؛ ابحث بالبصمة لتضييق النتائج. المجموعات: ","Equal recorded digests do not verify current files or their owners and are not independent corroboration. This card shows the first 8 groups and 4 references per group; search a digest to narrow results. Groups: ")+panel.groups.length}
  Repeater {model:panel.groups.slice(0,8);delegate:Panel {required property var modelData;Layout.fillWidth:true;title:panel.app.trx("مراجع مسجلة: ","Recorded references: ")+modelData.memberCount;iconName:"document_text"
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.text;text:modelData.sha256;LayoutMirroring.enabled:false}
   Repeater {model:modelData.members.slice(0,4);delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:panel.nodeType(modelData.type)+": "+panel.technical(modelData.reference)+"\n"+panel.technical(modelData.evidenceId);LayoutMirroring.enabled:false}}
  }}
 }
}
