import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 property var app
 objectName:"helpWorkspace"
 ColumnLayout {
  anchors.fill:parent;spacing:12
  TabBar {id:aboutTabs;objectName:"helpAboutTabs";Layout.fillWidth:true
   TabButton {text:page.app.trx("المساعدة","Help")}
   TabButton {text:page.app.trx("عن النظام","About the system")}
   TabButton {text:page.app.trx("عن المطوّر","About the developer")}
  }
  StackLayout {currentIndex:aboutTabs.currentIndex;Layout.fillWidth:true;Layout.fillHeight:true
  ScrollView {id:helpScroll;contentWidth:availableWidth;clip:true
  ColumnLayout {width:helpScroll.availableWidth;spacing:16
  Label {text:page.app.trx("مساعدة محطة العمل","Workstation help");font.pixelSize:25;font.bold:true;color:page.app.design.text}
  GridLayout {
   Layout.fillWidth:true;columns:width>900?2:1;columnSpacing:12;rowSpacing:12
   Panel {title:page.app.trx("ابدأ بجهاز واحد محدد","Start with one explicit target");iconName:"phone";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("1. اختر ADB أو Fastboot من مدير الأجهزة.\n2. حدد الجهاز؛ اقرأ هويته وبناءه.\n3. افتح التشخيص والخدمة المناسبة.\n4. راجع الموافقة والنتيجة والتحقق.\n5. صدّر التقرير من مصدره بعد مراجعة بيانات العميل.","1. Discover ADB or Fastboot targets.\n2. Select a device and read its identity/build.\n3. Choose diagnostics and a scoped service.\n4. Review consent, result and verification.\n5. Export from the source context after reviewing customer data.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    ActionButton {text:page.app.trx("فتح مدير الأجهزة","Open Device Manager");onClicked:page.app.navigateSection(1)}
   }
   Panel {title:page.app.trx("حالة القدرات والموافقات","Capabilities & consent");iconName:"shield_checkmark";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("نسخة Production تقيد عمليات الهاتف بالقراءة. نسخة Laboratory منفصلة للتجارب ولا تمنح اعتمادًا للموديل. عرض الشاشة يحتاج مراجعة موافقة وتشغيل مكون مؤقت. يظل سبب عدم التوفر ظاهرًا داخل بطاقة العملية.","Production restricts connected-device operations to reads. Laboratory is a separate experimental build, not model certification. Screen viewing requires consent and a temporary device component. Availability reasons appear within each operation card.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    StateBadge {text:toolBridge.laboratoryBuild?page.app.trx("مختبر تجريبي","Experimental laboratory"):page.app.trx("نسخة الإنتاج: فحص وقراءة","Production: inspection & reads");tint:page.app.design.accent}
   }
   Panel {title:page.app.trx("الأدوات والتوقيعات","Tools & signatures");iconName:"toolbox";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
    Label {text:page.app.trx("اختر الملفات التنفيذية الرسمية من الإعدادات. فحص ClamAV يستخدم clamscan.exe؛ تحديث القاعدة يستخدم freshclam.exe المجاور. احفظ إعداد المحرك ومجلد توقيعات قابلًا للكتابة قبل التحديث. لا ترسل ملفات العميل إلى خدمات خارجية تلقائيًا.","Choose trusted executables in Settings. ClamAV scanning uses clamscan.exe and signature updates use adjacent freshclam.exe. Save the engine and a writable signature directory before updating. Customer files are not uploaded automatically.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    ActionButton {text:page.app.trx("فتح إعدادات الأدوات","Open tool settings");onClicked:page.app.navigateSection(10)}
   }
   Panel {Layout.fillWidth:true;title:page.app.trx("اختصارات لوحة المفاتيح","Keyboard shortcuts");iconName:"settings"
    RowLayout {Layout.fillWidth:true
     Label {text:page.app.trx("اكتشاف أجهزة ADB","Discover ADB devices");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text}
     Label {text:"Ctrl+R";LayoutMirroring.enabled:false;color:page.app.design.silver}
    }
    RowLayout {Layout.fillWidth:true
     Label {text:page.app.trx("إغلاق البرنامج بعد مراجعة العمليات","Close after reviewing active operations");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text}
     Label {text:"Ctrl+Q";LayoutMirroring.enabled:false;color:page.app.design.silver}
    }
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;text:page.app.trx("زر الرجوع يعيدك إلى القسم السابق.","Back returns to the previous workspace.")}
   }
  }
  }
  }
  AboutSystemPage {app:page.app}
  AboutDeveloperPage {app:page.app}
  }
 }
}
