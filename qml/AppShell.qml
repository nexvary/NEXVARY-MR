import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:shell
 property var app
 default property alias workspace:workspace.data
 function sectionButton(index){return sidebar.itemAt(index)}
 ColumnLayout {
  anchors.fill:parent;spacing:0
  TopBar {app:shell.app;Layout.fillWidth:true;Layout.preferredHeight:shell.app.design.topBarHeight}
  RowLayout {
   Layout.fillWidth:true;Layout.fillHeight:true;Layout.margins:12;spacing:12
   Sidebar {id:sidebar;objectName:"workstationSidebar";app:shell.app;Layout.preferredWidth:collapsed?shell.app.design.collapsedSidebar:shell.app.design.expandedSidebar;Layout.fillHeight:true}
   DeviceInspector {visible:shell.app.showDevicePanel;app:shell.app;Layout.preferredWidth:208;Layout.fillHeight:true}
   ColumnLayout {
    Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0;spacing:8
    RowLayout {
     Layout.fillWidth:true
     ActionButton {objectName:"backButton";text:shell.app.trx("رجوع","Back");implicitHeight:30;implicitWidth:72;enabled:shell.app.pageHistory.length>0;onClicked:shell.app.goBack()}
     Label {text:shell.app.trx("مساحة العمل / ","WORKSPACE / ")+shell.app.pageTitle;Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideRight;color:shell.app.design.muted;font.pixelSize:11}
     Label {text:"0.4.0 · "+shell.app.trx("تطوير","DEVELOPMENT");color:shell.app.design.muted;font.pixelSize:10}
    }
    Item {id:workspace;objectName:"workstationWorkspace";Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0;clip:true}
    OperationProgress {visible:!shell.app.governmentMode||toolBridge.busy||firmwareBridge.busy||recoveryBridge.busy||securityBridge.busy;app:shell.app;Layout.fillWidth:true}
   }
  }
  Label {text:shell.app.trx("NEXVARY  •  محلي  •  أجهزة مصرح بها فقط","NEXVARY  •  LOCAL WORKSTATION  •  AUTHORIZED DEVICES ONLY");Layout.fillWidth:true;Layout.preferredHeight:24;horizontalAlignment:Text.AlignHCenter;color:shell.app.design.muted;font.pixelSize:10}
 }
}
