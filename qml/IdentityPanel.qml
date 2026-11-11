import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 property var identity:identityBridge
 objectName:"identityPanel"
 contentWidth:availableWidth
 clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {Layout.fillWidth:true;title:app.trx("هوية المستخدم المحلي","Local user identity");iconName:"document_lock"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("حسابات محلية ضمن حساب Windows الحالي. يلزم تحقق الجهة من هوية الأشخاص. لا تُرسل بيانات الحسابات والقضايا إلى خادم.","Local credentials within the current Windows account. Your organization must verify the people behind accounts. Identity and case data are not sent to a server.")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger;text:page.app.humanError(page.identity.error)}
   Label {visible:page.identity.authenticated;Layout.fillWidth:true;textFormat:Text.PlainText;color:app.design.text;text:(page.identity.session.displayName||"")+" · "+page.app.roleLabel(page.identity.session.role)}
   ColumnLayout {visible:!page.identity.authenticated;Layout.fillWidth:true
    TextField {id:username;objectName:"identityUsername";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("اسم المستخدم بالأحرف الصغيرة","Lowercase username");inputMethodHints:Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText}
    TextField {id:displayName;objectName:"identityDisplayName";visible:page.identity.needsSetup;Layout.fillWidth:true;maximumLength:128;placeholderText:app.trx("اسم المسؤول المحلي الأول","First local administrator display name")}
    TextField {id:password;objectName:"identityPassword";Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور (14 حرفًا على الأقل)","Password (at least 14 characters)")}
    ActionButton {objectName:"identitySignIn";text:page.identity.needsSetup?app.trx("إنشاء المسؤول الأول","Create first administrator"):app.trx("تسجيل الدخول","Sign in");enabled:page.identity.ready;primary:true;onClicked:{const setup=page.identity.needsSetup;const ok=!setup||page.identity.bootstrap(username.text,displayName.text,password.text);if(ok)page.identity.login(username.text,password.text);password.clear()}}
   }
   ColumnLayout {visible:page.identity.authenticated;Layout.fillWidth:true
    TextField {id:currentPassword;Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور الحالية للتغيير","Current password for rotation")}
    TextField {id:nextPassword;Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور الجديدة","New password")}
    ActionButton {text:app.trx("تغيير كلمة المرور وتسجيل الخروج","Change password and sign out");onClicked:{page.identity.changePassword(currentPassword.text,nextPassword.text);currentPassword.clear();nextPassword.clear()}}
   }
   ActionButton {visible:page.identity.authenticated;objectName:"identityLogout";text:app.trx("تسجيل الخروج ومسح عرض القضية","Sign out and clear case view");onClicked:page.identity.logout()}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تنتهي الجلسة بعد 15 دقيقة. يُقفل الحساب لخمس دقائق بعد خمس محاولات خاطئة. الحساب الإداري يدير المنح ولا يقرأ القضايا تلقائيًا.","Sessions expire after 15 minutes. Five failed attempts lock the account for five minutes. Administrators manage grants and do not automatically read cases.")}
  }
  Panel {visible:page.identity.session.role==="administrator";Layout.fillWidth:true;title:app.trx("إدارة الحسابات والمنح","Accounts and case grants");iconName:"clipboard_task"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("للعمل على القضايا، أنشئ حساب محقق من الحقول أدناه. يمكنك إنشاء الحساب والانتقال إليه بخطوة واحدة؛ حساب المدير يبقى لإدارة الحسابات والمنح.","For case work, create an investigator below. Create and switch in one step; the administrator retains account and grant management.")}
   TextField {id:newUsername;objectName:"identityNewUsername";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("اسم المستخدم الجديد","New username")}
   TextField {id:newDisplay;objectName:"identityNewDisplayName";Layout.fillWidth:true;maximumLength:128;placeholderText:app.trx("اسم العرض","Display name")}
   ComboBox {id:newRole;Layout.fillWidth:true;model:["investigator","reviewer","auditor","trainer","repair-technician","administrator"]}
   TextField {id:newPassword;objectName:"identityNewPassword";Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة مرور الحساب الجديد","New account password")}
   ActionButton {text:app.trx("إنشاء حساب","Create account");onClicked:{page.identity.createUser(newUsername.text,newDisplay.text,newRole.currentText,newPassword.text);newPassword.clear()}}
   ActionButton {objectName:"identityCreateInvestigatorAndSignIn";text:page.app.trx("إنشاء حساب المحقق والدخول إليه","Create investigator and sign in");primary:true;onClicked:{const user=newUsername.text;const pass=newPassword.text;const id=page.identity.createUser(user,newDisplay.text,"investigator",pass);if(id!=="")page.identity.login(user,pass);newPassword.clear()}}
   Repeater {model:page.identity.users;delegate:RowLayout {required property var modelData;Layout.fillWidth:true
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:modelData.username+" · "+page.app.roleLabel(modelData.role)+"\n"+modelData.id}
    ActionButton {text:modelData.enabled?page.app.trx("تعطيل","Disable"):page.app.trx("تمكين","Enable");enabled:modelData.id!==page.identity.session.id;onClicked:page.identity.setEnabled(modelData.id,!modelData.enabled)}
   }}
   TextField {id:caseId;Layout.fillWidth:true;maximumLength:36;placeholderText:app.trx("UUID القضية — يقدمه مالك القضية","Case UUID supplied by its owner")}
   TextField {id:principal;Layout.fillWidth:true;maximumLength:36;placeholderText:app.trx("UUID الحساب أعلاه","Account UUID shown above")}
   RowLayout {Layout.fillWidth:true
    ActionButton {text:app.trx("منح الوصول","Grant access");onClicked:page.identity.setCaseGrant(caseId.text,principal.text,true)}
    ActionButton {text:app.trx("سحب المنحة","Revoke grant");onClicked:page.identity.setCaseGrant(caseId.text,principal.text,false)}
   }
  }
  GovernmentNetworkPanel {app:page.app;Layout.fillWidth:true}
 }
}
