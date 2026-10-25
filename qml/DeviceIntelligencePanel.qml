import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:panel
 property bool fieldsReady:false
 Component.onCompleted:fieldsReady=true
 property var app
 readonly property var report:toolBridge.deviceIntelligence
 readonly property var health:report.health||({})
 readonly property var battery:health.battery||({})
 readonly property var storage:health.storage||({})
 readonly property var partitions:health.partitions||({})
 title:"Device Intelligence";iconName:"search_visual";accent:"blue"
 stateText:toolBridge.activeOperation==="properties"?app.trx("تحديث جارٍ · آخر قراءة","Refreshing · last reading"):(report.reportedFieldCount||0)+app.trx(" حقل مرصود"," observed fields")
 function field(id){for(let i=0;i<fields.count;i++){const item=fields.itemAt(i);if(item&&item.modelData.id===id)return item}return null}
 function fieldState(state){return state==="reported"?app.trx("تصريح الجهاز","Device reported"):state==="expired"?app.trx("قراءة قديمة — أعد القراءة","Expired — read again"):state==="missing"?app.trx("غير متاح في القراءة","Not supplied by reading"):app.trx("لم تُقرأ هوية الهدف","Target identity unread")}
 Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("كل قيمة مرتبطة بمصدرها ووقت قراءتها. تصريحات الجهاز قابلة للتعديل ولا تمثل تحققًا تشفيريًا أو اعتماد موديل. القيم الغائبة أو القديمة لا تُملأ من كتالوج الموديلات.","Each value retains its source and collection time. Device declarations are mutable, not cryptographic attestation or model acceptance. Missing/expired fields are never filled from a model catalog.")}
 Label {objectName:"intelligenceReadingTime";text:app.trx("وقت قراءة الخصائص: ","Properties collected: ")+(panel.report.propertiesObservedAtUtc||"—");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:app.design.muted;font.pixelSize:12;textFormat:Text.PlainText}
 Repeater {model:panel.report.warnings||[]
  delegate:Label {required property var modelData;objectName:"intelligenceWarning-"+modelData.id;text:panel.app.ar?modelData.ar:modelData.en;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:panel.app.design.danger;textFormat:Text.PlainText}
 }
 GridLayout {
  Layout.fillWidth:true;columns:width>=1000?4:width>=650?3:2;columnSpacing:16;rowSpacing:16
  Repeater {id:fields;model:panel.fieldsReady?(panel.report.fields||[]).length:0
   delegate:ColumnLayout {
    required property int index
    readonly property var modelData:(panel.report.fields||[])[index]||({})
    objectName:"intelligenceField-"+modelData.id
    Layout.fillWidth:true;Layout.minimumWidth:0;Layout.alignment:Qt.AlignTop;spacing:4
    Label {text:panel.app.ar?modelData.labelAr:modelData.labelEn;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:panel.app.design.muted;font.pixelSize:11}
    Label {objectName:"intelligenceValue-"+modelData.id;text:modelData.state==="reported"?modelData.value:panel.fieldState(modelData.state);Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;color:modelData.state==="reported"?panel.app.design.text:panel.app.design.muted;font.bold:modelData.state==="reported";textFormat:Text.PlainText;font.pixelSize:13}
    Label {text:modelData.source+"\n"+modelData.key;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;color:panel.app.design.muted;font.pixelSize:10;textFormat:Text.PlainText;horizontalAlignment:Text.AlignLeft;LayoutMirroring.enabled:false}
   }
  }
 }
 GridLayout {
  visible:panel.report.profileCurrent&&panel.health.transport==="adb"
  Layout.fillWidth:true;columns:width>=850?3:1;columnSpacing:12;rowSpacing:12
  Panel {title:panel.app.trx("مؤشرات البطارية","Battery indicators");iconName:"gauge";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:panel.app.trx("الشحن: ","Charge: ")+(panel.battery.chargePercent===undefined?"?":Number(panel.battery.chargePercent).toFixed(1)+"%")+"\n"+panel.app.trx("الحرارة: ","Temperature: ")+(panel.battery.temperatureC===undefined?"?":panel.battery.temperatureC+" °C");color:panel.app.design.text}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:(panel.battery.source||"none")+"\n"+(panel.battery.observedAtUtc||"—");color:panel.app.design.muted;font.pixelSize:11;textFormat:Text.PlainText}
  }
  Panel {title:panel.app.trx("مساحة /data المعلنة","Reported /data capacity");iconName:"database";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:panel.storage.state==="observed"?panel.app.trx("المتاح: ","Available: ")+(Number(panel.storage.availableKiB)/1048576).toFixed(2)+" GiB\n"+panel.app.trx("الإجمالي: ","Total: ")+(Number(panel.storage.totalKiB)/1048576).toFixed(2)+" GiB":panel.app.trx("غير متاحة ضمن القراءة والصلاحيات","Unavailable in this reading/permission scope");color:panel.app.design.text}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:(panel.storage.source||"none")+"\n"+(panel.storage.observedAtUtc||"—");color:panel.app.design.muted;font.pixelSize:11;textFormat:Text.PlainText}
  }
  Panel {title:panel.app.trx("الأجهزة الكتلية المرئية","Visible block devices");iconName:"board";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:panel.partitions.state==="observed"?(panel.partitions.entries||[]).length+panel.app.trx(" سجل — ليست خريطة تفليش"," entries — not a flashing map"):panel.app.trx("لم تتوفر بيانات أقسام","Partition observations unavailable");color:panel.app.design.text}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;text:(panel.partitions.source||"none")+"\n"+(panel.partitions.observedAtUtc||"—");color:panel.app.design.muted;font.pixelSize:11;textFormat:Text.PlainText}
  }
 }
 Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("البطارية والتخزين والأقسام تحتاج قراءة القدرات الموسعة عبر مسار المراجعة. توفر خدمة في السياسة الأصلية يعني إمكانية مراجعتها، وليس تفويض تنفيذها أو اعتماد الهاتف.","Battery, storage and block devices require the reviewed extended capability probe. Native-policy availability means eligible for review, not execution approval or phone certification.")}
 Label {objectName:"intelligenceServiceCount";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.text;text:(panel.report.capabilities||[]).filter(function(item){return item.state==="available-for-review"}).length+app.trx(" خدمة متاحة لمراجعة متطلباتها وفق السياسة الحالية"," services eligible for requirement review under current policy")}
 Flow {Layout.fillWidth:true;spacing:8
  ActionButton {objectName:"intelligenceReviewCapabilities";text:app.trx("مراجعة القدرات وقراءات العتاد","Review capabilities & health readings");enabled:toolBridge.selectedSerial!==""&&!toolBridge.busy;onClicked:app.navigateServices()}
  ActionButton {objectName:"intelligenceUsbDoctor";text:"USB Doctor";onClicked:app.navigateSection(4)}
 }
 Label {objectName:"intelligenceScope";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;font.pixelSize:11;color:app.design.muted;text:app.trx("مطابقة USB: ","USB correlation: ")+(panel.report.usbScope||"not-correlated")+app.trx(" · اعتماد الموديل والتحقق التشفيري غير مثبتين."," · Model acceptance and cryptographic attestation remain unestablished.")}
}
