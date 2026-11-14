import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:inspector
 objectName:"deviceInspector"
 property var app
 readonly property string selectedModel:toolBridge.deviceInfo["serial"]===toolBridge.selectedSerial?toolBridge.deviceInfo["ro.product.model"]||"":""
 function copyValue(editor){
  if(!editor.text)return
  editor.selectAll();editor.copy()
  inspector.app.notice(true,inspector.app.trx("تم النسخ إلى الحافظة","Copied to clipboard"))
 }
 title:app.trx("سياق الجهاز","Device context");iconName:"phone"
 ScrollView {
  id:scroll;objectName:"deviceScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:12
   SemanticIcon {iconSource:"qrc:/assets/icons-professional/phone.svg";Layout.preferredWidth:56;Layout.preferredHeight:56;fillMode:Image.PreserveAspectFit}
   Label {text:inspector.app.trx("موديل الهاتف","Phone model");color:inspector.app.design.muted}
   RowLayout {Layout.fillWidth:true;spacing:6
    EntryField {id:modelValue;objectName:"selectedDeviceModel";Layout.fillWidth:true;Layout.minimumWidth:0;readOnly:true;requiredInput:false;text:inspector.selectedModel;placeholderText:inspector.app.trx("لم تُقرأ الهوية","Identity unread");color:inspector.app.design.text;Accessible.name:inspector.app.trx("موديل الهاتف، قابل للنسخ","Phone model, selectable for copying")}
    ActionButton {objectName:"copyDeviceModel";text:inspector.app.trx("نسخ","Copy");implicitWidth:64;Layout.preferredWidth:64;Layout.maximumWidth:64;enabled:modelValue.length>0;Accessible.name:inspector.app.trx("نسخ الموديل","Copy model");ToolTip.visible:hovered;ToolTip.text:Accessible.name;onClicked:inspector.copyValue(modelValue)}
   }
   Label {text:inspector.app.trx("الرقم التسلسلي","Serial number");color:inspector.app.design.muted}
   RowLayout {Layout.fillWidth:true;spacing:6
    EntryField {id:serialValue;objectName:"selectedDeviceSerial";Layout.fillWidth:true;Layout.minimumWidth:0;readOnly:true;requiredInput:false;text:toolBridge.selectedSerial||"";placeholderText:inspector.app.trx("اختر جهازًا","Select a target");color:inspector.app.design.text;LayoutMirroring.enabled:false;horizontalAlignment:TextInput.AlignLeft;Accessible.name:inspector.app.trx("الرقم التسلسلي، قابل للنسخ","Device serial, selectable for copying")}
    ActionButton {objectName:"copyDeviceSerial";text:inspector.app.trx("نسخ","Copy");implicitWidth:64;Layout.preferredWidth:64;Layout.maximumWidth:64;enabled:serialValue.length>0;Accessible.name:inspector.app.trx("نسخ الرقم التسلسلي","Copy serial number");ToolTip.visible:hovered;ToolTip.text:Accessible.name;onClicked:inspector.copyValue(serialValue)}
   }
   Label {text:toolBridge.deviceInfo["ro.build.fingerprint"]||inspector.app.trx("لا توجد معلومات بناء مقروءة بعد","No build identity read yet");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:inspector.app.design.muted;font.pixelSize:14;textFormat:Text.PlainText}
   ActionButton {text:inspector.app.trx("مدير الأجهزة","Device Manager");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(1)}
   ActionButton {text:inspector.app.trx("أمر صيانة جديد","Repair orders");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(7)}
   ActionButton {objectName:"quickBackup";text:inspector.app.trx("النسخ الاحتياطي","Backup & restore");Layout.fillWidth:true;onClicked:inspector.app.navigateSection(10)}
   Label {text:inspector.app.trx("تُراجع الموافقات داخل بطاقة العملية. اختيار الجهاز لا يمنح تصريحًا لتغيير بياناته.","Consent is reviewed within each operation. Target selection does not authorize data changes.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:inspector.app.design.muted;font.pixelSize:14}
  }
 }
}
