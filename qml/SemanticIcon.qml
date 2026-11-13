import QtQuick
import QtQuick.Window
Image {
 id:icon
 property string name:""
 property url iconSource:name!==""?"qrc:/assets/icons-professional/"+name+".svg":""
 readonly property string semanticId:name||String(iconSource).split("/").pop().replace(/\.svg$/,"")
 property string accessibleLabel:semanticId.replace(/_/g," ")
 property bool decorative:true
 property bool usingFallback:false
 implicitWidth:28;implicitHeight:28
 fillMode:Image.PreserveAspectFit
 asynchronous:false
 opacity:enabled?1:0.4
 mirror:false
 source:usingFallback?"qrc:/assets/icons-professional/question_circle.svg":iconSource
 sourceSize:Qt.size(Math.max(32,Math.ceil(width*Screen.devicePixelRatio)),Math.max(32,Math.ceil(height*Screen.devicePixelRatio)))
 onIconSourceChanged:usingFallback=false
 onStatusChanged:if(status===Image.Error&&!usingFallback)usingFallback=true
 Accessible.role:Accessible.Graphic
 Accessible.name:accessibleLabel
 Accessible.ignored:decorative
}
