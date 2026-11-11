import QtQuick
Rectangle {
 color:"#040507"
 property real dimming:0.24
 Image {id:wave;objectName:"osintBackgroundImage";anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;height:width*251/1536;source:"qrc:/assets/branding/mr-network-wave.png";fillMode:Image.PreserveAspectFit;smooth:true;asynchronous:true}
 Rectangle {anchors.fill:wave
  gradient:Gradient {GradientStop {position:0;color:"#040507"}GradientStop {position:0.22;color:"#60040507"}GradientStop {position:0.55;color:"#00040507"}GradientStop {position:1;color:"#30040507"}}
 }
 Rectangle {anchors.fill:parent;color:"#040507";opacity:parent.dimming}
}
