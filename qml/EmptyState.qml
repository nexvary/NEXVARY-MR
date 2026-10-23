import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id:state
 property var app
 property string title:""
 property string description:""
 property string iconName:"phone"
 spacing:12
 Image {source:"qrc:/assets/icons/"+state.iconName+".svg";Layout.preferredWidth:40;Layout.preferredHeight:40;fillMode:Image.PreserveAspectFit;sourceSize.width:48;sourceSize.height:48}
 Label {text:state.title;Layout.fillWidth:true;wrapMode:Text.WordWrap;font.bold:true;font.pixelSize:16;color:state.app.design.text}
 Label {text:state.description;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:state.app.design.muted;font.pixelSize:13}
}
