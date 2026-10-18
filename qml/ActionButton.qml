import QtQuick
import QtQuick.Controls
Button {
 id: control
 readonly property var themeWindow:ApplicationWindow.window
 function tone(value){return themeWindow?themeWindow.tone(value):value}
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
  color: control.enabled ? control.tone("#edf5ff") : control.tone("#8192a5")
  horizontalAlignment: Text.AlignHCenter
  verticalAlignment: Text.AlignVCenter
  elide: Text.ElideRight
 }
 background: GlossSurface {
  reflection:control.enabled?(control.down?0.05:control.hovered?0.20:0.12):0.035
  radius: 9
  color: !control.enabled ? control.tone("#162434") : control.down ? control.tone("#124b94") : control.primary ? (control.hovered ? control.tone("#307ef3") : control.tone("#1767dc")) : (control.hovered ? control.tone("#1d3b55") : control.tone("#10293e"))
  border.width: control.activeFocus ? 2 : 1
  border.color: control.activeFocus ? control.tone("#a6d9ff") : control.primary ? control.tone("#519cff") : control.tone("#31516a")
  Rectangle {
   visible: control.primary && control.enabled
   anchors.left: parent.left
   anchors.right: parent.right
   anchors.top: parent.top
   anchors.margins: 2
   height: 1
   color: control.tone("#75b7ff")
   opacity: 0.6
  }
 }
}
