import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
 id:command
 property bool primary:false
 property string iconName:""
 implicitHeight:30;implicitWidth:Math.max(72,contentItem.implicitWidth+18)
 hoverEnabled:true;font.pixelSize:12;focusPolicy:Qt.StrongFocus
 Accessible.name:text
 contentItem:RowLayout {
  spacing:6
  Image {visible:command.iconName!=="";source:visible?"qrc:/assets/icons-metallic/"+command.iconName+".svg":"";Layout.preferredWidth:22;Layout.preferredHeight:22;sourceSize:Qt.size(24,24);opacity:command.enabled?1:0.4}
  Label {text:command.text;font:command.font;color:command.enabled?"#E5EDF3":"#718897";horizontalAlignment:Text.AlignHCenter;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
 }
 background:Rectangle {radius:2;color:command.down?"#115875":command.primary?"#0C435C":command.hovered?"#0E3044":"#0A2231";border.color:command.activeFocus?"#50D4F7":command.primary?"#23B9E7":"#24556D";border.width:command.activeFocus?2:1}
}
