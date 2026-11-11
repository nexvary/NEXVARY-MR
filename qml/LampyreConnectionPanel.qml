import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ColumnLayout {
 id:panel;objectName:"lampyreConnectionPanel"
 property var app
 readonly property var connection:typeof lampyreConnection!=="undefined"?lampyreConnection:null
 property var toolArguments:({})
 property string inputError:""
 property var invalidFields:({})
 property string exportMessage:""
 readonly property var selected:connection&&toolSelector.currentIndex>=0?connection.tools[toolSelector.currentIndex]||({}):({})
 readonly property var schema:selected.inputSchema||({})
 function put(key,value){const copy=Object.assign({},panel.toolArguments);copy[key]=value;panel.toolArguments=copy;const errors=Object.assign({},invalidFields);delete errors[key];invalidFields=errors;inputError=Object.keys(errors).join(", ");requestConsent.checked=false}
 function message(code){
  const a={"disconnected":"لم يتم إعداد الاتصال بعد","configured":"تم حفظ الإعدادات؛ اختبر الاتصال","connecting":"جارٍ التحقق من الاتصال وقراءة الأدوات…","connected":"متصل بخادم MCP؛ لم يُنفّذ بحث تلقائي","running":"جارٍ تنفيذ الأداة التي اخترتها…","completed":"وصلت نتيجة من المزود","provider-tool-error":"أعاد المزود نتيجة بخطأ؛ راجع النتيجة","cancelled-local":"توقف انتظار النتيجة محليًا؛ قد يستمر الطلب لدى المزود","sign-in-required":"سجّل الدخول أولًا لحفظ إعدادات حسابك","invalid-endpoint":"أدخل رابط HTTPS كاملًا لخادم MCP دون مفتاح داخل الرابط","invalid-key":"أدخل مفتاح API صالحًا دون مسافات أو أسطر جديدة","configuration-required":"أدخل رابط MCP ومفتاح API ثم احفظ الإعدادات","connection-consent-required":"أكد الاتصال بالمزود أولًا","request-consent-required":"أكد تنفيذ الطلب وتكلفته قبل الإرسال","authentication-failed":"رفض المزود المصادقة؛ راجع المفتاح وصلاحيات الحساب","payment-required":"رفض المزود الطلب بسبب الدفع أو الرصيد؛ راجع حسابك لديه","rate-limited":"حد الطلبات لدى المزود؛ أعد المحاولة لاحقًا","redirect-refused":"رُفض تحويل الرابط لحماية المفتاح؛ استخدم عنوان MCP النهائي","endpoint-or-session-expired":"الرابط غير متاح أو انتهت الجلسة؛ اختبر الاتصال مجددًا","network-error":"تعذر الاتصال بالخادم؛ راجع الإنترنت والعنوان","timeout":"انتهت مهلة انتظار المزود","response-too-large":"تجاوزت الاستجابة الحد المدعوم","unsupported-protocol":"إصدار MCP لدى الخادم غير مدعوم","invalid-response":"استجابة الخادم لا تطابق MCP المدعوم","invalid-catalog":"كتالوج الأدوات غير صالح أو تجاوز الحد","catalog-limit":"تجاوز كتالوج الأدوات حد الصفحات","invalid-session":"معرّف جلسة الخادم غير صالح","provider-rpc-error":"رفض المزود الطلب؛ راجع الأداة والمعاملات وصلاحيات الحساب","invalid-arguments":"اختر أداة وأدخل معاملاتها بالشكل المطلوب","investigator-required":"استخدم حساب محقق أو مراجع مخوّل لتنفيذ البحث؛ حساب المدير لإدارة الحسابات فقط","not-connected":"اختبر الاتصال أولًا قبل تنفيذ أداة","external-analysis-approval-required":"يلزم تفعيل صلاحية التحليل الخارجي من الإدارة للاتصال؛ وتنفيذ البحث يحتاج حسابًا مخوّلًا","policy-revoked":"انتهى التفعيل أو سُحبت صلاحية الاتصال؛ لم تُعتمد نتيجة الطلب","protected-storage-required":"تعذر حفظ المفتاح محميًا؛ التخزين الدائم يتطلب حماية Windows","credential-read":"تعذر قراءة بيانات الاتصال المحمية؛ أعد إدخالها","export-consent-required":"اختر حفظ نسخة لتصدير النتيجة","no-result":"لا توجد نتيجة للحفظ","choose-new-file":"اختر اسم ملف جديد؛ لن يُستبدل ملف موجود","export-write":"تعذر حفظ الملف في المكان المحدد","credential-write":"تعذر حفظ أو حذف بيانات الاتصال المحمية"};
  return app.ar?(a[code]||code):code.replace(/-/g," ");
 }
 OsintDesktopPane {
  Layout.fillWidth:true;title:panel.app.trx("حساب Lampyre واتصال MCP","Lampyre account & MCP connection");iconName:"settings"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.text;text:panel.app.trx("أدخل رابط MCP ومفتاح API من إعدادات حساب Lampyre. كلمة مرور عميل Windows ومفتاح ترخيصه ليسا بديلًا عن بيانات اتصال MCP.","Enter the MCP URL and API key from your Lampyre account settings. A Windows desktop password or license key is not an MCP credential.")}
  Label {text:panel.app.trx("رابط خادم MCP","MCP server URL");color:panel.app.design.muted}
  TextField {id:endpoint;objectName:"lampyreMcpEndpoint";Layout.fillWidth:true;text:panel.connection?panel.connection.endpoint:"";placeholderText:panel.app.trx("الصق الرابط الكامل من حسابك","Paste the full URL from your account");maximumLength:2048;selectByMouse:true;LayoutMirroring.enabled:false;color:panel.app.design.text;placeholderTextColor:panel.app.design.muted;background:Rectangle {color:panel.app.design.surface;border.color:panel.app.design.border;radius:5} enabled:!panel.connection||!panel.connection.busy}
  Label {text:"API key";color:panel.app.design.muted}
  TextField {id:key;objectName:"lampyreApiKey";Layout.fillWidth:true;maximumLength:4096;echoMode:TextInput.Password;LayoutMirroring.enabled:false;selectByMouse:true;color:panel.app.design.text;placeholderTextColor:panel.app.design.muted;placeholderText:panel.connection&&panel.connection.hasCredential?panel.app.trx("مفتاح محفوظ؛ اترك الحقل فارغًا للاحتفاظ به","Saved key; leave empty to retain it"):panel.app.trx("أدخل مفتاح API","Enter API key");background:Rectangle {color:panel.app.design.surface;border.color:panel.app.design.border;radius:5} enabled:!panel.connection||!panel.connection.busy}
  CheckBox {id:remember;objectName:"lampyreRememberKey";checked:true;text:panel.app.trx("حفظ المفتاح محميًا لهذا الحساب على Windows","Remember a protected key for this account on Windows")}
  Flow {Layout.fillWidth:true;spacing:8
   OsintDesktopCommand {objectName:"lampyreSaveConnection";text:panel.app.trx("حفظ الإعدادات","Save settings");enabled:!!panel.connection&&!panel.connection.busy;onClicked:{panel.connection.configure(endpoint.text.trim(),key.text,remember.checked);key.clear()}}
   OsintDesktopCommand {objectName:"lampyreForgetConnection";text:panel.app.trx("حذف بيانات الاتصال","Forget connection");enabled:!!panel.connection&&!panel.connection.busy&&panel.connection.hasCredential;onClicked:{panel.connection.forget();key.clear();endpoint.clear()}}
   OsintDesktopCommand {text:panel.app.trx("فتح حساب Lampyre","Open Lampyre account");onClicked:panel.app.openExternal("https://account.lampyre.io/")}
  }
  CheckBox {id:connectionConsent;objectName:"lampyreConnectionConsent";text:panel.app.trx("أوافق على الاتصال بهذا الخادم للتحقق من المفتاح وجلب قائمة الأدوات","Connect to this server to validate the key and retrieve its tool list")}
  Flow {Layout.fillWidth:true;spacing:8
   OsintDesktopCommand {objectName:"lampyreTestConnection";primary:true;text:panel.app.trx("اختبار الاتصال وجلب الأدوات","Test connection & load tools");enabled:!!panel.connection&&panel.connection.hasCredential&&!panel.connection.busy&&connectionConsent.checked;onClicked:{panel.connection.connectProvider(connectionConsent.checked);connectionConsent.checked=false}}
   OsintDesktopCommand {objectName:"lampyreStopWaiting";visible:!!panel.connection&&panel.connection.busy;text:panel.app.trx("إيقاف الانتظار","Stop waiting");onClicked:panel.connection.cancel()}
  }
  Label {objectName:"lampyreConnectionStatus";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.connection&&panel.connection.errorCode!==""?panel.app.design.danger:panel.app.design.accent;text:panel.connection?panel.message(panel.connection.errorCode||panel.connection.state):panel.app.trx("موصل الاتصال غير متاح","Connection client unavailable")}
 }
 OsintDesktopPane {
  Layout.fillWidth:true;visible:!!panel.connection&&panel.connection.connected;title:panel.app.trx("أدوات حسابك","Your account tools");iconName:"search_visual"
  ComboBox {id:toolSelector;objectName:"lampyreToolSelector";Layout.fillWidth:true;model:panel.connection?panel.connection.tools:[];textRole:"name";onCurrentIndexChanged:{panel.toolArguments=({});panel.invalidFields=({});requestConsent.checked=false;panel.inputError=""}}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.muted;text:panel.selected.description||""}
  Repeater {model:Object.keys(panel.schema.properties||{})
   delegate:ColumnLayout {id:parameterField;required property string modelData;readonly property var field:(panel.schema.properties||{})[parameterField.modelData]||({});Layout.fillWidth:true
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.text;text:(parameterField.field.title||parameterField.modelData)+((panel.schema.required||[]).indexOf(parameterField.modelData)>=0?" *":"")}
    TextField {Layout.fillWidth:true;maximumLength:8192;placeholderText:parameterField.field.description||parameterField.field.type||"";color:panel.app.design.text;placeholderTextColor:panel.app.design.muted;background:Rectangle {color:panel.app.design.surface;border.color:panel.app.design.border;radius:5} onTextEdited:{
      try {let value=text;if(parameterField.field.type==="integer"||parameterField.field.type==="number"){value=Number(text);if(!text.trim()||!isFinite(value)||parameterField.field.type==="integer"&&!Number.isInteger(value))throw new Error("number")}
       else if(parameterField.field.type==="boolean"){if(text!=="true"&&text!=="false")throw new Error("boolean");value=text==="true"}
       else if(parameterField.field.type==="array"||parameterField.field.type==="object")value=JSON.parse(text);
       if(parameterField.field.enum&&parameterField.field.enum.indexOf(value)<0)throw new Error("enum");panel.put(parameterField.modelData,value)
      }catch(e){const args=Object.assign({},panel.toolArguments);delete args[parameterField.modelData];panel.toolArguments=args;const errors=Object.assign({},panel.invalidFields);errors[parameterField.modelData]=true;panel.invalidFields=errors;panel.inputError=panel.app.trx("راجع نوع قيمة الحقول: ","Check field value types: ")+Object.keys(errors).join(", ");requestConsent.checked=false}
     }}
   }
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.danger;text:panel.inputError;visible:panel.inputError!==""}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("التكلفة والرصيد غير مقروءين تلقائيًا. راجعهما في حساب المزود قبل التنفيذ؛ لا تعني قائمة الأدوات أن البحث مجاني.","Cost and balance are not read automatically. Check them in the provider account before execution; tool discovery does not make a search free.")}
  CheckBox {id:requestConsent;objectName:"lampyrePaidRequestConsent";Layout.fillWidth:true;text:panel.app.trx("أوافق على إرسال معاملات الأداة المحددة وأي خصم وفق حسابي لدى المزود","Send the selected tool arguments and accept any charge under my provider account")}
  OsintDesktopCommand {objectName:"lampyreExecuteTool";primary:true;text:panel.app.trx("تنفيذ الأداة المحددة","Run selected tool");enabled:!!panel.connection&&panel.connection.connected&&!panel.connection.busy&&toolSelector.currentIndex>=0&&requestConsent.checked&&panel.inputError==="";onClicked:{
    const required=panel.schema.required||[];for(const key of required)if(panel.toolArguments[key]===undefined||panel.toolArguments[key]===""){panel.inputError=panel.app.trx("أكمل الحقل المطلوب: ","Complete required field: ")+key;requestConsent.checked=false;return}
    panel.connection.callTool(panel.selected.name,JSON.stringify(panel.toolArguments),requestConsent.checked);requestConsent.checked=false
   }}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("تنفيذ البحث يتطلب حسابًا مخوّلًا وتفعيل التحليل الخارجي من الإدارة؛ حفظ المفتاح وحده لا يفعّل البحث.","Search execution requires an authorized account and external-analysis activation in Administration; saving a key does not activate searches.")}
 }
 OsintDesktopPane {
  Layout.fillWidth:true;visible:!!panel.connection&&panel.connection.resultText!=="";title:panel.app.trx("نتيجة المزود","Provider result");iconName:"document_text"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("النتيجة لهذه الجلسة. احفظ نسخة عند الحاجة؛ لا تُرفق بقضية تلقائيًا.","This result belongs to the current session. Save a copy when needed; it is not automatically attached to a case.")}
  OsintDesktopCommand {objectName:"lampyreExportResult";text:panel.app.trx("حفظ نسخة من النتيجة JSON","Save result copy as JSON");onClicked:{panel.exportMessage="";resultFile.open()}}
  Label {visible:panel.exportMessage!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.accent;text:panel.exportMessage}
  Repeater {model:panel.connection?panel.connection.resultItems:[];delegate:TextArea {required property var modelData;Layout.fillWidth:true;Layout.preferredHeight:Math.min(360,Math.max(90,implicitHeight));readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:modelData.text;color:panel.app.design.text;background:Rectangle {color:panel.app.design.surface}}}
  CheckBox {id:raw;objectName:"lampyreRawResponse";text:panel.app.trx("عرض الاستجابة التقنية الكاملة","Show full technical response")}
  TextArea {visible:raw.checked;Layout.fillWidth:true;Layout.preferredHeight:240;readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:panel.connection?panel.connection.resultText:"";color:panel.app.design.text;background:Rectangle {color:panel.app.design.surface}}
 }
 FileDialog {id:resultFile;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:{const ok=panel.connection.exportResult(selectedFile.toString(),true);panel.exportMessage=ok?panel.app.trx("حُفظت نسخة النتيجة؛ الملف غير مشفر","Result copy saved; the file is not encrypted"):panel.message(panel.connection.errorCode)}}
}
