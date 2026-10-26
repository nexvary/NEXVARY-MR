import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:panel
 property var app
 property string selectedRequest:""
 readonly property string role:identityBridge.session.role||""
 readonly property string principal:identityBridge.session.id||""
 readonly property bool canRequest:role==="administrator"||role==="investigator"||role==="repair-technician"
 readonly property bool canReview:role==="administrator"||role==="reviewer"
 readonly property bool canAudit:canReview||role==="auditor"
 objectName:"governmentNetworkPanel"
 title:app.trx("سياسة العمليات الحكومية","Government operation policy")
 iconName:"document_lock"
 stateText:operationPolicy.offline?app.trx("المنع الافتراضي","Default deny"):app.trx("استثناءات مؤقتة نشطة","Temporary exceptions active")
 function requestCategory(name){const consent=approval.checked;approval.checked=false;selectedRequest=operationPolicy.requestException(name,reason.text,caseRef.text,seconds.value,consent)}
 function caption(name){
  const ar={"cloud-publish":"نشر متابعة الورشة سحابيًا","tracking-portal":"بوابة المتابعة، بما فيها الاتصال المحلي","signature-update":"تنزيل تواقيع FreshClam","device-tools":"أدوات الأجهزة الخارجية، ADB وscrcpy وفحص USB","external-analysis":"محللات خارجية: ClamAV وmmls","external-links":"فتح روابط HTTP/HTTPS في المتصفح"}
  const en={"cloud-publish":"Cloud workshop publishing","tracking-portal":"Tracking listener, including loopback","signature-update":"FreshClam signature downloads","device-tools":"External device tools: ADB, scrcpy and USB inspection","external-analysis":"External analyzers: ClamAV and mmls","external-links":"HTTP/HTTPS browser handoff"}
  return (app.ar?ar:en)[name]||name
 }
 function statusCaption(status){
  const ar={"requested":"بانتظار المراجعة","in-review":"قيد المراجعة","approved":"موافق عليه — يحتاج تفعيل الطالب","rejected":"مرفوض","active":"نشط","revoked":"مسحوب","expired":"منتهي الصلاحية","started":"بدأ التنفيذ","completed":"مكتمل","failed":"فشل","cancelled":"ملغى"}
  return app.ar?(ar[status]||status):status
 }
 Label {objectName:"governmentNetworkScope";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("طلب لفئة واحدة وطالب محدد، ثم مراجعة وقرار من حساب آخر، ثم تفعيل الطالب لمدة لا تتجاوز عشر دقائق. تغيير الهوية أو الخروج يسحب التفعيل. يسجل التطبيق القرارات بتواقيع محلية؛ اختلاف الحسابات لا يثبت استقلال الأشخاص والتوقيت محلي. نطاق الفئة لا يقيد وجهة الشبكة أو الأداة، ومرجع القضية لا يمنح الوصول إليها. تبقى المشاركات المركبة والعمليات التابعة والمتصفح خارج احتواء نظام التشغيل المثبت.","Request one category for one principal, obtain review and a decision from a different account, then activate as the requester for at most ten minutes. Identity change/logout revokes activation. Decisions have local signatures; different accounts do not prove independent people and time is local. Category scope does not restrict destinations/tools; a case reference grants no case access. Mounted shares, descendants and browser activity remain outside verified OS containment.")}
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:operationPolicy.auditStatus.healthy?app.design.accent:app.design.danger;text:operationPolicy.auditStatus.healthy?app.trx("سجل السياسات متاح — تحقق محلي","Policy journal available — local verification"):app.trx("تعذر التحقق من سجل السياسات — الاستثناءات مغلقة","Policy journal unavailable — exceptions denied")}
 ColumnLayout {
  Layout.fillWidth:true
  visible:operationPolicy.operationReceipts.length>0
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:panel.app.trx("أحدث إيصالات تشغيل الفحص والتواقيع — قرار الموافقة وحده لا يعني اكتمال العملية. أوقات محلية؛ مرجع القضية ليس تصريح وصول.","Recent scan/signature operation receipts — approval alone is not completion. Local times; case references grant no access.");color:panel.app.design.muted}
  Repeater {
   model:operationPolicy.operationReceipts
   delegate:Label {
    required property var modelData
    objectName:"governmentOperationReceipt-"+modelData.operationId
    Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.text
    text:(panel.app.ar?(modelData.kind==="security-scan"?"فحص ملف":"تحديث تواقيع"):modelData.kind)+" · "+panel.statusCaption(modelData.status)+" · "+modelData.failureClass+"\n"+modelData.operationId+"\n"+modelData.startedUtc+" → "+modelData.completedUtc+"\nSHA-256: "+(modelData.outputSha256||"—")
   }
  }
 }
 TextField {id:reason;objectName:"governmentNetworkReason";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("سبب الطلب أو المراجعة أو القرار — يسجل بتوقيع","Request, review or decision reason — signed and retained")}
 TextField {id:caseRef;objectName:"governmentNetworkCaseRef";Layout.fillWidth:true;maximumLength:36;placeholderText:app.trx("مرجع القضية UUID اختياري، وليس تصريح وصول","Optional case UUID reference; not an access grant")}
 RowLayout {Layout.fillWidth:true
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("مدة التفعيل بالثواني (1–600)","Activation duration in seconds (1–600)")}
  SpinBox {id:seconds;objectName:"governmentNetworkDuration";from:1;to:600;value:600;editable:true}
 }
 NeonCheckBox {
  id:approval;objectName:"governmentNetworkApproval";Layout.fillWidth:true
  text:app.trx("أقر بطلب الاستثناء ومخاطر الفئة المحددة","Consent to the request and selected category's risks")
  contentItem:Label {text:approval.text;wrapMode:Text.WordWrap;color:panel.app.design.text;leftPadding:approval.mirrored?0:approval.indicator.width+approval.spacing;rightPadding:approval.mirrored?approval.indicator.width+approval.spacing:0;verticalAlignment:Text.AlignVCenter}
 }
 Repeater {
  model:operationPolicy.permissions
  delegate:ColumnLayout {
   required property var modelData
   Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:modelData.allowed?panel.app.design.accent:panel.app.design.text;text:panel.caption(modelData.capability)+" · "+(modelData.allowed?panel.app.trx("مسموح مؤقتًا","Temporarily allowed"):panel.app.trx("محظور","Blocked"))}
   Label {visible:modelData.allowed;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:(modelData.reason||"")+"\n"+(modelData.actor||"")+" · "+(modelData.expiresUtc||"")}
   RowLayout {Layout.fillWidth:true
    ActionButton {objectName:"governmentNetworkGrant-"+modelData.capability;Layout.fillWidth:true;text:panel.app.trx("طلب استثناء","Request exception");enabled:panel.canRequest&&approval.checked&&reason.text.trim().length>0&&!modelData.allowed;onClicked:panel.requestCategory(modelData.capability)}
    ActionButton {objectName:"governmentNetworkRevoke-"+modelData.capability;Layout.fillWidth:true;text:panel.app.trx("إيقاف فوري","Stop now");enabled:modelData.allowed;onClicked:operationPolicy.revoke(modelData.capability)}
   }
  }
 }
 ActionButton {objectName:"governmentNetworkRevokeAll";text:app.trx("إغلاق جميع الاستثناءات النشطة","Close all active exceptions");enabled:!operationPolicy.offline;onClicked:operationPolicy.revokeAll()}
 ActionButton {objectName:"governmentPolicyRefresh";text:app.trx("تحديث السجل والطلبات","Refresh journal and requests");onClicked:operationPolicy.refreshAudit()}
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("آخر 32 طلبًا مرئيًا لهذا الحساب؛ يحتوي التصدير على السجل الكامل ضمن حدوده.","Latest 32 requests visible to this account; export contains the complete bounded history.")}
 Repeater {
  model:operationPolicy.requests.slice(0,32)
  delegate:ColumnLayout {
   required property var modelData
   readonly property bool owner:modelData.requester===panel.principal
   readonly property bool assigned:modelData.reviewer===panel.principal
   readonly property bool closed:["rejected","revoked","expired"].indexOf(modelData.status)>=0
   Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.text;text:panel.caption(modelData.capability)+" · "+panel.statusCaption(modelData.status)+"\n"+modelData.id+"\n"+modelData.requesterActor+"\n"+modelData.reason+(modelData.caseRef?"\n"+modelData.caseRef:"")}
   Label {visible:!!modelData.reviewer;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:(modelData.reviewer||"")+"\n"+(modelData.reviewReason||"")+"\n"+(modelData.decisionReason||"")}
   ActionButton {objectName:"governmentPolicyReview-"+modelData.id;visible:modelData.status==="requested";enabled:panel.canReview&&!owner&&reason.text.trim().length>0;text:panel.app.trx("بدء المراجعة","Start review");onClicked:operationPolicy.reviewException(modelData.id,reason.text)}
   RowLayout {visible:modelData.status==="in-review";Layout.fillWidth:true
    ActionButton {objectName:"governmentPolicyApprove-"+modelData.id;Layout.fillWidth:true;enabled:panel.canReview&&assigned&&!owner&&reason.text.trim().length>0;text:panel.app.trx("موافقة","Approve");onClicked:operationPolicy.decideException(modelData.id,true,reason.text)}
    ActionButton {objectName:"governmentPolicyReject-"+modelData.id;Layout.fillWidth:true;enabled:panel.canReview&&assigned&&!owner&&reason.text.trim().length>0;text:panel.app.trx("رفض","Reject");onClicked:operationPolicy.decideException(modelData.id,false,reason.text)}
   }
   ActionButton {objectName:"governmentPolicyActivate-"+modelData.id;visible:modelData.status==="approved";enabled:owner;text:panel.app.trx("تفعيل بواسطة الطالب","Activate as requester");onClicked:operationPolicy.activateException(modelData.id)}
   ActionButton {objectName:"governmentPolicyCancel-"+modelData.id;visible:!closed;enabled:owner||assigned||panel.role==="administrator";text:panel.app.trx("سحب الطلب أو الموافقة","Revoke request or approval");onClicked:operationPolicy.revokeException(modelData.id)}
  }
 }
 ColumnLayout {
  visible:panel.canAudit;Layout.fillWidth:true
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("احتفظ بمرساة التحقق التالية في مكان مستقل. حذف السجل أو إرجاع ملف صحيح قديم بعد إعادة التشغيل يحتاج مرساة مستقلة لكشفه. التصدير غير مشفر ويحتوي أسباب القرارات ومراجعها.","Retain this anchor separately. Whole-history reset or valid-file rollback after restart requires an independently retained anchor to detect. Export is unencrypted and includes decision reasons and references.")}
  TextArea {objectName:"governmentPolicyAnchor";Layout.fillWidth:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;text:JSON.stringify({journalId:operationPolicy.auditStatus.journalId,publicKey:operationPolicy.auditStatus.publicKey,sequence:operationPolicy.auditStatus.sequence,hash:operationPolicy.auditStatus.hash})}
  TextField {id:auditPath;objectName:"governmentPolicyAuditPath";Layout.fillWidth:true;placeholderText:app.trx("المسار المحلي الكامل لملف السجل JSON","Full local path to the audit JSON file")}
  NeonCheckBox {id:exportConsent;text:app.trx("أوافق على تصدير سجل غير مشفر","Consent to unencrypted audit export")}
  ActionButton {objectName:"governmentPolicyExport";enabled:auditPath.text.length>0&&exportConsent.checked;text:app.trx("تصدير إلى ملف جديد","Export to a new file");onClicked:operationPolicy.exportAudit(auditPath.text,exportConsent.checked)}
  TextArea {id:retained;objectName:"governmentPolicyRetainedAnchor";Layout.fillWidth:true;selectByMouse:true;wrapMode:TextEdit.WrapAnywhere;placeholderText:app.trx("ألصق مرساة JSON المحفوظة مستقلًا","Paste the independently retained JSON anchor")}
  ActionButton {objectName:"governmentPolicyVerify";enabled:auditPath.text.length>0&&retained.text.length>0;text:app.trx("تحقق من التواقيع والترتيب والمرساة","Verify signatures, order and retained anchor");onClicked:operationPolicy.verifyAudit(auditPath.text,retained.text)}
  Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.muted;text:operationPolicy.auditStatus.verification?JSON.stringify(operationPolicy.auditStatus.verification):""}
 }
 Label {objectName:"governmentNetworkError";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger;text:operationPolicy.error}
 Connections {target:identityBridge;function onChanged(){reason.clear();caseRef.clear();approval.checked=false;exportConsent.checked=false;auditPath.clear();retained.clear();panel.selectedRequest=""}}
}
