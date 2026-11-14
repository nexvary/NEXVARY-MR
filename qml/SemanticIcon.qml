import QtQuick
import QtQuick.Window
Item {
 id:icon
 property string name:""
 property url iconSource:name!==""?"qrc:/assets/icons-professional/"+name+".svg":""
 readonly property string semanticId:name||String(iconSource).split("/").pop().replace(/\.svg$/,"")
 property string accessibleLabel:semanticId.replace(/_/g," ")
 property bool decorative:true
 property bool usingFallback:false
 readonly property int status:artwork.status
 readonly property url source:artwork.source
 readonly property size sourceSize:artwork.sourceSize
 property alias fillMode:artwork.fillMode
 implicitWidth:28;implicitHeight:28
 opacity:enabled?1:0.4
 onIconSourceChanged:usingFallback=false
 Accessible.role:Accessible.Graphic
 Accessible.name:accessibleLabel
 Accessible.ignored:decorative
 Image {
  id:artwork
  anchors.fill:parent
  fillMode:Image.PreserveAspectFit
  asynchronous:false
  mirror:false
  source:icon.usingFallback?"qrc:/assets/icons-professional/question_circle.svg":icon.iconSource
  sourceSize:Qt.size(Math.max(32,Math.ceil(icon.width*Screen.devicePixelRatio)),Math.max(32,Math.ceil(icon.height*Screen.devicePixelRatio)))
  onStatusChanged:if(status===Image.Error&&!icon.usingFallback)icon.usingFallback=true
  Accessible.ignored:true
 }
}
