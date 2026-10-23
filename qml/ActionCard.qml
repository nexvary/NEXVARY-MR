import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:card
 property var app
 property string description:""
 property string statusText:""
 property string actionText:""
 property string actionName:""
 property string unavailableReason:""
 property bool available:true
 property bool working:false
 property bool primary:false
 signal triggered()
 stateText:statusText
 implicitHeight:Math.max(196,content.implicitHeight+86)
 ColumnLayout {
  id:content;Layout.fillWidth:true;Layout.fillHeight:true;spacing:12
  Label {text:card.description;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:card.app.design.muted;textFormat:Text.PlainText;font.pixelSize:13}
  Label {visible:!card.available&&text!=="";text:card.unavailableReason;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:card.app.design.silver;textFormat:Text.PlainText;font.pixelSize:12}
  Item {Layout.fillHeight:true;Layout.minimumHeight:0}
  RowLayout {
   Layout.fillWidth:true
   BusyIndicator {visible:card.working;running:card.working;Layout.preferredWidth:24;Layout.preferredHeight:24}
   ActionButton {objectName:card.actionName;text:card.actionText;primary:card.primary;enabled:card.available&&!card.working;Layout.fillWidth:true;onClicked:card.triggered()}
  }
 }
}
