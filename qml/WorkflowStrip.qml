import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Flow {
 id:flow
 property var app
 width:parent.width;spacing:6
 readonly property int reached:Object.keys(toolBridge.deviceInfo).length?2:toolBridge.devices.length?1:0
 Repeater {
  model:flow.app.ar?["اتصال","اكتشاف","تعريف","تشخيص","اختيار الخدمة","مراجعة","تنفيذ","تحقق","تقرير"]:["Connect","Detect","Identify","Diagnose","Service","Review","Execute","Verify","Report"]
  delegate:Rectangle {
   required property int index
   required property string modelData
   width:Math.max(72,caption.implicitWidth+20);height:32;radius:6
   color:index<=flow.reached?Qt.rgba(flow.app.design.accent.r,flow.app.design.accent.g,flow.app.design.accent.b,0.10):flow.app.design.raised
   border.color:index===flow.reached?flow.app.design.accent:flow.app.design.border
   Label {id:caption;anchors.centerIn:parent;text:(index+1)+"  "+modelData;font.pixelSize:11;color:index<=flow.reached?flow.app.design.text:flow.app.design.muted}
  }
 }
}
