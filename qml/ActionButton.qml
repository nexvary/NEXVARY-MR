import QtQuick
import QtQuick.Controls
Button {
 id: control
 property bool primary: false
 implicitHeight: 42
 implicitWidth: Math.max(112, contentItem.implicitWidth + 30)
 hoverEnabled: true
 font.pixelSize: 13
 focusPolicy: Qt.StrongFocus
 Accessible.name: text
 contentItem: Text {
  text: control.text
  font: control.font
  color: control.enabled ? "#edf5ff" : "#8192a5"
  horizontalAlignment: Text.AlignHCenter
  verticalAlignment: Text.AlignVCenter
  elide: Text.ElideRight
 }
 background: Rectangle {
  radius: 9
  color: !control.enabled ? "#162434" : control.down ? "#124b94" : control.primary ? (control.hovered ? "#307ef3" : "#1767dc") : (control.hovered ? "#1d3b55" : "#10293e")
  border.width: control.activeFocus ? 2 : 1
  border.color: control.activeFocus ? "#a6d9ff" : control.primary ? "#519cff" : "#31516a"
  Rectangle {
   visible: control.primary && control.enabled
   anchors.left: parent.left
   anchors.right: parent.right
   anchors.top: parent.top
   anchors.margins: 2
   height: 1
   color: "#75b7ff"
   opacity: 0.6
  }
 }
}
