import QtQuick
import QtQuick.Controls
ComboBox {
 id:control
 DesignTokens {id:fallbackDesign}
 readonly property var design:ApplicationWindow.window?ApplicationWindow.window.design:fallbackDesign
 implicitHeight:40
 contentItem: Text {
  text:control.displayText; font:control.font;color:control.design.text;textFormat:Text.PlainText
  verticalAlignment:Text.AlignVCenter;elide:Text.ElideRight
  leftPadding:control.mirrored?28:12;rightPadding:control.mirrored?12:28
 }
 background:Rectangle {implicitWidth:180;implicitHeight:40;color:control.design.surface;radius:6;border.color:control.design.text;border.width:control.activeFocus?2:1}
 delegate:ItemDelegate {
  required property int index
  width:control.width
  text:control.textAt(index)
  highlighted:control.highlightedIndex===index
  contentItem:Text {text:parent.text;color:control.design.text;font:control.font;textFormat:Text.PlainText;elide:Text.ElideRight;verticalAlignment:Text.AlignVCenter}
  background:Rectangle {color:parent.highlighted?control.design.raised:control.design.surface}
 }
 popup:Popup {
  y:control.height; width:control.width
  implicitHeight:Math.min(contentItem.implicitHeight+12,320)
  padding:6
  contentItem:ListView {clip:true;implicitHeight:contentHeight;model:control.popup.visible?control.delegateModel:null;currentIndex:control.highlightedIndex;ScrollBar.vertical:ScrollBar {}}
  background:Rectangle {color:control.design.surface;border.color:control.design.text;radius:6}
 }
}
