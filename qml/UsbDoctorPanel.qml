import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:doctor
 objectName:"usbDoctorPanel"
 property var app
 readonly property var report:toolBridge.connectionDiagnosis
 title:"NEXVARY USB Doctor";iconName:"phone_laptop";accent:"blue"
 stateText:app.trx("قراءة وتفسير فقط","READ & EXPLAIN")
 function finding(id){for(let i=0;i<findings.count;i++){const item=findings.itemAt(i);if(item&&item.modelData.id===id)return item}return null}
 function readingState(value){const states={"ok":["قراءة ناجحة","Read completed"],"running":["قيد القراءة","Reading"],"failed":["قراءة فاشلة / ملغاة","Failed / cancelled"],"unavailable":["أداة غير متاحة","Tool unavailable"],"windows-only":["يتطلب Windows","Requires Windows"],"not-read":["لم يُقرأ بعد","Not read"]};return states[value]?app.trx(states[value][0],states[value][1]):value}
 function ageState(value){return value==="current"?app.trx("حديثة","Current"):value==="stale"?app.trx("منتهية الصلاحية","Expired"):value==="running"?app.trx("قيد القراءة","Reading"):app.trx("دون قراءة","Unread")}
 Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("يفصل بين رصد الاتصال، تفسير الحالة، والسبب المادي غير المثبت. صلاحية القراءة التفسيرية دقيقتان؛ لا يثبت ذلك اعتماد موديل أو سلامة كابل. لا يثبت البرنامج تعريفات ولا يعيد تشغيل ADB تلقائيًا.","Separates connection observations, interpretation and unproven physical causes. Advisory observations expire after two minutes; this is not model or cable certification. No driver installation or automatic ADB restart.")}
 Flow {
  Layout.fillWidth:true;spacing:8
  ActionButton {objectName:"doctorScanAdb";text:app.trx("اكتشاف ADB","Discover ADB");primary:true;enabled:!toolBridge.busy;onClicked:toolBridge.scanAdb()}
  ActionButton {objectName:"doctorScanFastboot";text:app.trx("اكتشاف Fastboot","Discover Fastboot");enabled:!toolBridge.busy;onClicked:toolBridge.scanFastboot()}
  ActionButton {objectName:"doctorReadUsb";text:app.trx("جرد Windows دون تحديد هاتف","Windows inventory — no target required");enabled:!toolBridge.busy;onClicked:toolBridge.inspectUsbInventory()}
  ActionButton {objectName:"doctorSelectTarget";text:app.trx("اختيار الجهاز وقراءة هويته","Select target / read identity");onClicked:app.navigateSection(1)}
 }
 GridLayout {
  Layout.fillWidth:true;columns:width>=800?3:1;columnSpacing:8;rowSpacing:8
  Repeater {model:doctor.report.sources||[]
   delegate:Rectangle {
    required property var modelData
    Layout.fillWidth:true;implicitHeight:sourceColumn.implicitHeight+20;radius:8;color:doctor.app.design.raised;border.color:doctor.app.design.border
    ColumnLayout {id:sourceColumn;anchors.fill:parent;anchors.margins:10;spacing:4
     Label {text:modelData.source.toUpperCase()+" · "+doctor.readingState(modelData.state);Layout.fillWidth:true;wrapMode:Text.WordWrap;color:doctor.app.design.text;font.bold:true;font.pixelSize:12}
     Label {text:doctor.ageState(modelData.freshness);Layout.fillWidth:true;color:modelData.freshness==="stale"?doctor.app.design.danger:doctor.app.design.muted;font.pixelSize:11}
     Label {text:modelData.observedAtUtc||"—";Layout.fillWidth:true;elide:Text.ElideMiddle;color:doctor.app.design.muted;font.pixelSize:11;textFormat:Text.PlainText}
    }
   }
  }
 }
 GridLayout {
  Layout.fillWidth:true;columns:(doctor.report.findings||[]).length>1&&width>=1050?2:1;columnSpacing:12;rowSpacing:12
  Repeater {id:findings;model:doctor.report.findings||[]
   delegate:Panel {
    required property var modelData
    objectName:"doctorFinding-"+modelData.id
    Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    title:doctor.app.ar?modelData.titleAr:modelData.titleEn;iconName:modelData.severity==="warning"?"warning":"search_visual"
    stateText:modelData.confidence==="reported"?doctor.app.trx("تصريح مرصود","REPORTED"):doctor.app.trx("غير محسوم","UNRESOLVED")
    Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:doctor.app.design.muted;text:doctor.app.ar?modelData.detailAr:modelData.detailEn}
    Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:doctor.app.design.text;text:doctor.app.trx("الخطوة التالية: ","Next check: ")+(doctor.app.ar?modelData.nextAr:modelData.nextEn)}
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:11;color:doctor.app.design.muted;text:doctor.app.trx("المصدر: ","Source: ")+modelData.source}
   }
  }
 }
 Label {objectName:"doctorScope";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("نطاق USB: ","USB scope: ")+(doctor.report.usbScope||"not-correlated")+app.trx(". أرقام VID/PID/MI مستخرجة من هوية Windows، وليست قراءة واصفات USB. السبب المادي واعتماد الواجهة غير مثبتين.",". VID/PID/MI come from Windows instance IDs, not USB descriptor reads. Physical cause and interface certification remain unverified.")}
 CheckBox {id:showNodes;objectName:"doctorShowNodes";text:app.trx("إظهار عقد USB المقروءة — قد تحتوي معرّفات خاصة","Show observed USB nodes — may contain private identifiers");enabled:(doctor.report.nodes||[]).length>0}
 Repeater {
  model:showNodes.checked?doctor.report.nodes||[]:[]
  delegate:Label {required property var modelData;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:doctor.app.design.muted;font.pixelSize:12;text:(modelData.name||"?")+"\n"+modelData.instanceId+"\nVID "+(modelData.vid||"?")+" · PID "+(modelData.pid||"?")+" · MI "+(modelData.interfaceNumber||"?")+" · "+(modelData.service||"?")+" · "+(modelData.infName||"?")}
 }
}
