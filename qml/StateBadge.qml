import QtQuick
import QtQuick.Controls
Rectangle {
 id:badge
 property string text:""
 property color tint:"#269AFF"
 implicitWidth:caption.implicitWidth+18;implicitHeight:26;radius:6
 color:Qt.rgba(tint.r,tint.g,tint.b,0.10)
 border.color:Qt.rgba(tint.r,tint.g,tint.b,0.30)
 Label {id:caption;anchors.centerIn:parent;text:badge.text;color:badge.tint;font.pixelSize:11;font.bold:true;textFormat:Text.PlainText}
}
