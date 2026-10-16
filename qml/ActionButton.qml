import QtQuick
import QtQuick.Controls
Button {
 id: control
 property bool primary: false
 implicitHeight: 38
 implicitWidth: Math.max(100, contentItem.implicitWidth + 28)
 hoverEnabled: true
 font.pixelSize: 13
 contentItem: Text { text: control.text; font: control.font; color: control.enabled ? "#ecf3ff" : "#63758a"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
 background: Rectangle {
  radius: 6
  color: !control.enabled ? "#101e2c" : control.down ? "#154c9a" : control.primary ? (control.hovered ? "#2778f1" : "#1261df") : (control.hovered ? "#18344d" : "#102638")
  border.color: control.activeFocus ? "#77bfff" : control.primary ? "#3986ff" : "#254257"
 }
}
