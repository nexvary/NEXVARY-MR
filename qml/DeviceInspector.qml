import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:inspector
 property var app
 title:app.trx("سياق الجهاز","Device context");iconName:"phone"
 ScrollView {
  id:scroll;objectName:"deviceScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:12
   Image {source:"qrc:/assets/icons-professional/phone.svg";Layout.preferredWidth:56;Layout.preferredHeight:56;fillMode:Image.PreserveAspectFit}
   Label {text:toolBridge.deviceInfo["ro.product.model"]||inspector.app.trx("لم تُقرأ الهوية","Identity unread");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:inspector.app.design.text;font.bold:true;textFormat:Text.PlainText}
   // A real QML TextField owns the clipboard operation, so this works on Windows
   // without depending on an undocumented Qt.application.clipboard property.
   TextField {id:serialClipboard;visible:false;readOnly:true}
   TextField {id:modelClipboard;visible:false;readOnly:true}
   function copyValue(field,value){
    if(!value)return
    field.text=String(value)
    field.selectAll()
    field.copy()
    field.deselect()
    inspector.app.notice(true,inspector.app.trx("تم النسخ إلى الحافظة","Copied to clipboard"))
   }
   RowLayout {Layout.fillWidth:true;spacing:6
    TextField {id:modelValue;objectName:"selectedDeviceModel";Layout.fillWidth:true;readOnly:true;selectByMouse:true;text:toolBridge.deviceInfo["ro.product.model"]||"";placeholderText:inspector.app.trx("لم تُقرأ الهوية","Identity unread")}
    ActionButton {objectName:"copyDeviceModel";text:inspector.app.trx("نسخ الموديل","Copy model");enabled:modelValue.text.length>0;onClicked:copyValue(modelClipboard,modelValue.text)}
   }
   RowLayout {Layout.fillWidth:true;spacing:6
    TextField {id:serialValue;objectName:"selectedDeviceSerial";Layout.fillWidth:true;readOnly:true;selectByMouse:true;text:toolBridge.selectedSerial||"";placeholderText:inspector.app.trx("اختر جهازًا من مدير الأجهزة","Select a target in Device Manager")}
    ActionButton {objectName:"copyDeviceSerial";text:inspector.app.trx("نسخ الرقم","Copy serial");enabled:serialValue.text.length>0;onClicked:copyValue(serialClipboard,serialValue.text)}
   }
   Label {text:toolBridge.deviceInfo["ro.build.fingerprint"]||inspector.app.trx("لا توجد معلومات بناء مقروءة بعد","No build identity read yet");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:inspector.app.design.muted;font.pixelSize:11;textFormat:Text.PlainText}
   ActionButton {text:inspector.app.trx("مدير الأجهزة","Device Manager");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(1)}
   ActionButton {text:inspector.app.trx("أمر صيانة جديد","Repair orders");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(7)}
   ActionButton {objectName:"quickBackup";text:inspector.app.trx("النسخ الاحتياطي","Backup & restore");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(10)}
   Label {text:inspector.app.trx("تُراجع الموافقات داخل بطاقة العملية. اختيار الجهاز لا يمنح تصريحًا لتغيير بياناته.","Consent is reviewed within each operation. Target selection does not authorize data changes.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:inspector.app.design.muted;font.pixelSize:11}
  }
 }
}
