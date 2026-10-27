import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"lawEnforcementWorkspace"
 property var app
 property int section:0
 property int evidenceView:1
 property bool auditVerified:false
 property bool anchorVerified:false
 property var lastBackup:({})
 function saveProtectedBackup(path){lastBackup=lawCaseBridge.exportProtectedBackup(path,backupConsent.checked)}
 function restoreProtectedBackup(path){const id=lawCaseBridge.restoreProtectedBackup(path,restoreDigest.text,restoreConsent.checked);if(id!==""){restoreDigest.text="";page.section=0}return id}
 readonly property var titles:app.ar?["لوحة القضية","قضية جديدة","فتح قضية","بحث القضايا","أجهزة القضية","السند القانوني","الأدلة والحيازة","تقييم الوصول","التقارير","مختبر التدريب","الحسابات والصلاحيات","الاحتفاظ والأرشفة"]:["Case dashboard","New case","Open case","Case search","Case devices","Legal authorization","Evidence & custody","Device access assessment","Reports","Training laboratory","Accounts and permissions","Retention & archiving"]
 readonly property int viewIndex:[0,1,2,2,3,4,5,6,7,8,9,10][section]
 function newCase(){intake.startNew();section=1}
 function editCase(){intake.edit(lawCaseBridge.activeCase.metadata||{});section=1}
 Connections {target:lawCaseBridge;function onChanged(){page.auditVerified=false;page.anchorVerified=false;caseExportConsent.checked=false;backupConsent.checked=false;restoreConsent.checked=false;page.lastBackup=({})}}
 Column {anchors.fill:parent;spacing:10
  // The viewport owns height; wrapped content cannot resize its parent.
  RowLayout {width:parent.width
   ColumnLayout {Layout.fillWidth:true;spacing:3
    Label {text:app.trx("مساحة إنفاذ القانون","Law Enforcement");color:app.design.text;font.bold:true;font.pixelSize:23;Layout.fillWidth:true}
    Label {text:lawCaseBridge.activeCase.id?(lawCaseBridge.activeCase.metadata.caseNumber+" · "+lawCaseBridge.activeCase.status):app.trx("اختر قضية أو ابدأ سيناريو تدريب اصطناعي","Choose a case or a synthetic training scenario");Layout.fillWidth:true;elide:Text.ElideRight;color:app.design.muted;textFormat:Text.PlainText}
   }
   ActionButton {text:app.trx("الدخول / الحسابات","Sign in / accounts");onClicked:page.section=10}
   StateBadge {text:lawCaseBridge.role;tint:app.design.silver}
  }
  Label {text:lawCaseBridge.protection;width:parent.width;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:lawCaseBridge.ready?app.design.muted:app.design.danger;font.pixelSize:11}
  Label {visible:lawCaseBridge.error!=="";objectName:"lawError";text:lawCaseBridge.error;width:parent.width;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger}
  RowLayout {width:parent.width
   ComboBox {id:sectionSelector;objectName:"lawSectionSelector";onModelChanged:Qt.callLater(function(){sectionSelector.currentIndex=Qt.binding(function(){return page.section})});Layout.preferredWidth:Math.min(330,page.width*0.45);model:page.titles;currentIndex:page.section;onActivated:page.section=currentIndex}
   ActionButton {objectName:"lawNewCase";text:app.trx("قضية جديدة","New case");enabled:lawCaseBridge.ready&&lawCaseBridge.permissions.create;onClicked:page.newCase()}
   ActionButton {objectName:"lawOpenCases";text:app.trx("فتح / بحث","Open / search");enabled:lawCaseBridge.ready&&lawCaseBridge.permissions.read;onClicked:{lawCaseBridge.searchCases("");page.section=2}}
   Item {Layout.fillWidth:true}
  }
  StackLayout {width:parent.width;height:Math.max(0,page.height-y);implicitWidth:0;implicitHeight:0;currentIndex:page.viewIndex
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
     ActionButton {objectName:"lawRetention";text:app.trx("الاحتفاظ والأرشفة","Retention & archiving");enabled:!!lawCaseBridge.activeCase.id;onClicked:page.section=11}
     GridLayout {Layout.fillWidth:true;columns:width>950?3:2;columnSpacing:12;rowSpacing:12
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("الأجهزة المرتبطة","Case devices");iconName:"phone";statusText:lawCaseBridge.devices.length+app.trx(" جهاز"," device(s)");description:app.trx("سجّل المشاهدات والمسؤول والاستلام دون تغيير الهاتف.","Record observations, custodian and receipt without changing the phone.");actionText:app.trx("إدارة الأجهزة","Manage devices");onTriggered:page.section=4}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("السند والنطاق","Authority & scope");iconName:"document_lock";statusText:app.trx("إقرار للمراجعة","Declaration for review");description:app.trx("بيانات الإذن والجهة والمدة؛ لا تصادق المنصة على الصلاحية القانونية.","Authority, issuer and dates; the platform does not authenticate legal entitlement.");actionText:app.trx("مراجعة البيانات","Review declaration");onTriggered:page.section=5}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("سجل التدقيق","Audit trail");iconName:"history";statusText:lawCaseBridge.audit.length+app.trx(" حدث"," event(s)");description:app.trx("راجع الأحداث المحمية وترابطها؛ لا يعني ذلك توقيعًا مستقلًا أو تسليمًا ماديًا.","Inspect protected linked events; not independent signatures or physical handover proof.");actionText:app.trx("عرض التدقيق","Review audit");onTriggered:{page.evidenceView=3;page.section=6}}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("تقييم الوصول","Access assessment");iconName:"shield_checkmark";description:app.trx("حدود الواجهات والتشفير والقفل، دون استغلال الحماية.","Interface, encryption and lock limits without protection exploitation.");statusText:app.trx("تقييم فقط","Assessment only");actionText:app.trx("عرض القيود والمسارات","Review scope & constraints");onTriggered:page.section=7}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("التقارير","Reports");iconName:"document_text";description:app.trx("ملخص PDF قابل للمراجعة؛ تصدير غير مشفر بموافقة صريحة.","Reviewable PDF summary; plaintext export requires explicit approval.");statusText:app.trx("موافقة مطلوبة","Consent required");actionText:app.trx("فتح التقارير","Open reports");onTriggered:page.section=8}
      ActionCard {app:page.app;Layout.fillWidth:true;title:app.trx("المختبر التدريبي","Training laboratory");iconName:"book_open_lightbulb";description:app.trx("قرارات وتشخيصات افتراضية مع تقييم وتفسير؛ دون أجهزة حقيقية.","Virtual decisions with scoring and explanations; no real devices.");statusText:app.trx("اصطناعي فقط","Synthetic only");actionText:app.trx("بدء التدريب","Start training");onTriggered:page.section=9}
     }
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;font.pixelSize:11;text:app.trx("حسابات محلية مصادق عليها ومنح منفصلة للقضايا. يبقى التحقق من الهوية المؤسسية وربط SSO ضمن متطلبات النشر.","Local authenticated accounts and case grants. Agency identity verification and SSO remain deployment requirements.")}
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
   ScrollView {id:auditScroll;objectName:"lawAuditScroll";contentWidth:availableWidth;clip:true
    ColumnLayout {width:auditScroll.availableWidth;spacing:12
     RowLayout {Layout.fillWidth:true
      ActionButton {text:app.trx("الأدلة","Evidence");onClicked:page.evidenceView=1}
      ActionButton {text:app.trx("سلسلة الحيازة","Chain of custody");onClicked:page.evidenceView=2}
      ActionButton {text:app.trx("سجل التدقيق","Audit trail");onClicked:page.evidenceView=3}
     }
     Panel {visible:page.evidenceView===3;Layout.fillWidth:true;title:app.trx("سلامة سجل القضية","Case audit integrity");iconName:"history"
      ActionButton {objectName:"lawVerifyAudit";text:app.trx("التحقق من ترابط السجل","Verify linked audit");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.audit;onClicked:page.auditVerified=lawCaseBridge.verifyAudit()}
      Label {objectName:"lawAuditResult";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:page.auditVerified?app.trx("نجح تحقق السجل المحلي. لا يثبت تسليمًا ماديًا أو مقاومة استرجاع نسخة قاعدة قديمة.","Local linked-record verification passed. It does not prove physical handover or resistance to whole-database rollback."):app.trx("لم يتم التحقق في هذا العرض بعد.","Not verified in this view yet.")}
      CheckBox {id:anchorConsent;text:app.trx("أوافق على تصدير مرجع توقيع محلي للاحتفاظ به منفصلًا","Approve local signed anchor export for separate retention")}
      RowLayout {Layout.fillWidth:true
       ActionButton {text:app.trx("تصدير مرجع موقّع","Export signed anchor");enabled:identityBridge.authenticated&&!!lawCaseBridge.activeCase.id&&anchorConsent.checked;onClicked:anchorExport.open()}
       ActionButton {text:app.trx("مقارنة مرجع محفوظ","Compare retained anchor");enabled:identityBridge.authenticated&&!!lawCaseBridge.activeCase.id;onClicked:anchorInput.open()}
      }
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:page.anchorVerified?app.trx("تطابق السجل مع المرجع المحلي المحفوظ. لا يثبت توقيتًا خارجيًا أو تسليمًا ماديًا.","History matches the retained local anchor. This does not establish external time or physical handover."):app.trx("لم يُتحقق من مرجع منفصل بعد. احتفظ به خارج قاعدة القضية؛ المرجع الأقدم لا يكشف حذف أحداث أحدث منه.","No separate anchor verified. Retain anchors outside the case database; an older anchor cannot detect removal of events newer than itself.")}
      Repeater {model:lawCaseBridge.audit;delegate:Label {required property var modelData;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(modelData)}}
     }
     LawEvidencePage {visible:page.evidenceView!==3;custodyOnly:page.evidenceView===2;Layout.fillWidth:true;Layout.preferredHeight:900;app:page.app}
    }
   }
   ScrollView {id:accessScroll;contentWidth:availableWidth;clip:true
    ColumnLayout {width:accessScroll.availableWidth;spacing:12
     Repeater {model:lawCaseBridge.accessAssessment;delegate:Panel {required property var modelData;Layout.fillWidth:true;title:page.app.ar?modelData.ar:modelData.en;iconName:"shield_checkmark";stateText:modelData.state
      Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:page.app.design.muted;text:modelData.fact}
     }}
    }
   }
   ScrollView {id:reportsScroll;objectName:"lawReportsScroll";contentWidth:availableWidth;clip:true
    ColumnLayout {width:reportsScroll.availableWidth;spacing:12
     ActionButton {text:app.trx("فتح سجل التدقيق","Open audit trail");onClicked:{page.evidenceView=3;page.section=6}}
     Panel {title:app.trx("تقرير مراجعة القضية PDF","Case review PDF");iconName:"document_text";Layout.fillWidth:true
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("يتضمن الإقرار القانوني والأجهزة المسجلة وسجل التدقيق وحدود النموذج. لا يمثل تقرير استخلاص كاملًا أو اعتمادًا قضائيًا.","Contains recorded authority, registered devices, audit and prototype limits. Not a complete acquisition report or judicial accreditation.")}
      CheckBox {id:caseExportConsent;objectName:"lawReportConsent";text:app.trx("أوافق على تصدير بيانات القضية غير مشفرة","Approve plaintext case-data export")}
      ActionButton {objectName:"lawExportPdf";text:app.trx("حفظ PDF جديد","Save new PDF");primary:true;enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.report&&caseExportConsent.checked;onClicked:casePdf.open()}
     }
     Panel {objectName:"lawProtectedBackupPanel";title:app.trx("نسخة قضية محمية واستعادة","Protected case backup & restore");iconName:"document_lock";Layout.fillWidth:true
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تحفظ نسخة الأدلة المحمية دون مفاتيح حسابات البرنامج. تعتمد الاستعادة على سياق حماية Windows الأصلي وسجل الهوية ومنح القضية. الحد 32 MiB للمحتوى المحمي و64 MiB للحاوية. احتفظ بالبصمة ومرجع التدقيق منفصلين.","Preserves protected case/evidence blobs without account signing keys. Recovery requires the original Windows protection context, identity registry and case grants. Limits: 32 MiB protected payloads, 64 MiB container. Retain the digest and audit anchor separately.")}
      CheckBox {id:backupConsent;objectName:"lawBackupConsent";text:app.trx("أوافق على حفظ نسخة محمية لهذه القضية","Approve a protected backup of this case")}
      ActionButton {objectName:"lawSaveProtectedBackup";text:app.trx("حفظ نسخة محمية جديدة","Save new protected backup");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.backup&&backupConsent.checked;onClicked:protectedBackupOutput.open()}
      TextArea {objectName:"lawBackupDigest";visible:!!page.lastBackup.sha256;Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;color:app.design.success;text:page.lastBackup.sha256||""}
      Repeater {model:lawCaseBridge.activeCase.backups||[];delegate:TextArea {required property var modelData;readOnly:true;selectByMouse:true;Layout.fillWidth:true;wrapMode:TextEdit.WrapAnywhere;textFormat:TextEdit.PlainText;color:page.app.design.muted;text:modelData.atUtc+" · "+modelData.sourceRevision+"\nSHA-256: "+modelData.sha256}}
      TextField {id:restoreDigest;objectName:"lawRestoreDigest";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("بصمة النسخة المحفوظة SHA-256: 64 حرفًا","Separately retained backup SHA-256: 64 characters")}
      CheckBox {id:restoreConsent;objectName:"lawRestoreConsent";text:app.trx("أوافق على الاستعادة دون استبدال أي قضية","Approve restoration without replacing an existing case")}
      ActionButton {objectName:"lawRestoreProtectedBackup";text:app.trx("اختيار نسخة محمية للاستعادة","Choose protected backup to restore");enabled:lawCaseBridge.permissions.restore&&/^[0-9a-fA-F]{64}$/.test(restoreDigest.text)&&restoreConsent.checked;onClicked:protectedBackupInput.open()}
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تحتفظ الاستعادة بمعرّف القضية والمالك والأصول، وتضيف حدث استعادة جديدًا. قارن بمرجع التدقيق المحفوظ لكشف تاريخ أحدث أو متعارض. هذه ليست استعادة شاملة لمفاتيح الحساب أو النظام.","Restoration retains case ID, owner and originals and appends a recovery event. Compare a retained audit anchor to detect newer/divergent history. This is not full account-key or operating-system disaster recovery.")}
     }
     Panel {title:app.trx("تقرير التدريب الاصطناعي","Synthetic training report");iconName:"book_open_lightbulb";Layout.fillWidth:true
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:lawCaseBridge.trainingAttempts.length+app.trx(" محاولة؛ لا يتضمن بيانات قضايا حقيقية."," attempts; contains no real case data.")}
      CheckBox {id:trainingExportConsent;text:app.trx("أوافق على حفظ تقرير التدريب","Approve training report export")}
      ActionButton {objectName:"lawExportTraining";text:app.trx("حفظ JSON تدريبي","Save training JSON");enabled:lawCaseBridge.permissions.training&&trainingExportConsent.checked;onClicked:trainingReport.open()}
     }
    }
   }
   LawTrainingPage {objectName:"lawTrainingPage";app:page.app}
   IdentityPanel {app:page.app}
   LawRetentionPage {app:page.app;onBackRequested:page.section=0}
  }
 }
 FileDialog {id:protectedBackupOutput;fileMode:FileDialog.SaveFile;defaultSuffix:"nmrbackup";nameFilters:["Protected case backup (*.nmrbackup)"];onAccepted:page.saveProtectedBackup(selectedFile.toString())}
 FileDialog {id:protectedBackupInput;fileMode:FileDialog.OpenFile;nameFilters:["Protected case backup (*.nmrbackup)"];onAccepted:page.restoreProtectedBackup(selectedFile.toString())}
 FileDialog {id:anchorExport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";onAccepted:lawCaseBridge.exportAuditAnchor(selectedFile.toString(),anchorConsent.checked)}
 FileDialog {id:anchorInput;fileMode:FileDialog.OpenFile;onAccepted:page.anchorVerified=lawCaseBridge.verifyAuditAnchor(selectedFile.toString())}
 FileDialog {id:casePdf;fileMode:FileDialog.SaveFile;defaultSuffix:"pdf";nameFilters:["PDF (*.pdf)"];onAccepted:lawCaseBridge.exportCasePdf(selectedFile.toString(),page.app.ar,caseExportConsent.checked)}
 FileDialog {id:trainingReport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:lawCaseBridge.exportTrainingReport(selectedFile.toString(),trainingExportConsent.checked)}
}
