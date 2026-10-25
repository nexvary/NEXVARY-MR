import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:side
 property var app
 property bool collapsed:app.width<1200
 function itemAt(index){return navigation.itemAt(index)}
 color:app.design.surface;radius:12;border.color:app.design.border
 ColumnLayout {
  anchors.fill:parent;anchors.margins:8;spacing:8
  ActionButton {objectName:"workspaceMode";text:side.app.governmentMode?side.app.trx("الصيانة","Repair"):side.app.trx("الحكومة","Government");Layout.fillWidth:true;implicitWidth:48;implicitHeight:32;onClicked:side.app.setWorkspace(!side.app.governmentMode)}
  ActionButton {objectName:"collapseSidebar";text:side.collapsed?side.app.trx("فتح","Expand"):side.app.trx("طي القائمة","Collapse navigation");Layout.fillWidth:true;implicitWidth:48;implicitHeight:32;onClicked:side.collapsed=!side.collapsed}
  ScrollView {
   id:scroll;objectName:"sidebarScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
   ColumnLayout {
    width:scroll.availableWidth;spacing:4
    Repeater {
     id:navigation;model:side.app.sections
     delegate:NavigationItem {
      required property var modelData
      required property int index
      objectName:"sectionNav"+index;app:side.app;Layout.fillWidth:true
      text:side.app.ar?modelData.ar:modelData.en;iconName:modelData.icon;selected:side.app.activeSection===index;compact:side.collapsed
      onClicked:side.app.navigateSection(index)
     }
    }
   }
  }
  ColumnLayout {
   visible:!side.collapsed;Layout.fillWidth:true;spacing:4
   StateBadge {text:toolBridge.laboratoryBuild?"LABORATORY":"READ-ONLY";tint:toolBridge.laboratoryBuild?side.app.design.danger:side.app.design.success}
   Label {text:side.app.trx("محطة محلية • إصدار تطوير","Local workstation • Development");color:side.app.design.muted;font.pixelSize:10;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  }
 }
}
