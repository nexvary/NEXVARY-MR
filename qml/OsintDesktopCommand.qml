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
  Image {visible:command.iconName!=="";source:visible?"qrc:/assets/icons/"+command.iconName+".svg":"";Layout.preferredWidth:18;Layout.preferredHeight:18;sourceSize:Qt.size(24,24);opacity:command.enabled?1:0.4}
  Label {text:command.text;font:command.font;color:command.enabled?"#24313C":"#889098";horizontalAlignment:Text.AlignHCenter;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
 }
 background:Rectangle {radius:2;color:command.down?"#C3DDB9":command.primary?"#D9EAD0":command.hovered?"#E7F2E1":"#F5F6F7";border.color:command.activeFocus?"#468D36":command.primary?"#91B782":"#BCC3CB";border.width:command.activeFocus?2:1}
}
