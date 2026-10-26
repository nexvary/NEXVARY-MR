import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Panel {
 id:panel
 property var app
 objectName:"governmentNetworkPanel"
 title:app.trx("سياسة العمليات الحكومية","Government operation policy")
 iconName:"document_lock"
 stateText:operationPolicy.offline?app.trx("المنع الافتراضي","Default deny"):app.trx("استثناءات مؤقتة نشطة","Temporary exceptions active")
 function grantCategory(name){const note=reason.text,approved=approval.checked;approval.checked=false;operationPolicy.grant(name,note,approved)}
 function caption(name){
  const ar={"cloud-publish":"نشر متابعة الورشة سحابيًا","tracking-portal":"بوابة المتابعة، بما فيها الاتصال المحلي","signature-update":"تنزيل تواقيع FreshClam","device-tools":"أدوات الأجهزة الخارجية، ADB وscrcpy وفحص USB","external-analysis":"محللات خارجية: ClamAV وmmls","external-links":"فتح روابط HTTP/HTTPS في المتصفح"}
  const en={"cloud-publish":"Cloud workshop publishing","tracking-portal":"Tracking listener, including loopback","signature-update":"FreshClam signature downloads","device-tools":"External device tools: ADB, scrcpy and USB inspection","external-analysis":"External analyzers: ClamAV and mmls","external-links":"HTTP/HTTPS browser handoff"}
  return (app.ar?ar:en)[name]||name
 }
 Label {objectName:"governmentNetworkScope";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تبدأ الفئات الست مغلقة، ولا يغير التنقل بين مساحات العمل صلاحياتها. الاستثناء لمسؤول محلي فقط، ومدته عشر دقائق كحد أقصى وينتهي عند الخروج أو تغيير الهوية. يوقف السحب الطلبات والعمليات المضيفة الجارية، ولا يسترجع بيانات نُشرت أو تغييرات بدأت. هذه سياسة تطبيق وليست عزلًا للشبكة على مستوى نظام التشغيل؛ تبقى المشاركات المركبة والعمليات التابعة خارج الاحتواء المثبت.","All six categories start blocked; switching workspaces does not grant permission. Only a local administrator can grant an exception, for at most ten minutes and until logout or identity change. Revocation stops active requests and host processes; published data and started changes cannot be recalled. This is an application gate, not an OS network sandbox; mounted shares and descendant processes remain outside verified containment.")}
 TextField {id:reason;objectName:"governmentNetworkReason";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("سبب الاستثناء المؤقت — محفوظ في الذاكرة فقط","Temporary exception reason — retained in memory only")}
 NeonCheckBox {
  id:approval;objectName:"governmentNetworkApproval";Layout.fillWidth:true
  text:app.trx("أوافق صراحة على مخاطر الفئة المحددة","Explicitly approve the selected category's risks")
  contentItem:Label {
   text:approval.text;wrapMode:Text.WordWrap;color:panel.app.design.text
   leftPadding:approval.mirrored?0:approval.indicator.width+approval.spacing
   rightPadding:approval.mirrored?approval.indicator.width+approval.spacing:0
   verticalAlignment:Text.AlignVCenter
  }
 }
 Repeater {
  model:operationPolicy.permissions
  delegate:ColumnLayout {
   required property var modelData
   Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:modelData.allowed?panel.app.design.accent:panel.app.design.text;text:panel.caption(modelData.capability)+" · "+(modelData.allowed?panel.app.trx("مسموح مؤقتًا","Temporarily allowed"):panel.app.trx("محظور","Blocked"))}
   Label {visible:modelData.allowed;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.muted;text:(modelData.reason||"")+"\n"+(modelData.actor||"")+" · "+(modelData.expiresUtc||"")}
   RowLayout {Layout.fillWidth:true
    ActionButton {objectName:"governmentNetworkGrant-"+modelData.capability;text:panel.app.trx("منح استثناء","Grant exception");enabled:identityBridge.session.role==="administrator"&&approval.checked&&reason.text.trim().length>0&&!modelData.allowed;onClicked:panel.grantCategory(modelData.capability)}
    ActionButton {objectName:"governmentNetworkRevoke-"+modelData.capability;text:panel.app.trx("سحب","Revoke");enabled:modelData.allowed;onClicked:operationPolicy.revoke(modelData.capability)}
    Item {Layout.fillWidth:true}
   }
  }
 }
 ActionButton {objectName:"governmentNetworkRevokeAll";text:app.trx("إغلاق جميع الاستثناءات","Close all exceptions");enabled:!operationPolicy.offline;onClicked:operationPolicy.revokeAll()}
 Label {objectName:"governmentNetworkError";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger;text:operationPolicy.error}
 Connections {target:identityBridge;function onChanged(){reason.clear();approval.checked=false}}
}
