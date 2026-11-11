import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:pane
 property string title:""
 property string subtitle:""
 property string iconName:""
 default property alias body:body.data
 color:"#FFFFFF";border.color:"#B6BDC5";border.width:1;radius:1
 implicitHeight:layout.implicitHeight+2
 ColumnLayout {
  id:layout;anchors.fill:parent;anchors.margins:1;spacing:0
  Rectangle {
   Layout.fillWidth:true;implicitHeight:28;color:"#E9ECEF";visible:pane.title!==""
   RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8;spacing:6
    Image {visible:pane.iconName!=="";source:visible?"qrc:/assets/icons/"+pane.iconName+".svg":"";Layout.preferredWidth:16;Layout.preferredHeight:16;sourceSize:Qt.size(20,20)}
    Label {text:pane.title;Layout.fillWidth:true;color:"#27333D";font.pixelSize:12;font.bold:true;elide:Text.ElideRight}
   }
  }
  ColumnLayout {id:body;Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:8;spacing:7}
 }
}
