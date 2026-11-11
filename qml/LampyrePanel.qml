import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id: panel
 property var app
 readonly property var bridge: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 objectName: "lampyrePanel"
 title: app.trx("Lampyre — تقييم دون اتصال", "Lampyre — offline evaluation")
 iconName: "shield_checkmark"
 LayoutMirroring.enabled: app.ar
 LayoutMirroring.childrenInherit: true
 Label {
  Layout.fillWidth: true; wrapMode: Text.WordWrap; color: panel.app.design.danger
  text: panel.app.trx("التجربة: التفعيل غير مؤكد • الاتصال الفعلي معطل حتى مراجعة توثيق الحساب والصلاحيات.", "Trial activation unconfirmed • Live connection disabled pending account documentation and permissions.")
 }
 Label {
  Layout.fillWidth: true; wrapMode: Text.WordWrap; color: panel.app.design.muted
  text: panel.app.trx("الرصيد الفعلي: غير متاح. عرض 200 Photon غير متحقق. التكلفة غير متاحة — يلزم التأكيد. لا تُرسل أي بيانات أو طلبات مدفوعة من هذه الشاشة.", "Actual balance unavailable. The offered 200 Photons are unverified. Cost unavailable — confirmation required. This screen sends no data or paid requests.")
 }
 Label {
  Layout.fillWidth: true; wrapMode: Text.WordWrap; color: panel.app.design.text
  text: panel.app.trx("مختبر محلي ببيانات اصطناعية ثابتة. النتائج ليست أدلة أو نتائج Lampyre فعلية.", "Local laboratory with fixed synthetic data. Results are neither evidence nor actual Lampyre results.")
 }
 ComboBox {
  id: scenario; objectName: "lampyreScenario"; Layout.fillWidth: true
  enabled: !!panel.bridge && !panel.bridge.busy
  model: panel.app.ar ? ["نجاح اصطناعي", "فشل المصادقة", "انتهاء المهلة", "رصيد غير كافٍ", "حد الطلبات"] : ["Synthetic success", "Authentication failure", "Timeout", "Insufficient credits", "Rate limited"]
 }
 Flow {
  Layout.fillWidth: true; spacing: 8
  ActionButton {
   objectName: "lampyreRunSynthetic"
   text: panel.app.trx("تشغيل اختبار محلي", "Run local fixture")
   enabled: !!panel.bridge && !panel.bridge.busy
   onClicked: panel.bridge.runSynthetic(scenario.currentIndex)
  }
  ActionButton {
   objectName: "lampyreCancelSynthetic"; text: panel.app.trx("إلغاء الاختبار", "Cancel fixture")
   enabled: !!panel.bridge && panel.bridge.busy
   onClicked: panel.bridge.cancel()
  }
 }
 Label {
  Layout.fillWidth: true; textFormat: Text.PlainText; wrapMode: Text.WrapAnywhere
  color: panel.app.design.text
  text: panel.bridge ? panel.bridge.state : panel.app.trx("المختبر غير متاح في هذه البيئة", "Laboratory unavailable in this environment")
 }
 TextArea {
  Layout.fillWidth: true; readOnly: true; selectByMouse: true
  textFormat: TextEdit.PlainText; wrapMode: TextEdit.WrapAnywhere
  text: panel.bridge ? JSON.stringify(panel.bridge.result, null, 2) : ""
 }
}
