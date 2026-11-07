import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 objectName:"lawRetentionPage"
 signal backRequested()
 property var app
 readonly property var governance:lawCaseBridge.activeCase.retention||({})
 readonly property var items:lawCaseBridge.activeCase.evidence||[]
 readonly property string evidenceId:evidence.currentIndex>=0&&evidence.currentIndex<items.length?items[evidence.currentIndex].id:""
 property var assessment:({})
 property var verification:({})
 contentWidth:availableWidth;clip:true
 function evidenceLabel(id){for(const e of page.items)if(e.id===id)return e.name;return id}
 function operate(action,fields){const ok=lawCaseBridge.retentionOperation(action,fields);if(ok){assessment=lawCaseBridge.retentionAssessment(evidenceId);verification=({})}return ok}
 Connections {target:lawCaseBridge;function onChanged(){page.assessment=({});page.verification=({})}}
 ColumnLayout {width:page.availableWidth;spacing:12
  ActionButton {objectName:"retentionBack";text:app.trx("رجوع إلى القضية","Back to case");onClicked:page.backRequested()}
  Panel {Layout.fillWidth:true;title:app.trx("الاحتفاظ بالأدلة والأرشفة","Evidence retention & archiving");iconName:"document_lock"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("انتهاء المدة تقييم فقط. الحجز القانوني واستثناءات الحفظ يمنعان طلب الإتلاف. الموافقة تتطلب حساب مراجع مختلفًا؛ لا تُحذف ملفات الأدلة في هذا المسار.","Expiry is an assessment. Legal holds and preservation exceptions block disposal requests. Approval requires a different reviewer account; this workflow retains every evidence byte.")}
   ComboBox {id:evidence;objectName:"retentionEvidence";Layout.fillWidth:true;model:page.items;textRole:"name";onActivated:{page.assessment=({});page.verification=({})}}
   RowLayout {Layout.fillWidth:true
    ActionButton {objectName:"retentionAssess";text:app.trx("تقييم الاحتفاظ الآن","Assess retention now");enabled:page.evidenceId!=="";onClicked:page.assessment=lawCaseBridge.retentionAssessment(page.evidenceId)}
    ActionButton {objectName:"retentionArchive";text:app.trx("أرشفة محمية في موضعها","Archive protected in place");enabled:page.evidenceId!==""&&lawCaseBridge.permissions.archiveEvidence;onClicked:page.operate("archive",{evidenceId:page.evidenceId})}
   }
   GridLayout {visible:!!page.assessment.evidenceId;Layout.fillWidth:true;columns:width>850?3:2;columnSpacing:16;rowSpacing:12
    Repeater {model:[{ar:"السياسة",en:"Policy",value:page.assessment.policyAssigned?page.app.trx("مرتبطة","Assigned"):page.app.trx("غير مرتبطة","Not assigned")},{ar:"قيود الحفظ",en:"Preservation",value:page.assessment.held?page.app.trx("حجز أو استثناء نشط","Active hold or exception"):page.app.trx("لا يوجد قيد نشط","No active restriction")},{ar:"مدة الاحتفاظ",en:"Retention",value:page.assessment.expired?page.app.trx("انتهت المدة","Expired"):page.app.trx("لم تنتهِ المدة","Not expired")},{ar:"انتهاء المدة UTC",en:"Expiry UTC",value:page.assessment.expiryUtc||"—"},{ar:"قاعدة الأرشفة",en:"Archive rule",value:page.assessment.archiveDue?page.app.trx("حان موعدها","Due"):page.app.trx("لم يحن موعدها","Not due")},{ar:"طلب الإتلاف",en:"Disposal request",value:page.assessment.disposalEligible?page.app.trx("متاح للمراجعة","Available for review"):page.app.trx("غير مستوفٍ للشروط","Requirements not met")}]
     delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true;spacing:4
      Label {Layout.fillWidth:true;color:page.app.design.muted;text:page.app.ar?modelData.ar:modelData.en}
      Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;font.bold:true;text:String(modelData.value)}
     }
    }
   }
   Label {objectName:"retentionAssessment";visible:!!page.assessment.evidenceId;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("التقييم حسب ساعة الجهاز المحلية. احتفظ بمرجع التوقيت المعتمد لدى الجهة عند اتخاذ القرار.","Assessment uses the local system clock. Use your institution's trusted time reference for records decisions.")}
  }
  Panel {Layout.fillWidth:true;title:app.trx("سياسات يقرها المراجع","Reviewer controlled policies");iconName:"shield_checkmark"
   TextField {id:policyLabel;objectName:"retentionPolicyLabel";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("اسم السياسة","Policy label")}
   RowLayout {Layout.fillWidth:true
    Label {text:app.trx("مدة الاحتفاظ بالأيام","Retention days");color:app.design.muted}
    SpinBox {id:days;objectName:"retentionDays";from:1;to:36500;value:365}
    Label {text:app.trx("الأرشفة بعد أيام","Archive after days");color:app.design.muted}
    SpinBox {id:archiveDays;from:0;to:days.value;value:0}
   }
   TextField {id:authority;objectName:"retentionAuthority";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("مرجع السياسة أو السند المختص","Policy / competent authority reference")}
   ActionButton {objectName:"retentionCreatePolicy";text:app.trx("إنشاء سياسة ثابتة","Create immutable policy");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.retention&&policyLabel.text.trim()!==""&&authority.text.trim()!=="";onClicked:page.operate("policy",{label:policyLabel.text,days:days.value,archiveAfterDays:archiveDays.value,authorityReference:authority.text})}
   ComboBox {id:policy;objectName:"retentionPolicy";Layout.fillWidth:true;model:page.governance.policies||[];textRole:"label"}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.muted;text:policy.currentIndex>=0&&page.governance.policies?page.app.trx("الاحتفاظ: ","Retention: ")+page.governance.policies[policy.currentIndex].days+page.app.trx(" يوم · الأرشفة بعد: "," days · Archive after: ")+page.governance.policies[policy.currentIndex].archiveAfterDays+page.app.trx(" يوم\nالسند: "," days\nAuthority: ")+page.governance.policies[policy.currentIndex].authorityReference:""}
   ActionButton {objectName:"retentionAssignPolicy";text:app.trx("ربط السياسة بالدليل المحدد","Assign policy to selected evidence");enabled:page.evidenceId!==""&&policy.currentIndex>=0&&lawCaseBridge.permissions.retention;onClicked:page.operate("assign",{evidenceId:page.evidenceId,policyId:page.governance.policies[policy.currentIndex].id})}
  }
  Panel {Layout.fillWidth:true;title:app.trx("الحجز القانوني واستثناء الحفظ","Legal hold & preservation exception");iconName:"shield_checkmark"
   ComboBox {id:kind;Layout.fillWidth:true;model:[app.trx("حجز قانوني","Legal hold"),app.trx("استثناء حفظ","Preservation exception")]}
   TextField {id:reason;objectName:"retentionReason";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("سبب موثق للحجز أو المراجعة","Documented preservation / review reason")}
   TextField {id:holdAuthority;objectName:"retentionHoldAuthority";Layout.fillWidth:true;maximumLength:2048;placeholderText:app.trx("مرجع السند المختص","Competent authority reference")}
   ActionButton {objectName:"retentionPlaceHold";text:app.trx("تسجيل قيد حفظ","Record preservation restriction");enabled:page.evidenceId!==""&&lawCaseBridge.permissions.preserve&&reason.text.trim()!==""&&holdAuthority.text.trim()!=="";onClicked:page.operate("hold",{evidenceId:page.evidenceId,kind:kind.currentIndex===0?"legal-hold":"preservation-exception",reason:reason.text,authorityReference:holdAuthority.text})}
   Repeater {model:page.governance.holds||[]
    delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:(modelData.kind==="legal-hold"?page.app.trx("حجز قانوني","Legal hold"):page.app.trx("استثناء حفظ","Preservation exception"))+" · "+page.evidenceLabel(modelData.evidenceId)+"\n"+modelData.reason+"\n"+(modelData.releasedAtUtc||page.app.trx("نشط","Active"))}
     ActionButton {text:page.app.trx("رفع القيد بمرجع المراجعة","Release with review reference");enabled:!modelData.releasedAtUtc&&lawCaseBridge.permissions.releasePreservation&&reason.text.trim()!==""&&holdAuthority.text.trim()!=="";onClicked:page.operate("release",{holdId:modelData.id,reason:reason.text,authorityReference:holdAuthority.text})}
    }
   }
  }
  Panel {Layout.fillWidth:true;title:app.trx("التحقق من الأرشيف","Archive integrity verification");iconName:"checkmark_circle"
   Repeater {model:page.governance.archives||[]
    delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:page.evidenceLabel(modelData.evidenceId)+"\nSHA-256: "+modelData.sha256}
     ActionButton {objectName:"retentionVerifyArchive";text:page.app.trx("تحقق من البايتات المحمية","Verify protected bytes");enabled:lawCaseBridge.permissions.audit;onClicked:page.verification=lawCaseBridge.verifyArchive(modelData.id)}
    }
   }
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.success;text:page.verification.verified?app.trx("تم التحقق: ","Verified: ")+page.verification.sha256:""}
  }
  Panel {Layout.fillWidth:true;title:app.trx("طلبات الإتلاف ومراجعتها","Disposal requests & review");iconName:"warning"
   CheckBox {id:consent;objectName:"retentionDisposalConsent";text:app.trx("أقر بمراجعة السند والسياسة؛ هذا طلب بلا حذف","Confirm authority / policy review; request retains all bytes")}
   ActionButton {objectName:"retentionRequestDisposal";text:app.trx("إرسال طلب إتلاف للمراجعة","Submit disposal request for review");enabled:lawCaseBridge.permissions.requestDisposal&&page.evidenceId!==""&&consent.checked&&reason.text.trim()!==""&&holdAuthority.text.trim()!=="";onClicked:{page.operate("request-disposal",{evidenceId:page.evidenceId,reason:reason.text,authorityReference:holdAuthority.text});consent.checked=false}}
   Repeater {model:page.governance.disposals||[]
    delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.muted;text:page.evidenceLabel(modelData.evidenceId)+" · "+modelData.status+"\n"+modelData.reason}
     RowLayout {Layout.fillWidth:true
      ActionButton {text:page.app.trx("موافقة المراجع","Reviewer approval");enabled:modelData.status==="requested"&&lawCaseBridge.permissions.reviewDisposal&&reason.text.trim()!==""&&holdAuthority.text.trim()!=="";onClicked:page.operate("review-disposal",{requestId:modelData.id,decision:"approved",reason:reason.text,authorityReference:holdAuthority.text})}
      ActionButton {text:page.app.trx("رفض الطلب","Reject request");enabled:modelData.status==="requested"&&lawCaseBridge.permissions.reviewDisposal&&reason.text.trim()!==""&&holdAuthority.text.trim()!=="";onClicked:page.operate("review-disposal",{requestId:modelData.id,decision:"rejected",reason:reason.text,authorityReference:holdAuthority.text})}
     }
    }
   }
  }
 }
}
