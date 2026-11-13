import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:nav
 objectName:"mrPrimaryNavigation"
 property var app
 readonly property int sectionCount:app.sections.length
 readonly property bool overflowing:sectionsRow.implicitWidth>viewport.width+1
 function itemAt(index){return sectionsRepeater.itemAt(app.primarySectionOrder.indexOf(index))}
 function reveal(index){
  const item=itemAt(index);if(!item)return;
  if(item.x<viewport.contentX)viewport.contentX=item.x;
  else if(item.x+item.width>viewport.contentX+viewport.width)viewport.contentX=item.x+item.width-viewport.width;
 }
 implicitHeight:96;color:"#E608121B"
 LayoutMirroring.enabled:false
 LayoutMirroring.childrenInherit:true
 Rectangle {anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;height:1;color:"#24556D"}
 Flickable {
  id:viewport;objectName:"primaryNavigationViewport"
  anchors.fill:parent;anchors.leftMargin:10;anchors.rightMargin:10;anchors.topMargin:4;anchors.bottomMargin:4
  contentWidth:sectionsRow.implicitWidth;contentHeight:height;clip:true
  boundsBehavior:Flickable.StopAtBounds;flickableDirection:Flickable.HorizontalFlick
  ScrollBar.horizontal:ScrollBar {policy:nav.overflowing?ScrollBar.AlwaysOn:ScrollBar.AlwaysOff}
  Row {
   id:sectionsRow;spacing:4;height:viewport.height
   layoutDirection:nav.app.ar?Qt.RightToLeft:Qt.LeftToRight
   Repeater {
    id:sectionsRepeater;model:nav.app.primarySectionOrder
    delegate:Button {
     id:section
     required property int modelData
     readonly property int sectionIndex:modelData
     readonly property var entry:nav.app.sections[sectionIndex]||nav.app.sections[0]
     readonly property bool selected:nav.app.activeSection===sectionIndex
     objectName:"primaryNav"+sectionIndex
     width:Math.max(104,Math.min(118,(viewport.width-4*(nav.sectionCount-1))/nav.sectionCount))
     height:viewport.height-(nav.overflowing?7:0)
     padding:3;hoverEnabled:true;focusPolicy:Qt.StrongFocus
     text:nav.app.ar?entry.ar:entry.en
     Accessible.name:text
     ToolTip.visible:hovered;ToolTip.text:text
     contentItem:ColumnLayout {
      spacing:3
      SemanticIcon {objectName:"primaryNavIcon"+section.sectionIndex;iconSource:"qrc:/assets/icons-professional/"+section.entry.icon+".svg";Layout.preferredWidth:44;Layout.preferredHeight:44;Layout.alignment:Qt.AlignHCenter;fillMode:Image.PreserveAspectFit}
      Label {text:section.text;color:section.selected?"#E5EDF3":section.hovered?"#C4E7F5":"#9EB4C1";font.pixelSize:12;font.weight:section.selected?Font.DemiBold:Font.Normal;Layout.fillWidth:true;Layout.preferredHeight:34;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter;wrapMode:Text.WordWrap;maximumLineCount:2;elide:Text.ElideRight}
     }
     background:Item {
      Rectangle {anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;anchors.leftMargin:8;anchors.rightMargin:8;height:section.activeFocus?3:2;color:section.selected||section.activeFocus?"#23B9E7":"#52758A";visible:section.selected||section.hovered||section.activeFocus}
     }
     onClicked:nav.app.navigateSection(sectionIndex)
     onActiveFocusChanged:if(activeFocus)nav.reveal(sectionIndex)
    }
   }
  }
 }
 Connections {target:nav.app;function onActiveSectionChanged(){nav.reveal(nav.app.activeSection)}function onArChanged(){Qt.callLater(function(){nav.reveal(nav.app.activeSection)})}function onGovernmentModeChanged(){Qt.callLater(function(){nav.reveal(nav.app.activeSection)})}}
 Component.onCompleted:Qt.callLater(function(){nav.reveal(nav.app.activeSection)})
}
