import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 property alias currentTab:tabs.currentIndex
 property var app
 spacing:10
 TabBar {id:tabs;visible:app.activeSection!==1&&app.activeSection!==9;objectName:"deviceTabs";Layout.fillWidth:true
  TabButton {text:app.trx("الأجهزة والمعلومات","Devices and information")}
  TabButton {text:app.trx("خدمات الشركات · ADB / Fastboot","OEM services · ADB / Fastboot")}
  TabButton {text:app.trx("كتالوج الأجهزة","Device reference catalog")}
 }
 StackLayout {currentIndex:tabs.currentIndex;Layout.fillWidth:true;Layout.fillHeight:true
  DevicesPage {app:parent.parent.app}
  DeviceServicesPage {app:parent.parent.app}
  SupportedDevicesPage {app:parent.parent.app}
 }
}
