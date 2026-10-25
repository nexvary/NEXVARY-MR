import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id:status
 property var app
 property bool failed:false
 property string lastSource:"device"
 readonly property bool working:lawCaseBridge.importBusy||toolBridge.foregroundBusy||firmwareBridge.busy||recoveryBridge.busy||securityBridge.busy
 Connections {target:lawCaseBridge;function onImportChanged(){if(lawCaseBridge.importBusy){status.lastSource="import";status.failed=false}}function onImportCompleted(ok){status.lastSource="import";status.failed=!ok}}
 Connections {target:toolBridge;function onBusyChanged(){if(toolBridge.foregroundBusy){status.lastSource="device";status.failed=false}}function onOperationFinished(ok){status.lastSource="device";status.failed=!ok}}
 Connections {target:firmwareBridge;function onBusyChanged(){if(firmwareBridge.busy){status.lastSource="firmware";status.failed=false}}function onInspectionFinished(matched){status.lastSource="firmware";status.failed=!matched&&firmwareBridge.avbReport.status!=="metadata-only"&&firmwareBridge.packageReport.status!=="structure-only"}}
 Connections {target:recoveryBridge;function onChanged(){if(recoveryBridge.busy){status.lastSource="case";status.failed=false}else if(status.lastSource==="case")status.failed=recoveryBridge.error!==""}}
 Connections {target:securityBridge;function onChanged(){if(securityBridge.busy){status.lastSource="security";status.failed=false}else if(status.lastSource==="security")status.failed=securityBridge.error!==""}}
 color:app.design.surface;radius:8;border.color:app.design.border
 implicitHeight:44
 RowLayout {
  anchors.fill:parent;anchors.margins:8;spacing:10
  BusyIndicator {visible:status.working;running:status.working;Layout.preferredWidth:24;Layout.preferredHeight:24}
  StateBadge {objectName:"operationOutcome";text:status.working?status.app.trx("قيد التنفيذ","IN PROGRESS"):status.failed?status.app.trx("راجع النتيجة","REVIEW FAILURE"):status.app.trx("جاهز","READY");tint:status.failed?status.app.design.danger:status.working?status.app.design.accent:status.app.design.success}
  Label {objectName:"operationStatus";text:status.lastSource==="import"?(lawCaseBridge.importBusy?status.app.trx("استيراد الأدلة قيد التنفيذ","Evidence import in progress"):lawCaseBridge.error||status.app.trx("راجع نتيجة الاستيراد","Review import result")):status.lastSource==="firmware"?firmwareBridge.status:status.lastSource==="case"?recoveryBridge.status:status.lastSource==="security"?(securityBridge.busy?status.app.trx("فحص الأمان قيد التنفيذ","Security operation in progress"):securityBridge.error||status.app.trx("راجع نتيجة فحص الأمان","Review security results")):toolBridge.status;Layout.fillWidth:true;Layout.minimumWidth:0;elide:Text.ElideRight;color:status.failed?status.app.design.danger:status.app.design.muted;textFormat:Text.PlainText;font.pixelSize:12;ToolTip.visible:statusMouse.containsMouse;ToolTip.text:text;MouseArea {id:statusMouse;anchors.fill:parent;hoverEnabled:true;acceptedButtons:Qt.NoButton}}
  ActionButton {objectName:"cancelActiveOperation";visible:status.working;text:status.app.trx("إلغاء العملية","Cancel operation");implicitHeight:30;onClicked:{lawCaseBridge.cancelImport();toolBridge.cancel();firmwareBridge.cancel();recoveryBridge.cancel();securityBridge.cancel()}}
 }
}
