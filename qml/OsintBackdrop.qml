import QtQuick
Rectangle {
 id:backdrop
 color:"#040507"
 property real dimming:0.24
 property bool fullWindow:false
 Image {id:wave;objectName:backdrop.fullWindow?"workstationBackgroundImage":"osintBackgroundImage";anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;height:backdrop.fullWindow?parent.height:width*251/1536;source:"qrc:/assets/branding/mr-network-wave.png";fillMode:backdrop.fullWindow?Image.PreserveAspectCrop:Image.PreserveAspectFit;opacity:backdrop.fullWindow?0.55:1;smooth:true;asynchronous:true}
 Rectangle {anchors.fill:wave
  gradient:Gradient {GradientStop {position:0;color:"#040507"}GradientStop {position:0.22;color:"#60040507"}GradientStop {position:0.55;color:"#00040507"}GradientStop {position:1;color:"#30040507"}}
 }
 Rectangle {anchors.fill:parent;visible:backdrop.fullWindow
  gradient:Gradient {
   GradientStop {position:0;color:"#08040507"}
   GradientStop {position:0.72;color:"#08102616"}
   GradientStop {position:1;color:"#36173A28"}
  }
 }
 Rectangle {anchors.fill:parent;color:"#040507";opacity:backdrop.dimming}
}
