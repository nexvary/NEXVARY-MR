import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
 id:window
 objectName:"mainWindow"
 width:1536;height:980;minimumWidth:840;minimumHeight:480;visible:true
 title:toolBridge.laboratoryBuild?"NEXVARY MR — LABORATORY / EXPERIMENTAL":"NEXVARY MR — Government Edition"
 property bool ar:true
 property bool darkTheme:true
 readonly property QtObject design:tokens
 DesignTokens {id:tokens;dark:window.darkTheme}
 LegacyPalette {id:legacy;dark:window.darkTheme}
 function tone(value){return legacy.tone(value)}
 color:design.background
 background:Rectangle {color:window.design.background;gradient:Gradient {GradientStop {position:0;color:window.darkTheme?"#171D25":"#F7FAFF"}GradientStop {position:0.4;color:window.design.background}GradientStop {position:1;color:window.darkTheme?"#10141A":"#E5ECF5"}}}
 property bool showDevicePanel:false
 property bool governmentMode:true
 property int currentPage:12
 property int activeSection:0
 property var pageHistory:[]
 property int previousPage:0
 property bool goingBack:false
 property bool allowClose:false
 property string toastText:""
 property bool toastOk:true
 property var states:["Received","Diagnosing","Awaiting parts","Repairing","Ready","Delivered","Cancelled"]
 property var statesAr:["تم الاستلام","التشخيص","انتظار القطع","قيد الإصلاح","جاهز","تم التسليم","ملغي"]
 // Legacy page numbers stay stable for native smoke consumers and page APIs.
 property var titles:ar?["لوحة المتابعة","مدير الأجهزة","استوديو الصيانة","مخططات اللوحات","ذكاء ملفات النظام","إدارة الورشة","قطع الغيار","الإعدادات","الاستعادة والنسخ الاحتياطي","مختبر الأدلة الرقمية","التقارير","المساعدة وعن البرنامج","إنفاذ القانون","مركز الأبحاث"]:["Dashboard","Device Manager","Repair Studio","Board workspace","Firmware Intelligence","Workshop CRM","Parts inventory","Settings","Recovery & Backup","Forensic Lab","Reports","Help & About","Law Enforcement","Research Center"]
 readonly property var professionalSections:[
  {ar:"لوحة المتابعة",en:"Dashboard",icon:"home",page:0},
  {ar:"مدير الأجهزة",en:"Device Manager",icon:"phone",page:1},
  {ar:"استوديو الصيانة",en:"Repair Studio",icon:"toolbox",page:2},
  {ar:"ذكاء ملفات النظام",en:"Firmware Intelligence",icon:"arrow_square_down",page:4},
  {ar:"تشخيص USB",en:"USB Diagnostics",icon:"phone_laptop",page:2,tab:3},
  {ar:"المختبر الجنائي",en:"Forensic Laboratory",icon:"document_lock",page:9},
  {ar:"الاستعادة والنسخ",en:"Recovery & Backup",icon:"history",page:8},
  {ar:"إدارة الورشة",en:"Workshop CRM",icon:"clipboard_task",page:5},
  {ar:"التقارير",en:"Reports",icon:"document_text",page:10},
  {ar:"مركز الأبحاث",en:"Research Center",icon:"book_open_lightbulb",page:13},
  {ar:"الإعدادات",en:"Settings",icon:"settings",page:7},
  {ar:"المساعدة وعن البرنامج",en:"Help & About",icon:"question_circle",page:11},
  {ar:"إنفاذ القانون",en:"Law Enforcement",icon:"document_lock",page:12}
 ]
 readonly property var governmentSections:[
  {ar:"لوحة التحكم الحكومية",en:"Government Dashboard",icon:"home",page:12,law:0},
  {ar:"فتح قضية",en:"Open Case",icon:"document_lock",page:12,law:2},
  {ar:"إدارة القضايا",en:"Case Management",icon:"clipboard_task",page:12,law:3},
  {ar:"تقييم الأجهزة",en:"Device Assessment",icon:"phone",page:12,law:7},
  {ar:"إدارة الأدلة",en:"Evidence Management",icon:"document_lock",page:12,law:6,evidence:1},
  {ar:"سلسلة الحيازة",en:"Chain of Custody",icon:"history",page:12,law:6,evidence:2},
  {ar:"التحليل الجنائي",en:"Forensic Analysis",icon:"shield_checkmark",page:14},
  {ar:"مركز الأبحاث",en:"Research Center",icon:"book_open_lightbulb",page:13},
  {ar:"مختبر التدريب",en:"Training Laboratory",icon:"book_open_lightbulb",page:12,law:9},
  {ar:"التقارير والتدقيق",en:"Reports & Audit",icon:"document_text",page:12,law:8},
  {ar:"الإدارة",en:"Administration",icon:"settings",page:12,law:10},
  {ar:"الصيانة الاحترافية",en:"Professional Repair",icon:"toolbox",page:0}
 ]
 readonly property var sections:governmentMode?governmentSections:professionalSections
 function setWorkspace(government){governmentMode=government;pageHistory=[];navigateSection(0)}
 readonly property string pageTitle:currentPage===3?titles[3]:currentPage===6?titles[6]:(ar?sections[activeSection].ar:sections[activeSection].en)
 function trx(a,e){return ar?a:e}
 function orderStatus(s){let i=states.indexOf(s);return ar&&i>=0?statesAr[i]:s}
 function notice(ok,message,errorMessage){toastOk=ok;toastText=ok?message:(errorMessage===undefined?workshopBridge.error:errorMessage);toastTimer.restart()}
 function sectionForPage(page){if(!governmentMode)return [0,1,2,2,3,7,7,10,6,5,8,11,12,9][page]||0;for(let i=0;i<sections.length;i++)if(sections[i].page===page)return i;return 11}
 function snapshot(page){return {page:page,section:activeSection,deviceTab:deviceHub.currentTab,diagnosticTab:diagnosticHub.currentTab,forensicTab:forensicHub.currentTab,workshopTab:workshopHub.currentTab,lawSection:lawWorkspace.section,evidenceView:lawWorkspace.evidenceView}}
 onCurrentPageChanged:{
  if(!goingBack&&currentPage!==previousPage){let history=pageHistory.slice();history.push(snapshot(previousPage));pageHistory=history.slice(-50)}
  previousPage=currentPage;activeSection=sectionForPage(currentPage)
 }
 function navigate(index){currentPage=index}
 function navigateSection(index){
  const entry=sections[index];if(!entry)return
  if(currentPage===entry.page&&activeSection!==index){let history=pageHistory.slice();history.push(snapshot(currentPage));pageHistory=history.slice(-50)}
  currentPage=entry.page
  if(entry.law!==undefined)lawWorkspace.section=entry.law
  if(entry.evidence!==undefined)lawWorkspace.evidenceView=entry.evidence
  if(entry.page===1)deviceHub.currentTab=entry.tab===undefined?0:entry.tab
  if(entry.page===2)diagnosticHub.currentTab=entry.tab===undefined?0:entry.tab
  activeSection=index
 }
 function goBack(){
  if(pageHistory.length===0)return
  let history=pageHistory.slice();const target=history.pop();pageHistory=history;goingBack=true
  currentPage=typeof target==="number"?target:target.page
  if(typeof target!=="number"){deviceHub.currentTab=target.deviceTab;diagnosticHub.currentTab=target.diagnosticTab;forensicHub.currentTab=target.forensicTab||0;workshopHub.currentTab=target.workshopTab||0;lawWorkspace.section=target.lawSection||0;lawWorkspace.evidenceView=target.evidenceView||1;activeSection=target.section}
  goingBack=false
 }
 function navigateServices(){currentPage=1;deviceHub.currentTab=1;activeSection=2}
 function navigateEvidence(){currentPage=9;forensicHub.currentTab=1;activeSection=5}
 function navigateSecurity(){currentPage=2;diagnosticHub.currentTab=2;activeSection=2}
 function sectionButton(index){return shell.sectionButton(index)}
 function navigationButton(legacyIndex){return sectionButton([0,1,2,2,3,7,7,10,6,5][legacyIndex]||0)}
 palette.window:design.background;palette.windowText:design.text;palette.base:design.surface;palette.alternateBase:design.raised;palette.text:design.text;palette.button:design.raised;palette.buttonText:design.text;palette.highlight:design.accent;palette.highlightedText:"white";palette.mid:design.border;palette.dark:design.background;palette.light:design.border;palette.placeholderText:design.muted
 font.family:"Segoe UI";font.pixelSize:14
 LayoutMirroring.enabled:ar
 LayoutMirroring.childrenInherit:true
 Shortcut {sequence:"Ctrl+1";onActivated:window.navigateSection(0)}
 Shortcut {sequence:"Ctrl+2";onActivated:window.navigateSection(1)}
 Shortcut {sequence:"Ctrl+3";onActivated:window.navigateSection(2)}
 Shortcut {sequence:"Ctrl+4";onActivated:window.navigateSection(3)}
 Shortcut {sequence:"Ctrl+5";onActivated:window.navigateSection(4)}
 Shortcut {sequence:"Ctrl+6";onActivated:window.navigateSection(5)}
 Shortcut {sequence:"Ctrl+7";onActivated:window.navigateSection(6)}
 Shortcut {sequence:"Ctrl+8";onActivated:window.navigateSection(7)}
 Shortcut {sequence:"Ctrl+9";onActivated:window.navigateSection(8)}
 Shortcut {sequence:"Ctrl+R";onActivated:if(!toolBridge.busy)toolBridge.scanAdb()}
 Shortcut {sequence:"Ctrl+Q";onActivated:window.close()}
 onClosing:function(close){if(!allowClose){close.accepted=false;exitDialog.open()}}
 Dialog {
  id:exitDialog;width:Math.min(480,window.width-32);objectName:"exitDialog";anchors.centerIn:parent;modal:true;title:window.trx("إغلاق NEXVARY MR","Close NEXVARY MR");standardButtons:Dialog.Yes|Dialog.No
  Label {width:exitDialog.availableWidth;wrapMode:Text.WordWrap;color:window.design.text;text:window.trx("هل تريد الإغلاق؟ ستُلغى الفحوص الجارية. احفظ جلسات المخططات والنماذج المفتوحة أولًا.","Close the application? Running checks will be cancelled. Save board sessions and open forms first.")}
  onAccepted:{lawCaseBridge.cancelImport();toolBridge.cancel();firmwareBridge.cancel();recoveryBridge.cancel();securityBridge.cancel();window.allowClose=true;window.close()}
 }
 AppShell {
  id:shell;anchors.fill:parent;app:window
  ColumnLayout {
   anchors.fill:parent;spacing:8
   RowLayout {
    visible:window.activeSection===2||window.activeSection===7;Layout.fillWidth:true
    ActionButton {visible:window.activeSection===2;text:window.trx("التشخيص","Diagnostics");implicitHeight:32;onClicked:window.navigateSection(2)}
    ActionButton {visible:window.activeSection===2;text:window.trx("خدمات الجهاز","Device services");implicitHeight:32;onClicked:window.navigateServices()}
    ActionButton {visible:window.activeSection===2;text:window.trx("مخططات اللوحات","Board workspace");implicitHeight:32;onClicked:{window.currentPage=3;window.activeSection=2}}
    ActionButton {visible:window.activeSection===7;text:window.trx("أوامر الصيانة","Repair orders");implicitHeight:32;onClicked:window.navigateSection(7)}
    ActionButton {visible:window.activeSection===7;text:window.trx("قطع الغيار","Parts inventory");implicitHeight:32;onClicked:{window.currentPage=6;window.activeSection=7}}
    Item {Layout.fillWidth:true}
   }
   StackLayout {
    id:pages;Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0;currentIndex:window.currentPage
    DashboardPage {app:window}
    DeviceHub {id:deviceHub;app:window}
    DiagnosticHub {id:diagnosticHub;app:window}
    BoardWorkspace {app:window}
    FirmwarePage {app:window}
    WorkshopHub {id:workshopHub;app:window}
    InventoryPage {app:window}
    SettingsPage {app:window}
    RecoveryPage {app:window}
    ForensicLabPage {id:forensicHub;app:window}
    ReportsPage {app:window}
    HelpPage {app:window}
    LawEnforcementPage {id:lawWorkspace;app:window}
    ResearchCenter {app:window}
    GovernmentAnalysisPage {app:window}
   }
  }
 }
 Rectangle {
  anchors.horizontalCenter:parent.horizontalCenter;anchors.bottom:parent.bottom;anchors.bottomMargin:76;width:Math.min(680,parent.width-40);height:toastLabel.implicitHeight+28;radius:8;visible:toastText!=="";color:window.design.raised;border.color:toastOk?window.design.success:window.design.danger;z:100
  Label {id:toastLabel;anchors.fill:parent;anchors.margins:14;text:window.toastText;color:window.design.text;wrapMode:Text.WordWrap;textFormat:Text.PlainText;verticalAlignment:Text.AlignVCenter}
 }
 Timer {id:toastTimer;interval:6000;onTriggered:window.toastText=""}
}
