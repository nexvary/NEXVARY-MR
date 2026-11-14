import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 property var identity:identityBridge
 property string feedback:""
 function roleAllows(role,p){if(p==="user-administration")return role==="administrator";if(p==="external-connection")return ["administrator","investigator","reviewer"].indexOf(role)>=0;if(p==="case-create"||p==="case-edit")return role==="investigator";if(p==="evidence-access")return ["investigator","reviewer","auditor"].indexOf(role)>=0;return role==="investigator"||role==="reviewer"}
 function connectionAllowed(){const rows=operationPolicy.permissions;for(const row of rows)if(row.capability==="external-analysis")return row.allowed;return false}
 objectName:"identityPanel"
 contentWidth:availableWidth
 clip:true
 component IdentityField: EntryField {
  id:field
  color:enabled?page.app.design.text:page.app.design.muted
  Layout.maximumWidth:260
  validationKind:echoMode===TextInput.Password?"password":"text"
  placeholderTextColor:page.app.design.muted
  selectionColor:page.app.design.accent
  selectedTextColor:page.app.design.background
  background:Rectangle {
   radius:5
   color:field.enabled?page.app.design.surface:page.app.design.raised
   border.width:field.activeFocus?2:1
   border.color:field.activeFocus?page.app.design.accent:field.hovered?page.app.design.silver:page.app.design.border
  }
 }
 ColumnLayout {
  width:page.availableWidth;spacing:12
  Label {visible:page.identity.directWork;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("العمل مباشر دون حساب داخل البرنامج. إعداد الاستخبارات متاح من تبويب الاستخبارات.","Work directly without an application account. Configure Intelligence in the Intelligence tab.")}
  ColumnLayout {visible:!page.identity.directWork;Layout.fillWidth:true;spacing:12
  Panel {Layout.fillWidth:true;title:app.trx("الإعداد الأول — تقدم متحقق","First-run setup — verified progress");iconName:"clipboard_task"
   Repeater {model:[
    {ar:"1. إنشاء المدير",en:"1. Set up administrator",done:page.identity.ready&&!page.identity.needsSetup},
    {ar:"2. حساب محقق مفعّل",en:"2. Enabled investigator account",done:page.identity.setupStatus.investigator},
    {ar:"3. تفعيل إذن الاتصال لحسابك",en:"3. Activate connection permission for your account",done:page.connectionAllowed()},
    {ar:"4. حفظ مفتاح استخبارات نيكسفاري محميًا",en:"4. Save protected Intelligence key",done:lampyreConnection.credentialSaved},
    {ar:"5. اختبار الاتصال واكتشاف الأدوات",en:"5. Test connection and discover tools",done:lampyreConnection.connected},
    {ar:"6. فتح قضية مسموحة",en:"6. Open an accessible case",done:!!lawCaseBridge.activeCase.id}
   ];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:modelData.done?page.app.design.accent:page.app.design.muted;text:(page.app.ar?modelData.ar:modelData.en)+" · "+(modelData.done?page.app.trx("مكتمل الآن","Complete now"):page.app.trx("غير مكتمل","Incomplete"))}}
   ActionButton {text:app.trx("فتح إعداد استخبارات نيكسفاري","Open Intelligence setup");onClicked:app.navigatePrimary(3)}
   ActionButton {text:app.trx("فتح القضايا","Open cases");onClicked:{app.setWorkspace(true);app.navigateSection(1)}}
  }
  GridLayout {id:accountForms;Layout.fillWidth:true;columns:width>=760?2:1;columnSpacing:16;rowSpacing:12
  Panel {Layout.fillWidth:true;Layout.alignment:Qt.AlignTop;title:app.trx("هوية المستخدم المحلي","Local user identity");iconName:"document_lock"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("حسابات محلية ضمن حساب Windows الحالي. يلزم تحقق الجهة من هوية الأشخاص. لا تُرسل بيانات الحسابات والقضايا إلى خادم.","Local credentials within the current Windows account. Your organization must verify the people behind accounts. Identity and case data are not sent to a server.")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.danger;text:page.app.humanError(page.identity.error)}
   Label {visible:page.identity.authenticated;Layout.fillWidth:true;textFormat:Text.PlainText;color:app.design.text;text:(page.identity.session.displayName||"")+" · "+page.app.roleLabel(page.identity.session.role)}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.accent;text:page.feedback}
   GridLayout {visible:!page.identity.authenticated;Layout.fillWidth:true;columns:width>=480?2:1;columnSpacing:16
    IdentityField {id:username;validationKind:"username";objectName:"identityUsername";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("اسم المستخدم بالأحرف الصغيرة","Lowercase username");inputMethodHints:Qt.ImhNoAutoUppercase|Qt.ImhNoPredictiveText}
    IdentityField {id:displayName;objectName:"identityDisplayName";visible:page.identity.needsSetup;Layout.fillWidth:true;maximumLength:128;placeholderText:app.trx("اسم المسؤول المحلي الأول","First local administrator display name")}
    IdentityField {id:password;objectName:"identityPassword";Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور (14 حرفًا على الأقل)","Password (at least 14 characters)")}
    ActionButton {objectName:"identitySignIn";text:page.identity.needsSetup?app.trx("إنشاء المسؤول الأول","Create first administrator"):app.trx("تسجيل الدخول","Sign in");enabled:page.identity.ready;primary:true;onClicked:{const setup=page.identity.needsSetup;const ok=!setup||page.identity.bootstrap(username.text,displayName.text,password.text);if(ok)page.identity.login(username.text,password.text);password.clear()}}
   }
   GridLayout {visible:page.identity.authenticated;Layout.fillWidth:true;columns:width>=480?2:1;columnSpacing:16
    IdentityField {id:currentPassword;Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور الحالية للتغيير","Current password for rotation")}
    IdentityField {id:nextPassword;Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة المرور الجديدة","New password")}
    ActionButton {text:app.trx("تغيير كلمة المرور وتسجيل الخروج","Change password and sign out");onClicked:{page.identity.changePassword(currentPassword.text,nextPassword.text);currentPassword.clear();nextPassword.clear()}}
   }
   ActionButton {visible:page.identity.authenticated;objectName:"identityLogout";text:app.trx("تبديل الحساب / تسجيل الخروج","Switch account / sign out");onClicked:page.identity.logout()}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تنتهي الجلسة بعد 15 دقيقة. يُقفل الحساب لخمس دقائق بعد خمس محاولات خاطئة. الحساب الإداري يدير المنح ولا يقرأ القضايا تلقائيًا.","Sessions expire after 15 minutes. Five failed attempts lock the account for five minutes. Administrators manage grants and do not automatically read cases.")}
  }
  Panel {visible:page.identity.session.role==="administrator";Layout.fillWidth:true;Layout.alignment:Qt.AlignTop;title:app.trx("إدارة الحسابات والمنح","Accounts and case grants");iconName:"clipboard_task"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("للعمل على القضايا، أنشئ حساب محقق من الحقول أدناه. يمكنك إنشاء الحساب والانتقال إليه بخطوة واحدة؛ حساب المدير يبقى لإدارة الحسابات والمنح.","For case work, create an investigator below. Create and switch in one step; the administrator retains account and grant management.")}
   IdentityField {id:newUsername;validationKind:"username";objectName:"identityNewUsername";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("اسم المستخدم الجديد","New username")}
   IdentityField {id:newDisplay;objectName:"identityNewDisplayName";Layout.fillWidth:true;maximumLength:128;placeholderText:app.trx("اسم العرض","Display name")}
   ComboBox {id:newRole;Layout.fillWidth:true;model:["investigator","reviewer","auditor","trainer","repair-technician","administrator"]}
   IdentityField {id:newPassword;objectName:"identityNewPassword";Layout.fillWidth:true;maximumLength:256;echoMode:TextInput.Password;placeholderText:app.trx("كلمة مرور الحساب الجديد","New account password")}
   ActionButton {text:app.trx("إنشاء حساب","Create account");onClicked:{const id=page.identity.createUser(newUsername.text,newDisplay.text,newRole.currentText,newPassword.text);page.feedback=id!==""?app.trx("تم إنشاء الحساب بنجاح؛ يمكنك الانتقال إليه عبر تبديل الحساب","Account created successfully; use Switch account to sign in"):"";newPassword.clear()}}
   ActionButton {objectName:"identityCreateInvestigatorAndSignIn";text:page.app.trx("إنشاء حساب المحقق والدخول إليه","Create investigator and sign in");primary:true;onClicked:{const user=newUsername.text;const pass=newPassword.text;const id=page.identity.createUser(user,newDisplay.text,"investigator",pass);if(id!=="")page.identity.login(user,pass);newPassword.clear()}}
  }
  }
  Panel {visible:page.identity.session.role==="administrator";Layout.fillWidth:true;title:app.trx("المستخدمون والمنح","Users and grants");iconName:"settings"
   Repeater {model:page.identity.users;delegate:ColumnLayout {id:accountRow;required property var modelData;Layout.fillWidth:true
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:modelData.username+" · "+page.app.roleLabel(modelData.role)+"\n"+modelData.id}
    ActionButton {text:modelData.enabled?page.app.trx("تعطيل","Disable"):page.app.trx("تمكين","Enable");enabled:modelData.id!==page.identity.session.id;onClicked:page.identity.setEnabled(modelData.id,!modelData.enabled)}
    RowLayout {
     Layout.fillWidth:true
     Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:page.app.trx("سقف إرسال الاستخبارات المتبقي: ","Remaining intelligence submissions: ")+(lampyreConnection.spendingAllowances[accountRow.modelData.id]||0)}
     SpinBox {id:adminBudget;from:0;to:100;value:0;editable:true}
     ActionButton {text:page.app.trx("ضبط السقف","Set ceiling");onClicked:{const ok=lampyreConnection.setSpendingLimit(accountRow.modelData.id,adminBudget.value);page.feedback=ok?page.app.trx("تم حفظ السقف الإداري؛ يستمر بعد إعادة التشغيل ولا يحدد تكلفة مالية","Administrative ceiling saved across restarts; it does not cap monetary cost"):page.app.humanError(lampyreConnection.errorCode)}}
    }
    Repeater {model:[{id:"case-create",ar:"إنشاء القضايا",en:"Create cases"},{id:"case-edit",ar:"تعديل القضايا",en:"Edit cases"},{id:"evidence-access",ar:"قراءة الأدلة",en:"Evidence access"},{id:"osint-investigation",ar:"تحقيق OSINT",en:"OSINT investigation"},{id:"external-connection",ar:"اتصال API/MCP",en:"API/MCP connection"},{id:"paid-search",ar:"البحث المدفوع",en:"Paid searches"},{id:"evidence-export",ar:"تصدير الأدلة",en:"Evidence export"},{id:"user-administration",ar:"إدارة المستخدمين",en:"User administration"}]
     delegate:NeonCheckBox {required property var modelData;text:page.app.ar?modelData.ar:modelData.en;checked:page.roleAllows(accountRow.modelData.role,modelData.id)&&(accountRow.modelData.deniedPermissions||[]).indexOf(modelData.id)<0;enabled:page.roleAllows(accountRow.modelData.role,modelData.id)&&!(accountRow.modelData.id===page.identity.session.id&&modelData.id==="user-administration");onClicked:{const ok=page.identity.setPermission(accountRow.modelData.id,modelData.id,checked);page.feedback=ok?page.app.trx("تم حفظ الصلاحية؛ يجب إعادة دخول الحساب المتأثر","Permission saved; affected account must sign in again"):""}}
    }
   }}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("صلاحيات كل حساب تظهر أدناه. السماح يبقى ضمن سقف الدور، وتغييرها ينهي جلسة الحساب المتأثر ويحتاج تسجيل دخول جديد. الوصول لقضية وموافقة التحليل الخارجي شرطان مستقلان.","Permissions remain within the role ceiling. A permission change ends the affected session and requires sign-in again. Case access and reviewed external-analysis grants remain separate requirements.")}
   IdentityField {id:caseId;validationKind:"uuid";Layout.fillWidth:true;maximumLength:36;placeholderText:app.trx("UUID القضية — يقدمه مالك القضية","Case UUID supplied by its owner")}
   IdentityField {id:principal;validationKind:"uuid";Layout.fillWidth:true;maximumLength:36;placeholderText:app.trx("UUID الحساب أعلاه","Account UUID shown above")}
   RowLayout {Layout.fillWidth:true
    ActionButton {text:app.trx("منح الوصول","Grant access");onClicked:page.identity.setCaseGrant(caseId.text,principal.text,true)}
    ActionButton {text:app.trx("سحب المنحة","Revoke grant");onClicked:page.identity.setCaseGrant(caseId.text,principal.text,false)}
   }
  }
  Panel {Layout.fillWidth:true;title:app.trx("حالة الحساب والصلاحيات الفعلية","Account and actual permission status");iconName:"document_lock"
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.design.text;text:app.trx("المستخدم: ","User: ")+(page.identity.session.username||app.trx("لم يسجل الدخول","Signed out"))+" · "+app.roleLabel(page.identity.session.role)}
   Repeater {model:[{ar:"إنشاء القضايا",en:"Create cases",key:"create"},{ar:"تعديل القضايا",en:"Edit cases",key:"edit"},{ar:"قراءة الأدلة",en:"Read evidence",key:"read"},{ar:"التحليل والتحقيق",en:"Analysis and investigation",key:"analyze"},{ar:"تصدير الأدلة",en:"Export evidence",key:"exportEvidence"}];delegate:Label {required property var modelData;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:page.app.design.text;text:(page.app.ar?modelData.ar:modelData.en)+" · "+(page.identity.authenticated&&lawCaseBridge.permissions[modelData.key]?page.app.trx("مسموح حسب الدور؛ الوصول للقضية شرط مستقل","Role eligible; case access checked separately"):page.app.trx("غير مسموح لهذا الدور","Not permitted for this role"))}}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("القضية المفتوحة: ","Open case: ")+(lawCaseBridge.activeCase.id||app.trx("لا توجد قضية مفتوحة","None"))+"\n"+app.trx("إدارة المستخدمين: ","User administration: ")+(page.identity.session.role==="administrator"?app.trx("مسموح","Allowed"):app.trx("غير مسموح","Not allowed"))}
  }
  GovernmentNetworkPanel {app:page.app;Layout.fillWidth:true}
 }
 }
}
