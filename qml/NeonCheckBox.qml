import QtQuick
import QtQuick.Controls
CheckBox {
 id: control
 spacing: 10
 implicitHeight: Math.max(44, contentItem.implicitHeight + 16)
 indicator: Rectangle {
  implicitWidth: 24; implicitHeight: 24
  x: control.mirrored ? control.width-width-control.rightPadding : control.leftPadding
  y: (control.height-height)/2
  radius: 4; color: control.checked ? "#39ff14" : "#0a0a0a"
  border.color: "#39ff14"; border.width: control.activeFocus ? 3 : 2
  Text {anchors.centerIn:parent;text:control.checked ? "✓" : "";color:"#071007";font.pixelSize:20;font.bold:true}
 }
 contentItem: Label {
  text:control.text; font:control.font; textFormat:Text.PlainText;wrapMode:Text.WordWrap
  color:control.palette.windowText
  leftPadding:control.mirrored ? 0 : 34
  rightPadding:control.mirrored ? 34 : 0
  verticalAlignment:Text.AlignVCenter
 }
}
