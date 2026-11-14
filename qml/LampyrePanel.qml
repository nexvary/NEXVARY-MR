import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id: panel
 property var app
 readonly property var bridge: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 objectName: "lampyrePanel"
 title: "NEXVARY Intelligence"; subtitle: app.trx("اتصال المزوّد والمختبر المحلي", "Provider connection & local laboratory")
 iconName: "shield_checkmark"; accent: "blue"
 LayoutMirroring.enabled: app.ar
 LayoutMirroring.childrenInherit: true
 GridLayout {
  Layout.fillWidth: true; columns: width > 740 ? 2 : 1; columnSpacing: 18; rowSpacing: 14
  Panel {
   Layout.fillWidth: true; Layout.fillHeight: true; title: panel.app.trx("حالة الاتصال", "Connection status"); iconName: "document_lock"
   StateBadge {text: panel.app.trx("التفعيل غير مؤكد", "Activation unconfirmed"); tint: "#6BB8FF"}
   Label {text: panel.app.trx("API / MCP غير متصل", "API / MCP disconnected"); font.pixelSize:22; font.bold: true; color: panel.app.design.text; Layout.fillWidth: true; wrapMode: Text.WordWrap}
   Label {text: panel.app.trx("التوثيق والصلاحيات الخاصة بالحساب مطلوبة قبل الاتصال. لا توجد مفاتيح مخزنة أو طلبات مدفوعة من هذا القسم.", "Account documentation and permissions are required before connecting. This section stores no keys and submits no paid requests."); color: panel.app.design.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  }
  Panel {
   Layout.fillWidth: true; Layout.fillHeight: true; title: panel.app.trx("التكلفة والتفويض", "Cost & approval"); iconName: "database"
   Label {text: panel.app.trx("الرصيد غير متاح", "Balance unavailable"); color: panel.app.design.text; font.pixelSize:22; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap}
   Label {text: panel.app.trx("تكلفة البحث غير متاحة. البحث الفعلي معطّل.", "Search cost is unavailable. Live research is disabled."); color: panel.app.design.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap}
  }
 }
 Panel {
  Layout.fillWidth: true; title: panel.app.trx("مختبر التحقق المحلي", "Local validation laboratory"); iconName: "settings"
  subtitle: panel.app.trx("بيانات ثابتة اصطناعية • دون اتصال • دون تكلفة", "Fixed synthetic fixtures • Offline • No cost")
  GridLayout {
   Layout.fillWidth: true; columns: width > 650 ? 3 : 1; columnSpacing: 12; rowSpacing: 10
   NeonComboBox {
    id: scenario; objectName: "lampyreScenario"; Layout.fillWidth: true
    enabled: !!panel.bridge && !panel.bridge.busy
    model: panel.app.ar ? ["نجاح اصطناعي", "فشل المصادقة", "انتهاء المهلة", "رصيد غير كافٍ", "حد الطلبات"] : ["Synthetic success", "Authentication failure", "Timeout", "Insufficient credits", "Rate limited"]
   }
   ActionButton {objectName: "lampyreRunSynthetic"; primary: true; text: panel.app.trx("تشغيل الاختبار", "Run fixture"); enabled: !!panel.bridge && !panel.bridge.busy; onClicked: panel.bridge.runSynthetic(scenario.currentIndex)}
   ActionButton {objectName: "lampyreCancelSynthetic"; text: panel.app.trx("إلغاء", "Cancel"); enabled: !!panel.bridge && panel.bridge.busy; onClicked: panel.bridge.cancel()}
  }
  Label {
   objectName: "lampyreStatus"; Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize:15; color: panel.app.design.silver
   text: !panel.bridge ? panel.app.trx("المختبر غير متاح في هذه البيئة", "Laboratory unavailable in this environment") : panel.bridge.busy ? panel.app.trx("جارٍ تشغيل اختبار محلي…", "Running local fixture…") : panel.bridge.state === "synthetic-completed" ? panel.app.trx("اكتمل الاختبار المحلي — ليس بحثًا فعليًا", "Local fixture completed — not a live search") : panel.bridge.state === "synthetic-failed" ? panel.app.trx("تمت محاكاة الخطأ المختار", "Selected error simulated") : panel.bridge.state === "synthetic-cancelled" ? panel.app.trx("أُلغي الاختبار المحلي", "Local fixture cancelled") : panel.app.trx("جاهز لاختبار اصطناعي", "Ready for a synthetic fixture")
  }
  Label {Layout.fillWidth: true; wrapMode: Text.WordWrap; color: panel.app.design.muted; text: panel.app.trx("النتائج الاصطناعية لا تُحفظ كأدلة ولا تثبت وجود حساب أو هوية شخص.", "Synthetic results are not retained as evidence and do not establish an account or identity.")}
  NeonCheckBox {id: details; text: panel.app.trx("إظهار التفاصيل التقنية", "Show technical details")
   contentItem: Text {text: details.text; color: panel.app.design.text; font: details.font; verticalAlignment: Text.AlignVCenter; leftPadding: details.mirrored ? 0 : 34; rightPadding: details.mirrored ? 34 : 0}
  }
  EntryArea {visible: details.checked; Layout.fillWidth: true; readOnly: true; selectByMouse: true; textFormat: TextEdit.PlainText; wrapMode: TextEdit.WrapAnywhere; text: panel.bridge ? JSON.stringify(panel.bridge.result, null, 2) : "{}"}
 }
}
