import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent; spacing:12
  RowLayout {
   Layout.fillWidth:true
   ActionButton {objectName:"scanAdb";text:app.trx("فحص ADB","Scan ADB");primary:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
   ActionButton {text:"Fastboot";enabled:!toolBridge.busy;onClicked:toolBridge.scanFastboot()}
   ActionButton {text:app.trx("فحص التعريفات","Inspect drivers");enabled:!toolBridge.busy;onClicked:toolBridge.inspectDrivers()}
   Switch {text:app.trx("تحديث تلقائي","Auto refresh");checked:toolBridge.autoRefresh;onToggled:toolBridge.autoRefresh=checked}
   Item {Layout.fillWidth:true}
   BusyIndicator {running:toolBridge.busy;Layout.preferredWidth:32;Layout.preferredHeight:32}
   ActionButton {text:app.trx("إلغاء","Cancel");enabled:toolBridge.busy;onClicked:toolBridge.cancel()}
  }
  RowLayout {
   Layout.fillWidth:true; Layout.fillHeight:true;spacing:12
   Panel {
    title:app.trx("الأجهزة المكتشفة","Discovered devices");Layout.preferredWidth:290;Layout.fillHeight:true
    Label {visible:toolBridge.devices.length===0;text:app.trx("لم يتم اكتشاف جهاز. صِل هاتفًا بكابل بيانات، ثم اسمح بتصحيح USB من الهاتف.","No device discovered. Connect a data cable, then authorize USB debugging on the phone.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:"#91a6bc"}
    ListView {
     Layout.fillWidth:true;Layout.fillHeight:true;model:toolBridge.devices;spacing:8;clip:true
     delegate:ItemDelegate {
      required property var modelData
      width:ListView.view.width;height:76
      onClicked:toolBridge.selectDevice(modelData.serial)
      background:Rectangle {radius:6;color:toolBridge.selectedSerial===modelData.serial?"#143c64":"#0b2131";border.color:"#234a63"}
      contentItem:Column {spacing:6;Label {text:modelData.model||modelData.serial;color:"#edf4ff";font.bold:true;textFormat:Text.PlainText} Label{text:modelData.serial+" • "+modelData.state;color:modelData.state==="device"?"#48dea4":"#f6bb67";font.pixelSize:12;textFormat:Text.PlainText}}
     }
    }
    ActionButton {text:app.trx("عرض شاشة الهاتف · scrcpy","Mirror device · scrcpy");Layout.fillWidth:true;enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:toolBridge.startMirror()}
   }
   ColumnLayout {
    Layout.fillWidth:true;Layout.fillHeight:true;spacing:12
    Panel {
     title:app.trx("خصائص الجهاز · قراءة فقط","Device properties · read only");Layout.fillWidth:true;Layout.fillHeight:true
     RowLayout {ActionButton {text:app.trx("قراءة المعلومات","Read properties");enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:toolBridge.readDeviceInfo()} ActionButton{text:app.trx("قراءة آخر 200 سجل","Read last 200 logs");enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:toolBridge.readLogcat()}}
     ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;text:JSON.stringify(toolBridge.deviceInfo,null,2);color:"#c7deef";font.family:"Consolas";background:null;wrapMode:TextEdit.Wrap}}
    }
    Panel {
     title:app.trx("السجلات والتعريفات","Logs and drivers");Layout.fillWidth:true;Layout.fillHeight:true
     ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;text:toolBridge.logcat||toolBridge.driverInfo||app.trx("نتائج الفحص ستظهر هنا. قد تحتوي السجلات على بيانات شخصية؛ شاركها بعد المراجعة.","Read-only results appear here. Logs may contain personal data; review before sharing.");color:"#a8bfd4";font.family:"Consolas";wrapMode:TextEdit.Wrap;background:null}}
    }
   }
  }
 }
}
