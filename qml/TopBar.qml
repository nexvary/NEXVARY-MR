import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:bar
 property var app
 readonly property bool working:lawCaseBridge.importBusy||toolBridge.foregroundBusy||firmwareBridge.busy||recoveryBridge.busy||securityBridge.busy
 readonly property var targetConnections:toolBridge.devices.filter(function(device){return device.serial===toolBridge.selectedSerial})
 readonly property string connectionState:!toolBridge.selectedSerial?"none":targetConnections.length!==1?"missing":targetConnections[0].transport==="fastboot"?"fastboot":targetConnections[0].state
 readonly property string connectionTitle:connectionState==="device"?app.trx("ADB مفوض","ADB authorized"):connectionState==="fastboot"?"Fastboot":connectionState==="unauthorized"?app.trx("ADB غير مفوض","ADB unauthorized"):connectionState==="offline"?app.trx("ADB غير متاح","ADB offline"):connectionState==="none"?app.trx("لم يحدد هدف","No target"):app.trx("اتصال غائب أو ملتبس","Missing / ambiguous")
 color:app.design.surface
 implicitHeight:96
 RowLayout {
  anchors.fill:parent;anchors.leftMargin:12;anchors.rightMargin:12;anchors.topMargin:8;anchors.bottomMargin:8;spacing:12
  Image {objectName:"nexvaryEyeSeal";source:"qrc:/assets/branding/nexvary-eye-seal.svg";sourceSize:Qt.size(512,512);Layout.minimumWidth:80;Layout.maximumWidth:80;Layout.preferredWidth:80;Layout.minimumHeight:80;Layout.maximumHeight:80;Layout.preferredHeight:80;fillMode:Image.PreserveAspectFit;smooth:true;mipmap:true;Accessible.name:bar.app.trx("ختم نيكسفاري — عين الاستخبارات","Nexvary intelligence eye seal")}
  ColumnLayout {visible:bar.app.width>=1000;spacing:3;Layout.preferredWidth:260
   Label {text:"NEXVARY MR";font.pixelSize:25;font.bold:true;color:bar.app.design.text}
   Label {objectName:"brandSubtitle";text:bar.app.trx("منظومة العمليات والاستخبارات","Operations & Intelligence System");font.pixelSize:14;color:bar.app.design.silver;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  }
  Rectangle {Layout.preferredWidth:1;Layout.preferredHeight:30;color:bar.app.design.border}
  ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:3
   Label {text:bar.app.pageTitle;font.pixelSize:16;font.bold:true;color:bar.app.design.text;Layout.fillWidth:true;elide:Text.ElideRight}
   Label {objectName:"governmentPolicyState";text:(identityBridge.directWork?bar.app.trx("عمل مباشر · ","Direct work · "):operationPolicy.offline?bar.app.trx("منع افتراضي · ","Default deny · "):bar.app.trx("استثناء مؤقت نشط · ","Temporary exception · "))+(toolBridge.selectedSerial?(toolBridge.deviceInfo["ro.product.model"]||bar.app.trx("جهاز محدد","Selected device"))+" · "+(toolBridge.deviceInfo["ro.build.version.release"]?"Android "+toolBridge.deviceInfo["ro.build.version.release"]:bar.app.trx("المعلومات غير مقروءة","Identity unread")):bar.app.trx("لم يُحدد هاتف • ابدأ بالاكتشاف","No selected device • Start with discovery"));font.pixelSize:14;color:operationPolicy.offline?bar.app.design.muted:bar.app.design.accent;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
  }
  StateBadge {objectName:"connectionBadge";text:bar.working?bar.app.trx("عملية جارية","Running"):bar.connectionTitle;tint:bar.working||bar.connectionState==="fastboot"?bar.app.design.accent:bar.connectionState==="device"?bar.app.design.success:bar.connectionState==="none"?bar.app.design.silver:bar.app.design.danger}
  ActionButton {objectName:"devicePanelButton";text:bar.app.trx("الجهاز","Device");implicitWidth:70;onClicked:bar.app.showDevicePanel=!bar.app.showDevicePanel}
  ActionButton {objectName:"themeButton";text:bar.app.darkTheme?bar.app.trx("فاتح","Light"):bar.app.trx("داكن","Dark");implicitWidth:62;onClicked:bar.app.darkTheme=!bar.app.darkTheme}
  ActionButton {objectName:"languageButton";text:bar.app.ar?"EN":"عربي";implicitWidth:48;onClicked:bar.app.ar=!bar.app.ar}
  ToolButton {objectName:"topSettings";Accessible.name:bar.app.trx("الإعدادات","Settings");contentItem:SemanticIcon {name:"settings";implicitWidth:22;implicitHeight:22}
   onClicked:bar.app.navigate(7);ToolTip.visible:hovered;ToolTip.text:Accessible.name}
  ToolButton {objectName:"topHelp";Accessible.name:bar.app.trx("المساعدة","Help");contentItem:SemanticIcon {name:"question_circle";implicitWidth:22;implicitHeight:22}
   onClicked:bar.app.navigate(11);ToolTip.visible:hovered;ToolTip.text:Accessible.name}
  ActionButton {objectName:"exitButton";text:bar.app.trx("خروج","Exit");implicitWidth:62;onClicked:bar.app.close()}
 }
}
