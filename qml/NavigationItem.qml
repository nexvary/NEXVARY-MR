import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
 id:item
 property var app
 property string iconName:"home"
 property bool selected:false
 property bool compact:false
 implicitHeight:40
 hoverEnabled:true
 Accessible.name:text
 ToolTip.visible:hovered&&compact;ToolTip.text:text
 background:Rectangle {
  radius:8;color:item.selected?Qt.rgba(item.app.design.accent.r,item.app.design.accent.g,item.app.design.accent.b,0.15):item.hovered?item.app.design.raised:"transparent"
  border.width:item.selected?1:0;border.color:Qt.rgba(item.app.design.accent.r,item.app.design.accent.g,item.app.design.accent.b,0.45)
 }
 contentItem:RowLayout {
  spacing:10
  Image {source:"qrc:/assets/icons/"+item.iconName+".svg";Layout.preferredWidth:26;Layout.preferredHeight:26;sourceSize.width:32;sourceSize.height:32;fillMode:Image.PreserveAspectFit}
  Label {visible:!item.compact;text:item.text;Layout.fillWidth:true;Layout.minimumWidth:0;color:item.selected?item.app.design.text:item.app.design.muted;font.pixelSize:12;font.bold:item.selected;elide:Text.ElideRight}
 }
}
