import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 property var app
 objectName:"dashboardWorkspace"
 readonly property int readyTools:toolBridge.tools.filter(function(t){return t.available}).length
 ScrollView {
  id:scroll;objectName:"dashboardScroll";anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:scroll.availableWidth;spacing:16
   RowLayout {
    Layout.fillWidth:true
    ColumnLayout {Layout.fillWidth:true;spacing:4
     Label {text:page.app.trx("محطة العمل، في نظرة واحدة","Your workstation at a glance");font.pixelSize:25;font.bold:true;color:page.app.design.text}
     Label {text:page.app.trx("اختر الجهاز، اجمع الدليل، ثم راجع العملية قبل تنفيذها.","Select a target, collect evidence, then review each operation before execution.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted}
    }
    StateBadge {text:page.app.trx("بيانات فعلية فقط","ACTUAL STATE ONLY");tint:page.app.design.silver}
   }
   GridLayout {
    Layout.fillWidth:true;columns:width>720?4:2;columnSpacing:12;rowSpacing:12
    Repeater {
     model:[{title:page.app.trx("أجهزة مكتشفة","Discovered devices"),value:toolBridge.devices.length,icon:"phone",sub:toolBridge.selectedSerial?page.app.trx("تم تحديد الهدف","Target selected"):page.app.trx("لم يُحدد هدف","No target selected")},
      {title:page.app.trx("الأدوات المتاحة","Available tools"),value:page.readyTools+" / "+toolBridge.tools.length,icon:"toolbox",sub:page.app.trx("مصادر محلية مفحوصة","Local executable discovery")},
      {title:page.app.trx("أوامر الورشة","Workshop orders"),value:workshopBridge.orders.length,icon:"clipboard_task",sub:page.app.trx("سجلات محفوظة محليًا","Locally saved records")},
      {title:page.app.trx("أدلة القضية المفتوحة","Open case evidence"),value:recoveryBridge.evidence.length,icon:"document_lock",sub:recoveryBridge.active?recoveryBridge.caseTitle:page.app.trx("لا توجد قضية مفتوحة","No active case")}]
     delegate:Rectangle {
      required property var modelData
      Layout.fillWidth:true;implicitHeight:104;radius:12;color:page.app.design.surface;border.color:page.app.design.border
      RowLayout {anchors.fill:parent;anchors.margins:16;spacing:12
       Image {source:"qrc:/assets/icons/"+modelData.icon+".svg";Layout.preferredWidth:34;Layout.preferredHeight:34;fillMode:Image.PreserveAspectFit}
       ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:2
        Label {text:modelData.title;color:page.app.design.muted;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight}
        Label {text:modelData.value;color:page.app.design.text;font.pixelSize:26;font.bold:true}
        Label {text:modelData.sub;color:page.app.design.muted;font.pixelSize:10;Layout.fillWidth:true;elide:Text.ElideRight;textFormat:Text.PlainText}
       }
      }
     }
    }
   }
   Panel {
    title:page.app.trx("مسار الفني","TECHNICIAN WORKFLOW");iconName:"arrow_sync";Layout.fillWidth:true
    WorkflowStrip {app:page.app;Layout.fillWidth:true}
    Label {text:page.app.trx("مراحل الاتصال والتعريف تعكس المعلومات المقروءة. التنفيذ والتحقق يحتاجان نتيجة العملية نفسها؛ اختيار بطاقة لا يثبت نجاحها.","Connection and identity reflect observed state. Execution and verification require operation results; opening a card does not establish success.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;font.pixelSize:11}
   }
   GridLayout {
    objectName:"dashboardActions";Layout.fillWidth:true;columns:width>=1000?3:2;columnSpacing:12;rowSpacing:12
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("مدير الأجهزة","Device Manager");iconName:"phone";description:page.app.trx("اكتشاف ADB وFastboot، تثبيت الهدف وقراءة هوية الهاتف والسجلات.","Discover ADB and Fastboot targets, select one device and read identity and logs.");statusText:toolBridge.selectedSerial?page.app.trx("هدف محدد","Selected"):page.app.trx("بانتظار هدف","Select target");actionText:page.app.trx("فتح مدير الأجهزة","Open Device Manager");actionName:"dashboardDevices";primary:true;onTriggered:page.app.navigateSection(1)}
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("استوديو الصيانة","Repair Studio");iconName:"toolbox";description:page.app.trx("إرشادات الأعطال، توثيق نتائج الفني، خدمات محددة ومخططات اللوحات.","Guided symptoms, technician findings, scoped services and board inspection.");statusText:page.app.trx("قدرات محددة","Scoped");actionText:page.app.trx("بدء التشخيص","Start diagnostics");actionName:"dashboardRepair";onTriggered:page.app.navigateSection(2)}
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("ذكاء ملفات النظام","Firmware Intelligence");iconName:"arrow_square_down";description:page.app.trx("فحص الحزم والبصمات وبيانات AVB مع فصل البنية عن مصادقة الشركة.","Inspect containers, hashes and AVB declarations with explicit trust boundaries.");statusText:page.app.trx("ملفات محلية","Local files");actionText:page.app.trx("تحليل Firmware","Analyze firmware");actionName:"dashboardFirmware";onTriggered:page.app.navigateSection(3)}
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("المختبر الجنائي","Forensic Laboratory");iconName:"document_lock";description:page.app.trx("قضايا وأدلة مصرح بها، تحقق SHA-256، EXIF وPCAP وسجل حيازة محلي.","Authorized cases, SHA-256 verification, EXIF, PCAP and local custody declarations.");statusText:recoveryBridge.active?page.app.trx("قضية مفتوحة","Case open"):page.app.trx("اختر قضية","Select case");actionText:page.app.trx("فتح المختبر","Open laboratory");actionName:"dashboardForensics";onTriggered:page.app.navigateSection(5)}
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("إدارة الورشة","Workshop CRM");iconName:"clipboard_task";description:page.app.trx("عملاء وأجهزة وأوامر صيانة وباركود وقطع غيار وفواتير.","Customers, devices, repair orders, barcodes, inventory and invoices.");statusText:workshopBridge.ready?page.app.trx("SQLite جاهزة","SQLite ready"):page.app.trx("تعذر الفتح","Unavailable");actionText:page.app.trx("فتح الورشة","Open workshop");actionName:"dashboardWorkshop";onTriggered:page.app.navigateSection(7)}
    ActionCard {app:page.app;Layout.fillWidth:true;Layout.fillHeight:true;title:page.app.trx("التقارير والتسليم","Reports & handover");iconName:"document_text";description:page.app.trx("انتقل إلى التقرير المرتبط بمصدره، راجع البيانات ثم صدّر بالموافقة.","Open reports in their source context, review data and approve export.");statusText:page.app.trx("تصدير بموافقة","Consent required");actionText:page.app.trx("مركز التقارير","Report center");actionName:"dashboardReports";onTriggered:page.app.navigateSection(8)}
   }
   GridLayout {
    Layout.fillWidth:true;columns:width>900?2:1;columnSpacing:12;rowSpacing:12
    Panel {
     title:page.app.trx("جاهزية محطة العمل","Workstation readiness");iconName:"shield_checkmark";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     Repeater {model:toolBridge.tools;delegate:RowLayout {required property var modelData;Layout.fillWidth:true
      Label {text:modelData.name;Layout.fillWidth:true;color:page.app.design.text;textFormat:Text.PlainText}
      StateBadge {text:modelData.available?page.app.trx("متاح","Available"):page.app.trx("إعداد مطلوب","Setup required");tint:modelData.available?page.app.design.success:page.app.design.silver}
     }}
     ActionButton {text:page.app.trx("إدارة الأدوات والتحديثات","Manage local tools");onClicked:page.app.navigateSection(10)}
    }
    Panel {
     title:page.app.trx("آخر نشاط محفوظ","Recent saved activity");iconName:"history";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     ListView {Layout.fillWidth:true;Layout.preferredHeight:Math.min(200,Math.max(44,workshopBridge.events.length*42));clip:true;spacing:8;model:workshopBridge.events
      delegate:Label {required property var modelData;width:ListView.view.width;text:modelData.created_at+" · "+modelData.action;color:page.app.design.muted;textFormat:Text.PlainText;font.pixelSize:12;wrapMode:Text.WordWrap}
     }
     EmptyState {visible:workshopBridge.events.length===0;app:page.app;iconName:"clipboard_task";title:page.app.trx("ابدأ بأمر صيانة","Start with a repair order");description:page.app.trx("سيظهر هنا النشاط المحفوظ بعد إنشاء أمر أو تسجيل نتيجة. لا توجد بيانات عرض مصطنعة.","Saved activity appears after an order or finding is recorded. No fabricated metrics are displayed.");Layout.fillWidth:true}
     ActionButton {text:page.app.trx("أوامر الصيانة والمخزون","Repair orders & inventory");onClicked:page.app.navigateSection(7)}
    }
   }
  }
 }
}
