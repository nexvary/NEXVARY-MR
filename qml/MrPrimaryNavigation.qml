import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:nav
 property var app
 implicitHeight:46;color:"#E608121B";border.color:"#24556D"
 LayoutMirroring.enabled:app.ar;LayoutMirroring.childrenInherit:true
 RowLayout {
  anchors.fill:parent;anchors.leftMargin:12;anchors.rightMargin:12;spacing:6
  Repeater {
   model:[{ar:"الرئيسية",en:"Home",icon:"home",page:0},{ar:"الأجهزة",en:"Devices",icon:"phone",page:1},{ar:"الصيانة",en:"Repair",icon:"toolbox",page:2},{ar:"الاستخبارات",en:"OSINT",icon:"search_visual",page:14},{ar:"الإعدادات",en:"Settings",icon:"settings",page:7}]
   delegate:OsintDesktopCommand {
    required property var modelData
    required property int index
    objectName:"primaryNav"+index
    iconName:modelData.icon;text:nav.app.ar?modelData.ar:modelData.en
    primary:nav.app.currentPage===modelData.page
    implicitHeight:34
    onClicked:nav.app.navigatePrimary(index)
   }
  }
  Item {Layout.fillWidth:true}
 }
}
