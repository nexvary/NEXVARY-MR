import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"driverCatalogPage"
 contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;Layout.preferredHeight:170;title:app.trx("تعريفات الشركات الرسمية","Official OEM drivers")
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("هذه مصادر رسمية، وليست حزم تعريفات مرفقة أو مثبتة. طابق رقم الموديل ونسخة Windows ووضع الاتصال. لا توجد حزمة عامة مضمونة لكل أجهزة Qualcomm أو MediaTek أو Unisoc، ولا يتم تعطيل توقيع التعريفات.","Official sources, not bundled or installed driver packages. Match exact model, Windows version and USB mode. No universal package is guaranteed for Qualcomm, MediaTek or Unisoc; driver-signing enforcement is not disabled.")}
   ActionButton {objectName:"inspectOemDrivers";text:app.trx("فحص تعريفات USB الحالية في Windows","Inspect current Windows USB drivers");enabled:!toolBridge.busy;onClicked:toolBridge.inspectDrivers()}
  }
  Repeater {model:serviceCatalog.drivers
   delegate:Panel {required property var modelData;Layout.fillWidth:true;Layout.preferredHeight:155;title:modelData.brand
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:modelData.kind;color:app.tone("#c6daeb")}
    Label {Layout.fillWidth:true;elide:Text.ElideMiddle;textFormat:Text.PlainText;text:modelData.url;color:app.tone("#8eb6d6");font.pixelSize:12}
    ActionButton {text:app.trx("فتح المصدر الرسمي","Open official source");onClicked:Qt.openUrlExternally(modelData.url)}
   }
  }
  Panel {Layout.fillWidth:true;Layout.preferredHeight:280;title:app.trx("نتيجة فحص التعريفات","Driver inspection output")
   ScrollView {Layout.fillWidth:true;Layout.fillHeight:true;TextArea {readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;textFormat:TextEdit.PlainText;text:toolBridge.driverInfo;color:app.tone("#c6daeb");background:null}}
  }
 }
}
