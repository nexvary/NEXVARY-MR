import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:hero
 property var app
 objectName:"mrHomeHero"
 implicitHeight:Math.min(550,Math.max(370,width*0.45))
 color:"#040507";radius:12;border.color:"#24556D";clip:true
 Image {objectName:"mrHomeArtwork";anchors.fill:parent;source:"qrc:/assets/branding/mr-home-background.png";fillMode:Image.PreserveAspectCrop;smooth:true;asynchronous:true}
 Rectangle {anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;height:104
  gradient:Gradient {GradientStop {position:0;color:"#00040507"}GradientStop {position:1;color:"#E6040507"}}
 }
 Rectangle {
  anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.margins:14;height:66;radius:9;color:"#D9081B2A";border.color:"#39758F"
  RowLayout {anchors.fill:parent;anchors.margins:10;spacing:10
   Image {source:"qrc:/assets/icons-professional/home.svg";Layout.preferredWidth:38;Layout.preferredHeight:38;sourceSize:Qt.size(48,48)}
   ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:3
    Label {text:hero.app.trx("مرحبًا بك في NEXVARY MR", "Welcome to NEXVARY MR");font.pixelSize:17;color:"#E5EDF3";font.weight:Font.Light;Layout.fillWidth:true;elide:Text.ElideRight}
    Label {text:hero.app.trx("الصيانة • البحث • التحليل", "REPAIR • RESEARCH • ANALYSIS");font.pixelSize:10;color:"#9EB4C1";font.letterSpacing:1;Layout.fillWidth:true;elide:Text.ElideRight}
   }
   OsintDesktopCommand {objectName:"dashboardDevices";iconName:"phone";text:hero.app.trx("الأجهزة", "Devices");onClicked:hero.app.navigatePrimary(1)}
   OsintDesktopCommand {objectName:"homeOpenOsint";iconName:"search_visual";primary:true;text:"OSINT";onClicked:hero.app.navigatePrimary(3)}
   OsintDesktopCommand {visible:hero.width>850;iconName:"document_text";text:hero.app.trx("التقارير", "Reports");onClicked:{hero.app.setWorkspace(false);hero.app.navigateSection(8)}}
  }
 }
}
