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
 color:"#EF08121B";border.color:"#C0C0C0";border.width:1.5;radius:1
 OsintBackdrop {anchors.fill:parent;visible:pane.wallpaper;dimming:0.24}
 implicitHeight:header.height+body.implicitHeight+18
 Rectangle {
  id:header;anchors.top:parent.top;anchors.left:parent.left;anchors.right:parent.right;anchors.margins:1
  height:pane.title!==""?Math.max(42,headerRow.implicitHeight+12):0;color:"#0D2A3D";visible:pane.title!==""
  RowLayout {id:headerRow;anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8;spacing:6
   SemanticIcon {visible:pane.iconName!=="";iconSource:visible?"qrc:/assets/icons-professional/"+pane.iconName+".svg":"";Layout.preferredWidth:20;Layout.preferredHeight:20;}
   Label {text:pane.title;Layout.fillWidth:true;color:"#E5EDF3";font.pixelSize:14;font.bold:true;elide:Text.ElideRight}
  }
 }
 ColumnLayout {
  id:body;anchors.top:header.bottom;anchors.topMargin:8;anchors.left:parent.left;anchors.right:parent.right;anchors.leftMargin:9;anchors.rightMargin:9;spacing:7
 }
}
