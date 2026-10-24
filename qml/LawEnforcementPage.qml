import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"lawEnforcementWorkspace"
 property var app
 property int section:0
 property bool auditVerified:false
 readonly property var titles:app.ar?["لوحة القضية","قضية جديدة","فتح قضية","بحث القضايا","أجهزة القضية","السند القانوني","الأدلة والحيازة","تقييم الوصول","التقارير","مختبر التدريب"]:["Case dashboard","New case","Open case","Case search","Case devices","Legal authorization","Evidence & custody","Device access assessment","Reports","Training laboratory"]
 readonly property int viewIndex:[0,1,2,2,3,4,5,6,7,8][section]
 function newCase(){intake.startNew();section=1}
 function editCase(){intake.edit(lawCaseBridge.activeCase.metadata||{});section=1}
 Connections {target:lawCaseBridge;function onChanged(){page.auditVerified=false;caseExportConsent.checked=false}}
 ColumnLayout {anchors.fill:parent;spacing:10
  RowLayout {Layout.fillWidth:true
   ColumnLayout {Layout.fillWidth:true;spacing:3
    Label {text:app.trx("مساحة إنفاذ القانون","Law Enforcement");color:app.design.text;font.bold:true;font.pixelSize:23;Layout.fillWidth:true}
    Label {text:lawCaseBridge.activeCase.id?(lawCaseBridge.activeCase.metadata.caseNumber+" · "+lawCaseBridge.activeCase.status):app.trx("اختر قضية أو ابدأ سيناريو تدريب اصطناعي","Choose a case or a synthetic training scenario");Layout.fillWidth:true;elide:Text.ElideRight;color:app.design.muted;textFormat:Text.PlainText}
   }
   StateBadge {text:lawCaseBridge.role;tint:app.design.silver}
  }
  Label {text:lawCaseBridge.protection;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:lawCaseBridge.ready?app.design.muted:app.design.danger;font.pixelSize:11}
  Label {visible:lawCaseBridge.error!=="";objectName:"lawError";text:lawCaseBridge.error;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger}
  RowLayout {Layout.fillWidth:true
   ComboBox {objectName:"lawSectionSelector";Layout.preferredWidth:Math.min(330,page.width*0.45);model:page.titles;currentIndex:page.section;onActivated:page.section=currentIndex}
   ActionButton {objectName:"lawNewCase";text:app.trx("قضية جديدة","New case");enabled:lawCaseBridge.ready&&lawCaseBridge.permissions.create;onClicked:page.newCase()}
   ActionButton {objectName:"lawOpenCases";text:app.trx("فتح / بحث","Open / search");enabled:lawCaseBridge.ready&&lawCaseBridge.permissions.read;onClicked:{lawCaseBridge.searchCases("");page.section=2}}
   Item {Layout.fillWidth:true}
  }
  StackLayout {Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumWidth:0;currentIndex:page.viewIndex
   ScrollView {id:dashboardScroll;contentWidth:availableWidth;clip:true
    ColumnLayout {width:dashboardScroll.availableWidth;spacing:12
     Panel {Layout.fillWidth:true;title:app.trx("القضية الحالية","Current case");iconName:"document_lock"
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.text;text:lawCaseBridge.activeCase.id?lawCaseBridge.activeCase.metadata.caseNumber+"\n"+lawCaseBridge.activeCase.metadata.investigatingAgency+"\n"+lawCaseBridge.activeCase.metadata.investigatorName:app.trx("لم تفتح قضية بعد. تبقى محاكاة التدريب منفصلة عن القضايا الحقيقية.","No case is open. Training simulation stays separate from real cases.")}
      RowLayout {Layout.fillWidth:true
       ActionButton {objectName:"lawEditCase";text:app.trx("تحرير البيانات","Edit record");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.edit;onClicked:page.editCase()}
       ActionButton {objectName:"lawCloseWorkspace";text:app.trx("إغلاق العرض","Close workspace");enabled:!!lawCaseBridge.activeCase.id;onClicked:lawCaseBridge.closeWorkspace()}
       ActionButton {objectName:"lawCloseCase";text:app.trx("وضع مغلقة","Mark closed");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.close;onClicked:if(lawCaseBridge.transitionCase("closed"))page.section=0}
       ActionButton {objectName:"lawArchiveCase";text:app.trx("أرشفة","Archive");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.archive;onClicked:lawCaseBridge.transitionCase("archived")}
      }
     }
     GridLayout {Layout.fillWidth:true;columns:width>950?3:2;columnSpacing:12;rowSpacing:12
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("الأجهزة المرتبطة","Case devices");iconName:"phone";statusText:lawCaseBridge.devices.length+app.trx(" جهاز"," device(s)");description:app.trx("سجّل المشاهدات والمسؤول والاستلام دون تغيير الهاتف.","Record observations, custodian and receipt without changing the phone.");actionText:app.trx("إدارة الأجهزة","Manage devices");onTriggered:page.section=4}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("السند والنطاق","Authority & scope");iconName:"document_lock";statusText:app.trx("إقرار للمراجعة","Declaration for review");description:app.trx("بيانات الإذن والجهة والمدة؛ لا تصادق المنصة على الصلاحية القانونية.","Authority, issuer and dates; the platform does not authenticate legal entitlement.");actionText:app.trx("مراجعة البيانات","Review declaration");onTriggered:page.section=5}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("سجل التدقيق","Audit trail");iconName:"history";statusText:lawCaseBridge.audit.length+app.trx(" حدث"," event(s)");description:app.trx("راجع الأحداث المحمية وترابطها؛ لا يعني ذلك توقيعًا مستقلًا أو تسليمًا ماديًا.","Inspect protected linked events; not independent signatures or physical handover proof.");actionText:app.trx("عرض التدقيق","Review audit");onTriggered:page.section=6}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("تقييم الوصول","Access assessment");iconName:"shield_checkmark";description:app.trx("حدود الواجهات والتشفير والقفل، دون استغلال الحماية.","Interface, encryption and lock limits without protection exploitation.");statusText:app.trx("تقييم فقط","Assessment only");actionText:app.trx("عرض القيود والمسارات","Review scope & constraints");onTriggered:page.section=7}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("التقارير","Reports");iconName:"document_text";description:app.trx("ملخص PDF قابل للمراجعة؛ تصدير غير مشفر بموافقة صريحة.","Reviewable PDF summary; plaintext export requires explicit approval.");statusText:app.trx("موافقة مطلوبة","Consent required");actionText:app.trx("فتح التقارير","Open reports");onTriggered:page.section=8}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("المختبر التدريبي","Training laboratory");iconName:"book_open_lightbulb";description:app.trx("قرارات وتشخيصات افتراضية مع تقييم وتفسير؛ دون أجهزة حقيقية.","Virtual decisions with scoring and explanations; no real devices.");statusText:app.trx("اصطناعي فقط","Synthetic only");actionText:app.trx("بدء التدريب","Start training");onTriggered:page.section=9}
     }
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;font.pixelSize:11;text:app.trx("نموذج محلي تجريبي للحساب الحالي؛ تطبيق الصلاحيات في API لا يستبدل تهيئة هويات وأدوار الجهة المؤسسية. بوابة الهوية المؤسسية لم تكتمل.","Experimental current-account prototype. API role policy does not replace authenticated agency identities and provisioning; that deployment gate remains incomplete.")}
    }
   }
   LawCaseForm {id:intake;objectName:"lawCaseForm";app:page.app;onSaved:page.section=0}
   LawCaseBrowser {app:page.app;onOpened:page.section=0}
   LawCaseDevices {app:page.app}
   ScrollView {id:legalScroll;contentWidth:availableWidth;clip:true
    Panel {width:legalScroll.availableWidth;title:app.trx("بيانات السند القانوني المسجلة","Recorded legal authority");iconName:"document_lock"
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("بيانات مسجلة للمراجعة المختصة، وليست مصادقة آلية. لا تمنح قراءة هذه البيانات إذنًا لتشغيل إجراء على جهاز.","Recorded declarations for competent review, not automatic validation or device-operation permission.")}
     Repeater {model:["authorityReference","issuingAuthority","prosecutorName","prosecutorOffice","courtName","validFromUtc","validUntilUtc","scope","jurisdiction"]
      delegate:Label {required property string modelData;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:intake.label(modelData)+": "+((lawCaseBridge.activeCase.metadata||{})[modelData]||"—")}
     }
     ActionButton {text:app.trx("تحرير الإقرار","Edit declaration");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.edit;onClicked:page.editCase()}
    }
   }
   ScrollView {id:auditScroll;contentWidth:availableWidth;clip:true
    ColumnLayout {width:auditScroll.availableWidth;spacing:12
     Panel {Layout.fillWidth:true;title:app.trx("سلامة سجل القضية","Case audit integrity");iconName:"history"
      ActionButton {objectName:"lawVerifyAudit";text:app.trx("التحقق من ترابط السجل","Verify linked audit");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.audit;onClicked:page.auditVerified=lawCaseBridge.verifyAudit()}
      Label {objectName:"lawAuditResult";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:page.auditVerified?app.trx("نجح تحقق السجل المحلي. لا يثبت تسليمًا ماديًا أو مقاومة استرجاع نسخة قاعدة قديمة.","Local linked-record verification passed. It does not prove physical handover or resistance to whole-database rollback."):app.trx("لم يتم التحقق في هذا العرض بعد.","Not verified in this view yet.")}
      Repeater {model:lawCaseBridge.audit;delegate:Label {required property var modelData;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(modelData)}}
     }
     LawEvidencePage {Layout.fillWidth:true;Layout.preferredHeight:900;app:page.app}
    }
   }
   ScrollView {id:accessScroll;contentWidth:availableWidth;clip:true
    ColumnLayout {width:accessScroll.availableWidth;spacing:12
     Repeater {model:lawCaseBridge.accessAssessment;delegate:Panel {required property var modelData;Layout.fillWidth:true;title:page.app.ar?modelData.ar:modelData.en;iconName:"shield_checkmark";stateText:modelData.state
      Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:page.app.design.muted;text:modelData.fact}
     }}
    }
   }
   ScrollView {id:reportsScroll;contentWidth:availableWidth;clip:true
    ColumnLayout {width:reportsScroll.availableWidth;spacing:12
     Panel {title:app.trx("تقرير مراجعة القضية PDF","Case review PDF");iconName:"document_text";Layout.fillWidth:true
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("يتضمن الإقرار القانوني والأجهزة المسجلة وسجل التدقيق وحدود النموذج. لا يمثل تقرير استخلاص كاملًا أو اعتمادًا قضائيًا.","Contains recorded authority, registered devices, audit and prototype limits. Not a complete acquisition report or judicial accreditation.")}
      CheckBox {id:caseExportConsent;objectName:"lawReportConsent";text:app.trx("أوافق على تصدير بيانات القضية غير مشفرة","Approve plaintext case-data export")}
      ActionButton {objectName:"lawExportPdf";text:app.trx("حفظ PDF جديد","Save new PDF");primary:true;enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.report&&caseExportConsent.checked;onClicked:casePdf.open()}
     }
     Panel {title:app.trx("تقرير التدريب الاصطناعي","Synthetic training report");iconName:"book_open_lightbulb";Layout.fillWidth:true
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:lawCaseBridge.trainingAttempts.length+app.trx(" محاولة؛ لا يتضمن بيانات قضايا حقيقية."," attempts; contains no real case data.")}
      CheckBox {id:trainingExportConsent;text:app.trx("أوافق على حفظ تقرير التدريب","Approve training report export")}
      ActionButton {objectName:"lawExportTraining";text:app.trx("حفظ JSON تدريبي","Save training JSON");enabled:lawCaseBridge.permissions.training&&trainingExportConsent.checked;onClicked:trainingReport.open()}
     }
    }
   }
   LawTrainingPage {objectName:"lawTrainingPage";app:page.app}
  }
 }
 FileDialog {id:casePdf;fileMode:FileDialog.SaveFile;defaultSuffix:"pdf";nameFilters:["PDF (*.pdf)"];onAccepted:lawCaseBridge.exportCasePdf(selectedFile.toString(),page.app.ar,caseExportConsent.checked)}
 FileDialog {id:trainingReport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:lawCaseBridge.exportTrainingReport(selectedFile.toString(),trainingExportConsent.checked)}
}
