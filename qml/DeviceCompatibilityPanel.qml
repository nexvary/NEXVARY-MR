import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:panel
 objectName:"deviceCompatibilityPanel"
 property var app
 property var profile:({})
 readonly property var platform:profile.platform||({})
 readonly property var layoutInfo:profile.layout||({})
 readonly property var security:profile.security||({})
 title:app.trx("توافق Android وبنية النظام","Android compatibility and system layout")
 visible:!!profile.schemaVersion
 function state(value){
  const words={"reported-enabled":["مفعّل حسب الجهاز","Reported enabled"],"reported-disabled":["معطّل حسب الجهاز","Reported disabled"],"recognized-api":["إصدار معروف","Recognized API"],"unrecognized-api":["إصدار غير مسجل؛ فحص القراءة فقط","Unrecognized API; read-only probing"],"invalid-api":["قيمة API غير صالحة","Invalid API value"],"reported-preview":["نسخة تجريبية حسب الجهاز","Reported preview"],"reported-release":["نسخة إصدار حسب الجهاز","Reported release"],"userspace-fastbootd":["fastbootd في مساحة المستخدم","Userspace fastbootd"],"bootloader":["Fastboot في Bootloader","Bootloader fastboot"]}
  return words[value]?app.trx(words[value][0],words[value][1]):app.trx("غير معروف","Unknown")
 }
 Label {objectName:"compatibilityEvidence";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("قيم أبلغ عنها الجهاز؛ اختبار الهاتف الفعلي معلق. لا تمثل اعتماد OEM أو تحققًا تشفيريًا من سلامة الإقلاع.","Device-reported values; physical phone validation pending. No OEM approval or cryptographic boot attestation.")}
 Label {objectName:"compatibilityPlatform";Layout.fillWidth:true;visible:panel.profile.transport==="adb";wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#c7deef");text:"Android: "+(panel.platform.androidFamily||"?")+" · API: "+(panel.platform.apiLevel||"?")+" · "+panel.state(panel.platform.state)+" · "+panel.state(panel.platform.previewState)}
 Label {objectName:"compatibilityPageSize";Layout.fillWidth:true;visible:panel.profile.transport==="adb";wrapMode:Text.WordWrap;color:app.tone("#c7deef");text:app.trx("حجم صفحة الذاكرة: ","Memory page size: ")+(panel.platform.pageSizeBytes?panel.platform.pageSizeBytes+" B":app.trx("غير متاح","Unavailable"))}
 Label {Layout.fillWidth:true;visible:panel.profile.transport==="adb";wrapMode:Text.WordWrap;color:app.tone("#a8bfd4");text:"Dynamic partitions: "+panel.state(panel.layoutInfo.dynamicPartitions)+" · Virtual A/B: "+panel.state(panel.layoutInfo.virtualAb)+" · A/B: "+panel.state(panel.layoutInfo.abUpdates)+" · Slot: "+(panel.layoutInfo.slotSuffix||"?")}
 Label {objectName:"compatibilityFastbootMode";Layout.fillWidth:true;visible:panel.profile.transport==="fastboot";wrapMode:Text.WordWrap;color:app.tone("#69b7ff");text:panel.state(panel.layoutInfo.fastbootMode)}
 Label {Layout.fillWidth:true;visible:panel.profile.transport==="adb";wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8bfd4");text:"Verified Boot: "+(panel.security.verifiedBoot||"?")+" · "+app.trx("التشفير: ","Encryption: ")+(panel.security.encryptionState||"?")+" / "+(panel.security.encryptionType||"?")+" · "+app.trx("التصحيح الأمني: ","Security patch: ")+(panel.security.securityPatch||"?")}
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("وجود ADB أو FBE أو أقسام ديناميكية لا يمنح وصولًا لبيانات محمية أو إذنًا بالتفليش. القيم الغائبة تبقى غير معروفة.","ADB, FBE or dynamic partitions grant no protected-data access or flashing approval. Missing observations remain unknown.")}
}
