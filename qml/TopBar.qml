import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:bar
 property var app
 readonly property bool working:toolBridge.busy||firmwareBridge.busy||recoveryBridge.busy||securityBridge.busy
 readonly property var targetConnections:toolBridge.devices.filter(function(device){return device.serial===toolBridge.selectedSerial})
 readonly property string connectionState:!toolBridge.selectedSerial?"none":targetConnections.length!==1?"missing":targetConnections[0].transport==="fastboot"?"fastboot":targetConnections[0].state
 readonly property string connectionTitle:connectionState==="device"?app.trx("ADB مفوض","ADB authorized"):connectionState==="fastboot"?"Fastboot":connectionState==="unauthorized"?app.trx("ADB غير مفوض","ADB unauthorized"):connectionState==="offline"?app.trx("ADB غير متاح","ADB offline"):connectionState==="none"?app.trx("لم يحدد هدف","No target"):app.trx("اتصال غائب أو ملتبس","Missing / ambiguous")
 color:app.design.surface
 implicitHeight:72
 RowLayout {
  anchors.fill:parent;anchors.margins:12;spacing:12
  Image {source:"qrc:/assets/nexvary-mr.png";Layout.preferredWidth:38;Layout.preferredHeight:38;fillMode:Image.PreserveAspectFit}
  ColumnLayout {visible:bar.app.width>=1100;spacing:2;Layout.preferredWidth:170;Label {text:"NEXVARY MR";font.pixelSize:19;font.bold:true;font.letterSpacing:1;color:bar.app.design.text}Label {text:bar.app.governmentMode?bar.app.trx("الإصدار الحكومي","GOVERNMENT EDITION"):bar.app.trx("الصيانة والتحليل الجنائي","REPAIR & FORENSICS");font.pixelSize:10;color:bar.app.design.muted}}
  Rectangle {Layout.preferredWidth:1;Layout.preferredHeight:30;color:bar.app.design.border}
  ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:3
   Label {text:bar.app.pageTitle;font.pixelSize:16;font.bold:true;color:bar.app.design.text;Layout.fillWidth:true;elide:Text.ElideRight}
   Label {text:toolBridge.selectedSerial?(toolBridge.deviceInfo["ro.product.model"]||bar.app.trx("جهاز محدد","Selected device"))+" · "+(toolBridge.deviceInfo["ro.build.version.release"]?"Android "+toolBridge.deviceInfo["ro.build.version.release"]:bar.app.trx("المعلومات غير مقروءة","Identity unread")):bar.app.trx("لم يُحدد هاتف • ابدأ بالاكتشاف","No selected device • Start with discovery");font.pixelSize:11;color:bar.app.design.muted;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
  }
  StateBadge {objectName:"connectionBadge";text:bar.working?bar.app.trx("عملية جارية","Running"):bar.connectionTitle;tint:bar.working||bar.connectionState==="fastboot"?bar.app.design.accent:bar.connectionState==="device"?bar.app.design.success:bar.connectionState==="none"?bar.app.design.silver:bar.app.design.danger}
  ActionButton {objectName:"devicePanelButton";text:bar.app.trx("الجهاز","Device");implicitWidth:70;onClicked:bar.app.showDevicePanel=!bar.app.showDevicePanel}
  ActionButton {objectName:"themeButton";text:bar.app.darkTheme?bar.app.trx("فاتح","Light"):bar.app.trx("داكن","Dark");implicitWidth:62;onClicked:bar.app.darkTheme=!bar.app.darkTheme}
  ActionButton {objectName:"languageButton";text:bar.app.ar?"EN":"عربي";implicitWidth:48;onClicked:bar.app.ar=!bar.app.ar}
  ToolButton {objectName:"topSettings";Accessible.name:bar.app.trx("الإعدادات","Settings");icon.source:"qrc:/assets/icons/settings.svg";icon.color:"transparent";onClicked:bar.app.navigate(7);ToolTip.visible:hovered;ToolTip.text:Accessible.name}
  ToolButton {objectName:"topHelp";Accessible.name:bar.app.trx("المساعدة","Help");icon.source:"qrc:/assets/icons/question_circle.svg";icon.color:"transparent";onClicked:bar.app.navigate(11);ToolTip.visible:hovered;ToolTip.text:Accessible.name}
  ActionButton {objectName:"exitButton";text:bar.app.trx("خروج","Exit");implicitWidth:62;onClicked:bar.app.close()}
 }
}
