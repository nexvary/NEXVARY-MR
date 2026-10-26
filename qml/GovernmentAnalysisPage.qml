import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 objectName:"governmentAnalysisWorkspace"
 property var app
 property var result:({})
 readonly property var summary:result.reportJson?JSON.parse(result.reportJson):({})
 property bool showTechnical:false
 property int timelinePage:0
 readonly property var timeline:lawCaseBridge.timelineResult
 readonly property var timelineRows:timeline.events||[]
 function displayIdentifier(value){return "\u2066"+String(value)+"\u2069"}
 function timelineExplanationAt(index){const row=timelineEntries.itemAt(index);return row?row.timeExplanation:""}
 function timeReason(code){
  const reasons={
   "explicit-offset-literal-only":["حُوّل إلى UTC وفق المنطقة الزمنية المذكورة؛ صحة وقت المصدر غير مثبتة","Converted using the stated offset; source time authenticity is unverified"],
   "shortened-source-value":["القيمة مختصرة؛ لا يمكن تحديد الوقت بدقة","The observed value was shortened; time cannot be resolved"],
   "missing-year-and-timezone":["السجل لا يحدد السنة والمنطقة الزمنية","The log omits the year and timezone"],
   "unsupported-format-or-missing-timezone":["صيغة غير مدعومة أو منطقة زمنية غير مذكورة","Unsupported format or missing timezone"],
   "invalid-calendar-date":["التاريخ غير صالح","Invalid calendar date"],
   "unsupported-submillisecond-precision":["دقة أعلى من مللي ثانية؛ احتُفظ بالقيمة دون تقريب","Submillisecond precision retained without rounding"],
   "invalid-time-or-leap-second":["وقت غير صالح أو ثانية كبيسة غير مدعومة","Invalid time or unsupported leap second"],
   "unknown-offset":["إزاحة المنطقة الزمنية غير معروفة","The timezone offset is unknown"],
   "offset-out-of-range":["إزاحة المنطقة الزمنية خارج النطاق المدعوم","Timezone offset outside the supported range"],
   "utc-out-of-range":["التحويل إلى UTC خارج نطاق التواريخ المدعوم","UTC conversion outside the supported calendar range"]
  }
  const reason=reasons[code];return reason?app.trx(reason[0],reason[1]):app.trx("وقت غير محسوم؛ راجع المصدر","Unresolved time; review the source")
 }
 Connections {target:lawCaseBridge;function onTimelineChanged(){page.timelinePage=0}}
 property var savedImport:({})
 property string lastCaseId:""
 readonly property var imported:page.savedImport.reportJson?page.savedImport:lawCaseBridge.importResult
 readonly property var importReport:page.imported.reportJson?JSON.parse(page.imported.reportJson):({})
 contentWidth:availableWidth;clip:true
 Connections {target:lawCaseBridge;function onChanged(){page.result=({});page.showTechnical=false;page.savedImport=({});importConsent.checked=false;if(page.lastCaseId!==(lawCaseBridge.activeCase.id||"")){importPurpose.text="";timelineQuery.text="";if(page.contentItem)page.contentItem.contentY=0;page.lastCaseId=lawCaseBridge.activeCase.id||""}}}
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;title:app.trx("تحليل أدلة القضية","Case evidence analysis");iconName:"shield_checkmark"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تحليل محلي لنسخة القضية المحمية بعد التحقق من SHA-256. الحد 8 MiB. يدعم إحصاءات classic PCAP وحقول JPEG EXIF المحدودة دون فك تشفير أو تشغيل الملف.","Local analysis of hash-verified protected case evidence, up to 8 MiB. Supports classic-PCAP statistics and bounded JPEG-EXIF fields without decryption or file execution.")}
   Label {Layout.fillWidth:true;textFormat:Text.PlainText;color:app.design.text;text:lawCaseBridge.activeCase.id?(lawCaseBridge.activeCase.metadata.caseNumber||lawCaseBridge.activeCase.id):app.trx("افتح قضية مصرحًا بها أولًا","Open a permitted case first")}
   ComboBox {id:evidence;objectName:"governmentAnalysisEvidence";Layout.fillWidth:true;model:lawCaseBridge.activeCase.evidence||[];textRole:"name"}
   ComboBox {id:parser;objectName:"governmentAnalysisParser";Layout.fillWidth:true;model:["Classic PCAP — summary v1","JPEG EXIF — summary v1"]}
   ActionButton {objectName:"governmentAnalyzeEvidence";text:app.trx("تحليل وحفظ نتيجة موثقة","Analyze and retain provenance");primary:true;enabled:lawCaseBridge.permissions.analyze&&!!lawCaseBridge.activeCase.id&&evidence.currentIndex>=0;onClicked:page.result=lawCaseBridge.analyzeEvidence(evidence.model[evidence.currentIndex].id,parser.currentIndex===0?"classic-pcap":"jpeg-exif")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.danger;textFormat:Text.PlainText;text:lawCaseBridge.error;visible:text!==""}
   GridLayout {visible:!!page.result.reportJson;Layout.fillWidth:true;columns:width>850?3:2;columnSpacing:12;rowSpacing:12
    Repeater {model:page.summary.packetCount!==undefined?[{ar:"عدد الحزم",en:"Packets",value:page.summary.packetCount},{ar:"البيانات الملتقطة بالبايت",en:"Captured bytes",value:page.summary.capturedBytes},{ar:"حزم خارج ترتيب الوقت",en:"Out-of-order timestamps",value:page.summary.outOfOrderTimestamps}]:[{ar:"بيانات EXIF موجودة",en:"EXIF present",value:page.summary.exifPresent},{ar:"حقول البيانات الوصفية",en:"Metadata fields",value:Object.keys(page.summary.fields||{}).length},{ar:"توثيق أصل البيانات",en:"Metadata authenticity",value:page.app.trx("غير مثبت","Not established")}]
     delegate:Panel {required property var modelData;Layout.fillWidth:true;title:page.app.ar?modelData.ar:modelData.en;iconName:"document_text"
      Label {Layout.fillWidth:true;color:page.app.design.text;font.pixelSize:24;font.bold:true;text:String(modelData.value===undefined?"—":modelData.value);textFormat:Text.PlainText}
     }
    }
   }
   Label {visible:!!page.result.reportJson;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("نتيجة محدودة النطاق؛ لا تثبت أصل الملف أو توقيتًا خارجيًا، ولم يتم فك تشفير المحتوى.","Scoped summary; original-file authenticity and external time are not established. No content decryption was performed.")}
   TextArea {visible:!!page.result.sourceSha256;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;text:"SHA-256: "+(page.result.sourceSha256||"")+"\n"+app.trx("بصمة التقرير: ","Report digest: ")+(page.result.reportSha256||"")}
   ActionButton {visible:!!page.result.reportJson;text:page.showTechnical?app.trx("إخفاء التفاصيل التقنية","Hide technical details"):app.trx("عرض التفاصيل التقنية","Show technical details");onClicked:page.showTechnical=!page.showTechnical}
   TextArea {objectName:"governmentAnalysisResult";visible:page.showTechnical;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;text:page.result.reportJson?JSON.stringify(JSON.parse(page.result.reportJson),null,2):""}
  }
  Panel {objectName:"governmentImportPanel";Layout.fillWidth:true;title:app.trx("استيراد أدلة مصرح بها","Authorized evidence import");iconName:"document_lock"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("استورد المحتوى من دليل مسجل في القضية: نسخة مؤقتة منفصلة، فحص SHA-256 قبل المعالجة وبعدها، ومحلل معزول بحد 4 MiB. يدعم نص UTF-8 وJSON وXML وCSV وSQLite المستقل وسجل Android بصيغة threadtime وفهرسة ADB v5 غير المشفر وغير المضغوط. لا يفك تشفير النسخ أو يستخلص هاتفًا مقفلًا.","Import from registered case evidence: a separate temporary copy, SHA-256 before/after processing and an isolated parser, limited to 4 MiB. Supports UTF-8 text, JSON, XML, CSV, standalone SQLite, Android threadtime logs and unencrypted, uncompressed ADB v5 inventories. No encrypted-backup decryption or locked-device acquisition.")}
   ComboBox {id:importEvidence;objectName:"governmentImportEvidence";Layout.fillWidth:true;model:lawCaseBridge.activeCase.evidence||[];textRole:"name";enabled:!lawCaseBridge.importBusy}
   ComboBox {id:importAdapter;objectName:"governmentImportAdapter";Layout.fillWidth:true;enabled:!lawCaseBridge.importBusy;textRole:app.ar?"ar":"en";model:[{id:"utf8-document",ar:"مستند نصي UTF-8",en:"UTF-8 text document"},{id:"json",ar:"JSON — حقول وقيم",en:"JSON — fields and values"},{id:"xml",ar:"XML — دون DTD أو كيانات خارجية",en:"XML — no DTD or external entities"},{id:"csv",ar:"CSV — خلايا مرقمة",en:"CSV — numbered cells"},{id:"sqlite",ar:"SQLite مستقل — عينة صفوف محدودة",en:"Standalone SQLite — bounded row sample"},{id:"android-log",ar:"سجل Android مصدّر — threadtime",en:"Exported Android log — threadtime"},{id:"adb-backup-v5",ar:"نسخة ADB v5 — فهرسة دون استخراج",en:"ADB v5 backup — inventory without extraction"}]}
   TextField {id:importPurpose;objectName:"governmentImportPurpose";Layout.fillWidth:true;maximumLength:512;enabled:!lawCaseBridge.importBusy;placeholderText:app.trx("غرض الاستيراد ضمن نطاق القضية","Import purpose within the recorded case scope")}
   CheckBox {id:importConsent;objectName:"governmentImportConsent";Layout.fillWidth:true;enabled:!lawCaseBridge.importBusy;text:app.trx("أقر بالتفويض وإنشاء نسخة عمل مؤقتة غير مشفرة","Confirm authority and creation of a temporary plaintext working copy");contentItem:Label {text:importConsent.text;wrapMode:Text.WordWrap;color:app.design.text;leftPadding:importConsent.mirrored?0:importConsent.indicator.width+importConsent.spacing;rightPadding:importConsent.mirrored?importConsent.indicator.width+importConsent.spacing:0;verticalAlignment:Text.AlignVCenter}}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تُزال النسخة بعد انتهاء العملية؛ الإزالة ليست محوًا جنائيًا مضمونًا، وقد تبقى آثار بعد تعطل النظام. لا تُحوّل الطوابع الزمنية أو تُستنتج وقائع غير مثبتة. تُحفظ النتائج والإخفاقات ضمن سجل القضية المحمي.","The copy is removed on completion; removal is not guaranteed forensic erasure and a system crash may leave traces. Timestamps are not inferred or normalized. Results and parser rejections are retained in the protected case audit.")}
   Flow {Layout.fillWidth:true;spacing:8
    ActionButton {objectName:"governmentImportStart";text:app.trx("استيراد وتوثيق النتائج","Import and retain findings");primary:true;enabled:!lawCaseBridge.importBusy&&lawCaseBridge.permissions.analyze&&!!lawCaseBridge.activeCase.id&&importEvidence.currentIndex>=0&&importPurpose.text.trim()!==""&&importConsent.checked;onClicked:{page.savedImport=({});lawCaseBridge.importEvidence(importEvidence.model[importEvidence.currentIndex].id,importAdapter.model[importAdapter.currentIndex].id,importPurpose.text,importConsent.checked);importConsent.checked=false}}
    ActionButton {objectName:"governmentImportCancel";text:app.trx("إلغاء الاستيراد","Cancel import");enabled:lawCaseBridge.importBusy;onClicked:lawCaseBridge.cancelImport()}
   }
   BusyIndicator {objectName:"governmentImportBusy";visible:lawCaseBridge.importBusy;running:visible;Layout.alignment:Qt.AlignHCenter}
   Label {objectName:"governmentImportStatus";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.importReport.status==="rejected"?app.design.danger:app.design.success;text:lawCaseBridge.importBusy?app.trx("يجري تحليل نسخة العمل في عملية منفصلة…","Parsing the working copy in a separate process…"):page.importReport.status==="imported"?app.trx("حُفظت نتيجة الاستيراد وبصمتها","Import result and digest retained"):page.importReport.status==="rejected"?app.trx("رُفض الإدخال بأمان وسُجل الإخفاق: ","Input safely rejected and failure recorded: ")+(page.importReport.errorCode||""):""}
   Label {visible:!!page.imported.reportJson;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.muted;text:"SHA-256: "+(page.imported.sourceSha256||"")+"\n"+app.trx("بصمة التقرير: ","Report SHA-256: ")+(page.imported.reportSha256||"")+"\n"+app.trx("ملاحظات محفوظة: ","Retained findings: ")+(page.importReport.findings||[]).length+(page.importReport.findingsTruncated?app.trx(" — عينة محدودة؛ راجع الحدود"," — bounded sample; review limits"):"")}
   Label {objectName:"governmentBackupScope";visible:!!page.importReport.backup;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("فهرس نسخة ADB v5 فقط: ملفات وبصمات ومسارات؛ دون فك محتوى التطبيقات أو استخراج الملفات. غير مشفرة وغير مضغوطة، بحد 4096 عضوًا و128 نتيجة محفوظة. وقت تعديل الملف غير موثّق ولا يُعد وقت وقوع حدث. الأعضاء المفحوصة: ","ADB v5 inventory only: members, hashes and paths; no app decoding or extraction. Unencrypted and uncompressed, up to 4,096 members and 128 retained findings. Modification time is untrusted and does not establish event time. Members scanned: ")+((page.importReport.backup||{}).membersScanned||0)}
   Repeater {model:(page.importReport.findings||[]).slice(0,12);delegate:Panel {required property var modelData;Layout.fillWidth:true;title:modelData.kind==="adb-backup-member"?page.app.trx("عضو في النسخة الاحتياطية","Backup member"):modelData.kind;iconName:"document_text"
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.muted;text:modelData.reference}
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.text;text:modelData.value+(modelData.valueTruncated?page.app.trx(" [قيمة مختصرة]"," [value shortened]"):"")}
   }}
   ActionButton {visible:!!page.imported.reportJson;text:app.trx("عرض جميع النتائج والمصدر","Show all findings and provenance");onClicked:page.showTechnical=!page.showTechnical}
   TextArea {objectName:"governmentImportResult";visible:page.showTechnical&&!!page.imported.reportJson;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;text:page.imported.reportJson?JSON.stringify(page.importReport,null,2):""}
  }
  Panel {objectName:"governmentTimelinePanel";Layout.fillWidth:true;title:app.trx("الخط الزمني للقيم المرصودة","Observed timestamp timeline");iconName:"history"
   Label {objectName:"governmentTimelineScope";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;topPadding:2;bottomPadding:4;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("عرض للقيم الزمنية من النتائج المحفوظة. تُحوّل المناطق الزمنية المذكورة إلى UTC، وتبقى الأوقات الناقصة غير محسومة. لا يثبت هذا العرض صحة الوقت أو وقوع الحدث.","Combines timestamp literals from retained import reports. Explicit offsets become UTC; incomplete times remain unresolved. Source time authenticity, real-world events and relationships between people are not established.")}
   TextField {id:timelineQuery;objectName:"governmentTimelineQuery";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("بحث حرفي في الوقت أو المصدر أو المرجع","Literal search in time, source or reference");enabled:!lawCaseBridge.timelineBusy}
   Flow {Layout.fillWidth:true;spacing:8
    ActionButton {objectName:"governmentTimelineBuild";text:app.trx("بناء الخط الزمني","Build timeline");primary:true;enabled:lawCaseBridge.permissions.analyze&&!!lawCaseBridge.activeCase.id&&!lawCaseBridge.timelineBusy&&!lawCaseBridge.artifactsBusy;onClicked:lawCaseBridge.buildTimeline(timelineQuery.text)}
    ActionButton {objectName:"governmentTimelineCancel";text:app.trx("إلغاء","Cancel");enabled:lawCaseBridge.timelineBusy;onClicked:lawCaseBridge.cancelTimeline()}
   }
   BusyIndicator {running:lawCaseBridge.timelineBusy;visible:running;Layout.alignment:Qt.AlignHCenter}
   Label {objectName:"governmentTimelineSummary";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.muted;text:page.timeline.schema?app.trx("أوقات UTC: ","UTC times: ")+page.timeline.normalizedCount+app.trx(" | أوقات غير محسومة: "," | Unresolved: ")+page.timeline.unresolvedCount+app.trx(" | قيم إضافية لم تُعرض: "," | Additional matching values omitted: ")+page.timeline.omittedMatchingCandidates+app.trx(" | تقارير استيراد مختصرة: "," | Truncated import reports: ")+page.timeline.truncatedImportReports:app.trx("ابنِ العرض بعد استيراد دليل. لا توجد نتائج معروضة.","Build the view after importing evidence. No results displayed.")}
   Repeater {id:timelineEntries;model:page.timelineRows.slice(page.timelinePage*12,(page.timelinePage+1)*12);delegate:Panel {required property var modelData;property alias timeExplanation:timelineReasonLabel.text;Layout.fillWidth:true;title:modelData.utc||page.app.trx("وقت غير محسوم","Unresolved time");iconName:"document_text"
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:modelData.originalTime;LayoutMirroring.enabled:false}
    Label {id:timelineReasonLabel;objectName:"governmentTimelineReason";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:page.timeReason(modelData.reason)}
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:modelData.sourceName+" | "+page.displayIdentifier(modelData.reference)+"\n"+page.app.trx("الدليل: ","Evidence: ")+page.displayIdentifier(modelData.evidenceId)+"\n"+page.app.trx("التحليل: ","Analysis: ")+page.displayIdentifier(modelData.analysisId)+"\nSHA-256: "+modelData.sourceSha256+"\nReport SHA-256: "+modelData.reportSha256;LayoutMirroring.enabled:false}
   }}
   Flow {Layout.fillWidth:true;spacing:8;visible:page.timelineRows.length>12
    ActionButton {objectName:"governmentTimelinePrevious";text:app.trx("السابق","Previous");enabled:page.timelinePage>0;onClicked:page.timelinePage--}
    ActionButton {objectName:"governmentTimelineNext";text:app.trx("التالي","Next");enabled:(page.timelinePage+1)*12<page.timelineRows.length;onClicked:page.timelinePage++}
   }
  }
  GovernmentArtifactPanel {app:page.app;Layout.fillWidth:true}
  Repeater {model:lawCaseBridge.activeCase.analyses||[];delegate:Panel {required property var modelData;Layout.fillWidth:true;title:modelData.parser;iconName:"document_text"
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:modelData.atUtc+"\nSHA-256: "+modelData.sourceSha256+"\nReport SHA-256: "+modelData.reportSha256}
   ActionButton {visible:modelData.parser.indexOf("authorized-import/")===0;text:page.app.trx("فتح نتيجة الاستيراد المحفوظة","Open retained import result");onClicked:{page.savedImport=modelData;page.showTechnical=true}}
  }}
 }
}
