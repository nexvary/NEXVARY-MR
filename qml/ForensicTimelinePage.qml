import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 property var events:recoveryBridge.timeline.filter(function(e){return (e.atUtc+" "+e.event+" "+e.detail+" "+e.operator+" "+e.evidenceId).toLowerCase().indexOf(query.text.toLowerCase())>=0})
 ColumnLayout {
  anchors.fill:parent;spacing:12
  Label {text:app.trx("أوقات جمع الأدلة وعمليات القضية بصيغة UTC. لا يمثل هذا نشاط التطبيقات أو تاريخ إنشاء الملفات على الهاتف.","UTC evidence collection and case-operation times. This is not application activity or phone file-creation history.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3")}
  TextField {id:query;objectName:"forensicTimelineSearch";Layout.fillWidth:true;placeholderText:app.trx("بحث في الخط الزمني","Search case timeline")}
  ListView {
   objectName:"forensicTimelineList";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:page.events;ScrollBar.vertical:ScrollBar {}
   delegate:Rectangle {
    required property var modelData
    width:ListView.view.width;height:eventColumn.implicitHeight+24;radius:8;color:app.tone("#0d2538");border.color:app.tone("#284b65")
    ColumnLayout {id:eventColumn;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:12;spacing:5
     Label {text:modelData.atUtc+" · "+modelData.event;textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.tone("#63d7b1");font.bold:true}
     Label {text:modelData.detail;textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.tone("#d3e8fa")}
     Label {text:modelData.operator+" · "+modelData.evidenceId;textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.tone("#9bbbd3");font.pixelSize:12}
    }
   }
  }
  RowLayout {Layout.fillWidth:true;CheckBox {id:consent;objectName:"timelinePlaintext";text:app.trx("أوافق على تصدير غير مشفر","Authorize plaintext export")}ActionButton {objectName:"exportForensicTimeline";text:app.trx("تصدير الخط الزمني JSON","Export timeline JSON");enabled:recoveryBridge.active&&!recoveryBridge.busy&&consent.checked;onClicked:output.open()}Item {Layout.fillWidth:true}}
 }
 FileDialog {id:output;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:app.notice(recoveryBridge.exportTimeline(selectedFile.toString(),consent.checked),app.trx("تم التصدير","Exported"),recoveryBridge.error)}
}
