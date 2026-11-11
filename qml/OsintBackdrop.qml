import QtQuick
Rectangle {
 color:"#040507"
 property real dimming:0.66
 Image {objectName:"osintBackgroundImage";anchors.fill:parent;source:"qrc:/assets/branding/mr-osint-background.png";fillMode:Image.PreserveAspectCrop;smooth:true;asynchronous:true}
 Rectangle {anchors.fill:parent;color:"#040507";opacity:parent.dimming}
}
