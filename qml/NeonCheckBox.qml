import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
CheckBox {
 id: control
 Layout.fillWidth:true
 Layout.minimumWidth:0
 DesignTokens {id:fallbackDesign}
 readonly property var design:ApplicationWindow.window?ApplicationWindow.window.design:fallbackDesign
 spacing: 10
 implicitHeight: Math.max(44, implicitContentHeight + topPadding + bottomPadding)
 contentItem:Label {
  text:control.text;font:control.font;color:control.enabled?control.design.text:control.design.muted
  wrapMode:Text.WordWrap;verticalAlignment:Text.AlignVCenter
  leftPadding:control.mirrored?0:control.indicator.width+control.spacing
  rightPadding:control.mirrored?control.indicator.width+control.spacing:0
 }
 indicator: Rectangle {
  objectName:"selectionIndicator";implicitWidth:28;implicitHeight:28
  x: control.mirrored ? control.width-width-control.rightPadding : control.leftPadding
  y: (control.height-height)/2
  radius: 4; color: control.checked ? "#A855F7" : "#261632"
  border.color: control.activeFocus ? "#E9C5FF" : "#BB86FC"; border.width: control.activeFocus ? 3 : 2
  SemanticIcon {objectName:"neonCheckmark";anchors.centerIn:parent;width:18;height:18;iconSource:"qrc:/assets/icons-professional/check_mark.svg";visible:control.checked;fillMode:Image.PreserveAspectFit}
 }
}
