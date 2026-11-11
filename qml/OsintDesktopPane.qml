import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:pane
 property bool wallpaper:false
 property string title:""
 property string subtitle:""
 property string iconName:""
 default property alias body:body.data
 color:"#EF08121B";border.color:"#24556D";border.width:1;radius:1
 OsintBackdrop {anchors.fill:parent;visible:pane.wallpaper;dimming:0.24}
 implicitHeight:layout.implicitHeight+2
 ColumnLayout {
  id:layout;anchors.fill:parent;anchors.margins:1;spacing:0
  Rectangle {
   Layout.fillWidth:true;implicitHeight:30;color:"#0D2A3D";visible:pane.title!==""
   RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8;spacing:6
    Image {visible:pane.iconName!=="";source:visible?"qrc:/assets/icons-metallic/"+pane.iconName+".svg":"";Layout.preferredWidth:20;Layout.preferredHeight:20;sourceSize:Qt.size(32,32)}
    Label {text:pane.title;Layout.fillWidth:true;color:"#E5EDF3";font.pixelSize:12;font.bold:true;elide:Text.ElideRight}
   }
  }
  ColumnLayout {id:body;Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:8;spacing:7}
 }
}
