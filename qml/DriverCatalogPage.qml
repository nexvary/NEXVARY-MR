import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"driverCatalogPage"
 contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  UsbDoctorPanel {app:page.app;Layout.fillWidth:true}
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;title:app.trx("تعريفات الشركات الرسمية","Official OEM drivers")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("هذه مصادر رسمية، وليست حزم تعريفات مرفقة أو مثبتة. طابق رقم الموديل ونسخة Windows ووضع الاتصال. لا توجد حزمة عامة مضمونة لكل أجهزة Qualcomm أو MediaTek أو Unisoc، ولا يتم تعطيل توقيع التعريفات.","Official sources, not bundled or installed driver packages. Match exact model, Windows version and USB mode. No universal package is guaranteed for Qualcomm, MediaTek or Unisoc; driver-signing enforcement is not disabled.")}
   ActionButton {objectName:"inspectOemDrivers";text:app.trx("فحص تعريفات USB الحالية في Windows","Inspect current Windows USB drivers");enabled:!toolBridge.busy;onClicked:toolBridge.inspectDrivers()}
  }
  Panel {Layout.fillWidth:true;accent:"blue";title:app.trx("مطابقة USB للهاتف المحدد","Selected phone USB correlation")
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.text;text:toolBridge.selectedSerial||app.trx("اختر الهاتف من الأجهزة والاتصال أولًا","Select the phone in Devices & USB first")}
   ActionButton {objectName:"correlateSelectedUsb";text:app.trx("قراءة تعريفات الهاتف المحدد","Read selected phone drivers");enabled:!toolBridge.busy&&toolBridge.selectedSerial!=="";onClicked:toolBridge.inspectSelectedDriver()}
   Label {objectName:"usbCorrelationState";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.text;text:{const s=toolBridge.driverBinding.state||"not-read";const labels={"serial-correlated":["تمت مطابقة الرقم التسلسلي وشجرة USB","USB serial and parent tree correlated"],"no-match":["لا توجد مطابقة؛ قد يختلف معرّف Windows أو يكون الاتصال لاسلكيًا","No match; Windows identity may differ or the connection may be wireless"],"ambiguous":["المعرّف مكرر؛ لا يمكن اختيار جهاز بثقة","Duplicate serial; cannot select a device confidently"],"unsupported-serial":["صيغة المعرّف لا تسمح بمطابقة USB الموثوقة","Serial format cannot be reliably correlated to USB"],"windows-only":["هذه الوظيفة متاحة على Windows فقط","This function requires Windows"],"unavailable":["لم تتوفر بيانات صالحة للمطابقة","No valid inventory available"],"not-read":["لم تُقرأ المطابقة بعد","Correlation not read yet"]};return labels[s]?(app.ar?labels[s][0]:labels[s][1]):s}}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8bfd4");text:app.trx("تُعرض التعريفات المثبتة للعقد المرتبطة بالرقم التسلسلي. لا تثبت المطابقة وحدها أي واجهة يستخدمها ADB أو Fastboot، ولا تعني أن التعريف مناسب للتفليش. حالة التوقيع كما يبلغ عنها Windows.","Shows installed drivers for nodes associated with the serial. Correlation alone does not prove which interface ADB or Fastboot uses or suitability for flashing. Signature state is reported by Windows.")}
   Repeater {model:toolBridge.driverBinding.nodes||[]
    delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#c6daeb");text:modelData.name+"\n"+modelData.instanceId+"\n"+(modelData.provider||"?")+" · "+(modelData.version||"?")+" · "+(modelData.infName||"?")+"\n"+app.trx("الخدمة: ","Service: ")+(modelData.service||"?")+" · "+app.trx("توقيع: ","Signed: ")+(modelData.signed===undefined?app.trx("غير معروف","Unknown"):modelData.signed?app.trx("نعم","Yes"):app.trx("لا","No"))+" · "+app.trx("رمز المشكلة: ","Problem code: ")+(modelData.problemCode===undefined?"?":modelData.problemCode)}
   }
   Label {visible:!!toolBridge.driverBinding.observedAtUtc;Layout.fillWidth:true;color:app.tone("#a8bfd4");text:"UTC: "+(toolBridge.driverBinding.observedAtUtc||"");textFormat:Text.PlainText}
  }
  Repeater {model:serviceCatalog.drivers
   delegate:Panel {required property var modelData;Layout.fillWidth:true;title:modelData.brand
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:modelData.kind;color:app.tone("#c6daeb")}
    Label {Layout.fillWidth:true;elide:Text.ElideMiddle;textFormat:Text.PlainText;text:modelData.url;color:app.tone("#8eb6d6");font.pixelSize:12}
    ActionButton {text:app.trx("فتح المصدر الرسمي","Open official source");onClicked:app.openExternal(modelData.url)}
   }
  }
  Panel {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.preferredHeight:280;title:app.trx("نتيجة فحص التعريفات","Driver inspection output")
   ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;textFormat:TextEdit.PlainText;text:toolBridge.driverInfo;color:app.tone("#c6daeb");background:null}}
  }
 }
}
