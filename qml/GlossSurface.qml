import QtQuick
import QtQuick.Controls
Rectangle {
 id:surface
 readonly property var themeWindow:ApplicationWindow.window
 property bool glossy:!themeWindow||themeWindow.darkTheme
 property real reflection:0.04
 color:"#0A0A0A";radius:12;clip:true
 gradient:glossy?glass:null
 Gradient {id:glass;GradientStop {position:0;color:Qt.lighter(surface.color,1.22)}GradientStop {position:0.48;color:surface.color}GradientStop {position:1;color:Qt.darker(surface.color,1.05)}}
 Rectangle {visible:surface.glossy;anchors.left:parent.left;anchors.right:parent.right;anchors.top:parent.top;anchors.margins:1;height:1;color:Qt.rgba(0.85,0.92,1,surface.reflection)}
}
