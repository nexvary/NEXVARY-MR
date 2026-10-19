import QtQuick
import QtQuick.Controls
ComboBox {
 id:control
 implicitHeight:40
 contentItem: Text {
  text:control.displayText; font:control.font;color:"#39ff14";textFormat:Text.PlainText
  verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight
  leftPadding:8;rightPadding:28
 }
 background:Rectangle {implicitWidth:180;implicitHeight:40;color:"#0a0a0a";radius:6;border.color:"#39ff14";border.width:control.activeFocus?2:1}
 delegate:ItemDelegate {
  required property int index
  width:control.width
  text:control.textAt(index)
  highlighted:control.highlightedIndex===index
  contentItem:Text {text:parent.text;color:"#39ff14";font:control.font;textFormat:Text.PlainText;elide:Text.ElideRight;verticalAlignment:Text.AlignVCenter}
  background:Rectangle {color:parent.highlighted?"#17451a":"#0a0a0a"}
 }
 popup:Popup {
  y:control.height; width:control.width
  implicitHeight:Math.min(contentItem.implicitHeight+12,320)
  padding:6
  contentItem:ListView {clip:true;implicitHeight:contentHeight;model:control.popup.visible?control.delegateModel:null;currentIndex:control.highlightedIndex;ScrollBar.vertical:ScrollBar {}}
  background:Rectangle {color:"#0a0a0a";border.color:"#39ff14";radius:6}
 }
}
