import QtQuick
Item {
 id:icon
 property int kind:0
 property color stroke:"#91bfff"
 implicitWidth:32;implicitHeight:32
 readonly property var files:["home","phone","gauge","board","arrow_square_down","clipboard_task","database","settings","history","document_lock"]
 Image {
  anchors.fill:parent
  source:"qrc:/assets/icons/"+icon.files[Math.max(0,Math.min(icon.files.length-1,icon.kind))]+".svg"
  fillMode:Image.PreserveAspectFit
  sourceSize.width:40;sourceSize.height:40
 }
}
