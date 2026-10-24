import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"reportsWorkspace";contentWidth:availableWidth;clip:true
 ColumnLayout {
  width:page.availableWidth;spacing:16
  Label {text:page.app.trx("تقارير مرتبطة بمصدرها","Reports with source context");color:page.app.design.text;font.pixelSize:25;font.bold:true}
  Label {text:page.app.trx("اختر نوع التقرير لفتح السجل والنتيجة وضوابط التصدير الخاصة به. وجود تقرير لا يثبت اعتماد الجهاز أو اكتمال الإصلاح.","Choose a report to open its source record, results and export controls. A report does not establish model acceptance or repair success.");color:page.app.design.muted;Layout.fillWidth:true;wrapMode:Text.WordWrap}
  GridLayout {
   Layout.fillWidth:true;columns:width>1050?3:2;columnSpacing:12;rowSpacing:12
   ActionCard {app:page.app;title:page.app.trx("خدمات الجهاز","Device services");iconName:"phone";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("سجل العمليات ونتائج التحقق؛ التقرير التقني يستبعد السجلات الخام ومعرفات USB.","Operation history and verification results; the technical projection omits raw logs and USB identifiers.");statusText:toolBridge.serviceHistory.length+page.app.trx(" عملية"," operation(s)");actionText:page.app.trx("مراجعة سجل الخدمات","Review service history");actionName:"reportDeviceServices";onTriggered:page.app.navigateServices()}
   ActionCard {app:page.app;title:page.app.trx("نتائج التشخيص","Diagnostic findings");iconName:"gauge";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("نتائج الفني المرتبطة بأمر محدد، مع الملاحظات والدليل الذي راجعه.","Technician findings linked to a specific order with reviewed evidence and notes.");statusText:page.app.trx("JSON / فاتورة PDF","JSON / invoice PDF");actionText:page.app.trx("فتح نتائج التشخيص","Open findings");actionName:"reportDiagnosis";onTriggered:page.app.navigateSection(2)}
   ActionCard {app:page.app;title:page.app.trx("تقارير الأدلة","Evidence reports");iconName:"document_lock";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("القضية والبصمات وسجل الحيازة؛ اختر الدليل وراجع موافقة التصدير غير المشفر.","Case metadata, hashes and custody declarations; review plaintext export consent.");statusText:"PDF / HTML / JSON";actionText:page.app.trx("فتح مستعرض الأدلة","Open evidence browser");actionName:"reportEvidence";onTriggered:page.app.navigateEvidence()}
   ActionCard {app:page.app;title:page.app.trx("فواتير وبطاقات الورشة","Invoices & device cards");iconName:"clipboard_task";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("اختر أمرًا لعرض فاتورته أو بطاقة باركود الجهاز. التصدير يشمل بيانات عميل.","Select an order for its invoice or barcode card. Exports contain customer data.");statusText:page.app.trx("PDF / CSV","PDF / CSV");actionText:page.app.trx("اختيار أمر الصيانة","Select repair order");actionName:"reportWorkshop";onTriggered:page.app.navigateSection(7)}
   ActionCard {app:page.app;title:page.app.trx("تحليل Firmware وAVB","Firmware & AVB");iconName:"arrow_square_down";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("نتائج البنية والبصمات والبيانات المعلنة، مع حدود المصادقة والتوافق.","Container, hash and declared metadata results with trust and compatibility limits.");statusText:page.app.trx("تقارير محلية","Local analysis");actionText:page.app.trx("فتح نتائج Firmware","Open firmware results");actionName:"reportFirmware";onTriggered:page.app.navigateSection(3)}
   ActionCard {app:page.app;title:page.app.trx("تقارير الأمان","Security scans");iconName:"shield_checkmark";Layout.fillWidth:true;Layout.fillHeight:true;description:page.app.trx("نتيجة فحص ملف مصرح به بإصدار محرك وقاعدة توقيعات؛ عدم الكشف ليس ضمان خلو.","Authorized file scans with engine/signature provenance; no detection is not a clean guarantee.");statusText:page.app.trx("محرك خارجي مطلوب","External engine required");actionText:page.app.trx("فتح فحص الأمان","Open security scans");actionName:"reportSecurity";onTriggered:page.app.navigateSecurity()}
  }
 }
}
