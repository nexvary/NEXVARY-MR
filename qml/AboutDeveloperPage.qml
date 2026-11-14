import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 objectName:"aboutDeveloperPage";contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:14
  Panel {Layout.fillWidth:true;title:page.app.trx("عن المطوّر — Nexvary Inc.","About the developer — Nexvary Inc.");iconName:"building"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:18;color:page.app.design.text;text:page.app.trx("شركة لتطوير البرمجيات والأمن السيبراني والذكاء الاصطناعي والبحث التقني. ولاية التأسيس: وايومنغ، الولايات المتحدة؛ سنة التأسيس: 2026.","A software, cybersecurity, artificial intelligence and technology research company. Incorporation jurisdiction: Wyoming, United States; incorporation year: 2026.")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;text:page.app.trx("تطوّر الشركة NEXVARY MR للعمليات والاستخبارات الرقمية، مع توثيق حدود القدرات والاختبارات. الشركة مطوّر تقني؛ ليست جهة إنفاذ قانون، ولا يعني استخدام النظام اعتمادًا حكوميًا.","Nexvary Inc. develops NEXVARY MR for digital operations and intelligence, with documented capability and validation limits. It is a technology developer, not a law-enforcement body; use does not imply government endorsement.")}
   Label {text:"info@nexvary.com";color:page.app.design.accent;Layout.fillWidth:true;textFormat:Text.PlainText}
   EntryField {objectName:"companyContactEmail";text:"info@nexvary.com";readOnly:true;requiredInput:false;Layout.maximumWidth:320;Layout.fillWidth:true}
   Flow {Layout.fillWidth:true;spacing:10
    ActionButton {text:page.app.trx("الموقع الرسمي","Official website");onClicked:page.app.openExternal("https://nexvary.com/")}
    ActionButton {text:page.app.trx("التواصل مع الشركة","Contact the company");onClicked:page.app.openExternal("https://nexvary.com/contact")}
    ActionButton {text:"GitHub · Nexvary";onClicked:page.app.openExternal("https://github.com/nexvary")}
   }
  }
  Panel {Layout.fillWidth:true;title:page.app.trx("الملكية والتراخيص","Ownership & licenses");iconName:"document_text"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("© 2026 Nexvary Inc. جميع حقوق مكونات الشركة محفوظة. تخضع مكونات الأطراف الأخرى لتراخيصها الخاصة، وتبقى حقوقها وإشعاراتها محفوظة. لا يعني دمج خدمة أو مكوّن ملكية الشركة له.","© 2026 Nexvary Inc. Company-owned components are proprietary. Third-party components remain subject to their respective licenses, rights and notices. Integration does not imply ownership of third-party services or software.")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.muted;text:page.app.trx("الخط العربي: Noto Kufi Arabic — SIL Open Font License 1.1. الأيقونات المكتبية: Fluent؛ إشعارات المكونات والتراخيص مضمّنة ضمن الحزمة.","Arabic font: Noto Kufi Arabic — SIL Open Font License 1.1. Desktop icons: Fluent; component license notices are included in the package.")}
  }
 }
}
