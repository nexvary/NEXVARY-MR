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
 contentWidth:availableWidth;clip:true
 Connections {target:lawCaseBridge;function onChanged(){page.result=({});page.showTechnical=false}}
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
  Repeater {model:lawCaseBridge.activeCase.analyses||[];delegate:Panel {required property var modelData;Layout.fillWidth:true;title:modelData.parser;iconName:"document_text"
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:modelData.atUtc+"\nSHA-256: "+modelData.sourceSha256+"\nReport SHA-256: "+modelData.reportSha256}
  }}
 }
}
