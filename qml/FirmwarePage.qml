import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:14
  Panel {
   title:app.trx("فحص توافق ملف النظام · دون تفليش","Firmware compatibility preflight · no flashing");Layout.fillWidth:true;Layout.fillHeight:true
   Label {text:app.trx("فحص SHA-256 وهوية الموديل المعلنة فقط. لا يُثبت أصالة الملف أو توقيعه؛ يجب التحقق من مصدر الشركة الرسمي.","Checks SHA-256 and declared model identity only. It does not authenticate firmware or its signature; verify official OEM provenance.");color:"#f0bc75";wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:16}
   Label {text:app.trx("موديل الجهاز المقروء: ","Detected device model: ")+(toolBridge.deviceInfo["ro.product.model"]||"—");color:"#9fb8ce";textFormat:Text.PlainText}
   RowLayout {TextField{id:manifest;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("ملف بيانات التوافق JSON","Compatibility manifest JSON");Layout.fillWidth:true} ActionButton{text:app.trx("اختيار البيان","Choose manifest");onClicked:manifestDialog.open()}}
   RowLayout {TextField{id:firmware;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("ملف النظام المحلي","Local firmware package");Layout.fillWidth:true} ActionButton{text:app.trx("اختيار ملف النظام","Choose firmware");onClicked:firmwareDialog.open()}}
   RowLayout {
    ActionButton {text:app.trx("بدء التحقق المحلي","Verify local files");primary:true;enabled:!firmwareBridge.busy&&manifest.text!==""&&firmware.text!=="";onClicked:firmwareBridge.inspect(manifest.text,firmware.text,toolBridge.deviceInfo["ro.product.brand"]||"",toolBridge.deviceInfo["ro.product.model"]||"")}
    ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
    Label {text:firmwareBridge.progress+"%";color:"#93b8d7"}
   }
   ProgressBar {Layout.fillWidth:true;from:0;to:100;value:firmwareBridge.progress;visible:firmwareBridge.busy}
   TextArea {Layout.fillWidth:true;Layout.fillHeight:true;readOnly:true;wrapMode:TextEdit.Wrap;text:firmwareBridge.status;color:firmwareBridge.compatible?"#4dd9a8":"#a9bfd2";background:Rectangle{color:"#0b2030";radius:6}}
   Label {text:app.trx("عمليات الاستعادة والتفليش غير مفعلة في هذا الإصدار. احتفظ بنسخة احتياطية واستخدم أدوات الشركة الرسمية وفقًا للموديل.","Recovery and flashing are not enabled in this build. Back up data and use official OEM tools for the exact device model.");color:"#89a3bb";wrapMode:Text.WordWrap;Layout.fillWidth:true}
  }
 }
 Connections {target:toolBridge;function onDeviceInfoChanged(){firmwareBridge.invalidate()} function onSelectedSerialChanged(){firmwareBridge.invalidate()}}
 FileDialog {id:manifestDialog;nameFilters:["JSON (*.json)"];onAccepted:manifest.text=selectedFile.toString()}
 FileDialog {id:firmwareDialog;onAccepted:firmware.text=selectedFile.toString()}
}
