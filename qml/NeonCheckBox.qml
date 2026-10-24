import QtQuick
import QtQuick.Controls
CheckBox {
 id: control
 DesignTokens {id:fallbackDesign}
 readonly property var design:ApplicationWindow.window?ApplicationWindow.window.design:fallbackDesign
 spacing: 10
 implicitHeight: Math.max(40, implicitContentHeight + topPadding + bottomPadding)
 indicator: Rectangle {
  implicitWidth: 24; implicitHeight: 24
  x: control.mirrored ? control.width-width-control.rightPadding : control.leftPadding
  y: (control.height-height)/2
  radius: 4; color: control.checked ? control.design.success : control.design.surface
  border.color: control.activeFocus ? control.design.accent : control.checked ? control.design.success : control.design.silver; border.width: control.activeFocus ? 3 : 2
  Image {objectName:"neonCheckmark";anchors.fill:parent;anchors.margins:2;source:"qrc:/assets/icons/check_mark.svg";visible:control.checked;fillMode:Image.PreserveAspectFit}
 }
}
