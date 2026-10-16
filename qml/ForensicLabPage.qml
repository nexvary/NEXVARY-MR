import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:lab
 objectName:"forensicLabWorkspace"
 property var app
 property var modules:[
  {ar:"إدارة القضايا",en:"Case management",ready:true,detail:"Encrypted local case create/open"},
  {ar:"جمع الأدلة",en:"Evidence acquisition",ready:true,detail:"Authorized local file import; not full physical acquisition"},
  {ar:"مستعرض الأدلة",en:"Evidence browser",ready:true,detail:"Case evidence list and export"},
  {ar:"استعادة الملفات",en:"File recovery",ready:true,detail:"Bounded contiguous PNG/JPEG carving"},
  {ar:"تحقيقات Android",en:"Android forensics",ready:false,detail:"Full artifact parsing not implemented"},
  {ar:"تحليل نسخ iOS",en:"iOS backup analysis",ready:false,detail:"No iOS parser installed"},
  {ar:"مؤشرات البرمجيات الضارة",en:"Malware indicators",ready:false,detail:"Forensic indicator engine not implemented"},
  {ar:"الخط الزمني",en:"Timeline analysis",ready:false,detail:"Cross-source timeline not implemented"},
  {ar:"التحقق من البصمة",en:"Hash verification",ready:true,detail:"SHA-256 verification on imported case evidence"},
  {ar:"التقارير",en:"Forensic reporting",ready:true,detail:"Case export with provenance limitations"},
  {ar:"سلسلة الحيازة",en:"Chain of custody",ready:false,detail:"Audit entries exist; signed custody transfers not implemented"},
  {ar:"مدير الأدوات",en:"Tool integration manager",ready:false,detail:"External tool execution not integrated"}
 ]
 ColumnLayout {
  anchors.fill:parent;spacing:10
  Label {text:app.trx("مختبر الأدلة الرقمية — محلي، بتفويض، دون تعديل المصدر","DIGITAL FORENSICS LAB — LOCAL, AUTHORIZED, SOURCE-PRESERVING");font.bold:true;font.pixelSize:17;color:"#8ac7f5";Layout.fillWidth:true;wrapMode:Text.WordWrap}
  Label {text:app.trx("الوحدات المتاحة تستخدم محرك القضايا والاستعادة الحالي. غير المنفذ معطل بوضوح. لا يوجد تجاوز تشفير أو قفل.","Available functions use the existing case/recovery engine. Unimplemented functions are clearly disabled. No encryption or lock bypass.");color:"#adc1d3";wrapMode:Text.WordWrap;Layout.fillWidth:true}
  TabBar {
   id:tabs;Layout.fillWidth:true
   TabButton{text:app.trx("مساحة القضايا والأدلة","Cases and evidence")}
   TabButton{text:app.trx("قدرات المختبر","Lab capabilities")}
  }
  StackLayout {
   currentIndex:tabs.currentIndex;Layout.fillWidth:true;Layout.fillHeight:true
   RecoveryPage {app:lab.app}
   ScrollView {
    clip:true
    GridLayout {
     width:parent.width;columns:2;columnSpacing:12;rowSpacing:12
     Repeater {
      model:lab.modules
      delegate:Rectangle {
       required property var modelData
       Layout.fillWidth:true;Layout.preferredHeight:105;radius:8
       color:"#10283a";border.color:modelData.ready?"#2a786f":"#465160"
       ColumnLayout {
        anchors.fill:parent;anchors.margins:12;spacing:4
        Label{text:app.ar?modelData.ar:modelData.en;color:"#e3effa";font.bold:true;font.pixelSize:15}
        Label{text:modelData.ready?app.trx("متاح ضمن الحدود المذكورة","Available with limitations"):app.trx("غير متاح بعد","Not yet available");color:modelData.ready?"#64dbb0":"#f0bb78";font.pixelSize:12}
        Label{text:modelData.detail;color:"#a7bed0";wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:11}
       }
      }
     }
    }
   }
  }
 }
}
