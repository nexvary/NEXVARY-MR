import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"helpWorkspace";contentWidth:availableWidth;clip:true
 ColumnLayout {
  width:page.availableWidth;spacing:16
  Label {text:page.app.trx("مساعدة محطة العمل","Workstation help");font.pixelSize:25;font.bold:true;color:page.app.design.text}
  GridLayout {
   Layout.fillWidth:true;columns:width>900?2:1;columnSpacing:12;rowSpacing:12
   Panel {title:page.app.trx("ابدأ بجهاز واحد محدد","Start with one explicit target");iconName:"phone";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("1. اختر ADB أو Fastboot من مدير الأجهزة.\n2. حدد الجهاز؛ اقرأ هويته وبناءه.\n3. افتح التشخيص والخدمة المناسبة.\n4. راجع الموافقة والنتيجة والتحقق.\n5. صدّر التقرير من مصدره بعد مراجعة بيانات العميل.","1. Discover ADB or Fastboot targets.\n2. Select a device and read its identity/build.\n3. Choose diagnostics and a scoped service.\n4. Review consent, result and verification.\n5. Export from the source context after reviewing customer data.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    ActionButton {text:page.app.trx("فتح مدير الأجهزة","Open Device Manager");onClicked:page.app.navigateSection(1)}
   }
   Panel {title:page.app.trx("حالة القدرات والموافقات","Capabilities & consent");iconName:"shield_checkmark";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("نسخة Production تقيد عمليات الهاتف بالقراءة. نسخة Laboratory منفصلة للتجارب ولا تمنح اعتمادًا للموديل. عرض الشاشة يحتاج مراجعة موافقة وتشغيل مكون مؤقت. يظل سبب عدم التوفر ظاهرًا داخل بطاقة العملية.","Production restricts connected-device operations to reads. Laboratory is a separate experimental build, not model certification. Screen viewing requires consent and a temporary device component. Availability reasons appear within each operation card.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    StateBadge {text:toolBridge.laboratoryBuild?"LABORATORY / EXPERIMENTAL":"PRODUCTION / READ-ONLY";tint:page.app.design.accent}
   }
   Panel {title:page.app.trx("الأدوات والتوقيعات","Tools & signatures");iconName:"toolbox";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("اختر الملفات التنفيذية الرسمية من الإعدادات. فحص ClamAV يستخدم clamscan.exe؛ تحديث القاعدة يستخدم freshclam.exe المجاور. احفظ إعداد المحرك ومجلد توقيعات قابلًا للكتابة قبل التحديث. لا ترسل ملفات العميل إلى خدمات خارجية تلقائيًا.","Choose trusted executables in Settings. ClamAV scanning uses clamscan.exe and signature updates use adjacent freshclam.exe. Save the engine and a writable signature directory before updating. Customer files are not uploaded automatically.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    ActionButton {text:page.app.trx("فتح إعدادات الأدوات","Open tool settings");onClicked:page.app.navigateSection(10)}
   }
   Panel {title:page.app.trx("عن NEXVARY MR","About NEXVARY MR");iconName:"question_circle";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("محطة صيانة وتحليل أدلة محلية — 0.4.0 تطوير.\nاختبار البناء لا يساوي اعتماد الأجهزة أو التوقيع التجاري.\nالأيقونات: Microsoft Fluent System Icons — MIT.\nCtrl+R: اكتشاف ADB · Ctrl+Q: إغلاق بمراجعة.\nزر الرجوع يعيدك إلى القسم السابق.","Local repair and evidence workstation — 0.4.0 development.\nBuild tests do not establish hardware acceptance or publisher signing.\nIcons: Microsoft Fluent System Icons — MIT.\nCtrl+R: ADB discovery · Ctrl+Q: reviewed exit.\nBack restores the previous workspace.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    ActionButton {text:page.app.trx("المشروع والمصادر","Project & sources");onClicked:Qt.openUrlExternally("https://github.com/nexvary/NEXVARY-MR")}
   }
  }
 }
}
