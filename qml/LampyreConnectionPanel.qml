import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "InputRules.js" as Rules
ColumnLayout {
 id:panel;objectName:"lampyreConnectionPanel"
 property var app
 property bool showSetup:true
 property bool showTools:true
 readonly property var connection:typeof lampyreConnection!=="undefined"?lampyreConnection:null
 readonly property string principal:identityBridge.session.id||""
 onPrincipalChanged:{
  if(!key||!connectionConsent||!requestConsent||!attachConsent)return
  key.clear()
  connectionConsent.checked=false
  requestConsent.checked=false
  attachConsent.checked=false
  exportMessage=""
 }
 property var toolArguments:({})
 property string inputError:""
 property var invalidFields:({})
 property string exportMessage:""
 readonly property bool catalogAvailable:!!connection&&connection.connected
 property string selectedToolName:""
 readonly property var selected:catalogAvailable?connection.tools.find(tool=>tool.name===selectedToolName)||({}):({})
 onCatalogAvailableChanged:if(!catalogAvailable)selectedToolName=""
 onSelectedToolNameChanged:{
  toolArguments=({})
  invalidFields=({})
  inputError=""
  if(requestConsent)requestConsent.checked=false
 }
 readonly property var schema:selected.inputSchema||({})
 function put(key,value){const copy=Object.assign({},panel.toolArguments);copy[key]=value;panel.toolArguments=copy;const errors=Object.assign({},invalidFields);delete errors[key];invalidFields=errors;inputError=Object.keys(errors).join(", ");requestConsent.checked=false}
 function message(code){
  const a={"audit-unavailable":"تعذر فتح سجل العمليات المحمي؛ أعد تشغيل التطبيق أو راجع صلاحية مجلد البيانات","administrator-limit-required":"يجب أن يضبط المدير سقف الإرسال من الحسابات والصلاحيات","budget-storage-error":"تعذر حفظ عداد الإرسال المحمي؛ لم يُرسل طلب البحث","local-export-permission-denied":"يلزم إذن تصدير الأدلة؛ يضبطه المدير ثم أعد الدخول","attachment-consent-required":"أكد إرفاق النتيجة غير المتحقق منها بالقضية","local-attachment-permission-denied":"يلزم إذن تعديل القضية وقراءة الأدلة لإرفاق النتيجة","case-attachment-failed":"تعذر إرفاق النتيجة؛ راجع صلاحية القضية والجهاز المختار","local-connection-permission-denied":"الحساب الحالي ممنوع من اتصال API/MCP. اطلب من المدير تفعيله في الحسابات والصلاحيات ثم سجّل الدخول مجددًا","local-search-permission-denied":"يلزم إذن تحقيق OSINT والبحث المدفوع للحساب الحالي؛ يضبطهما المدير في الحسابات والصلاحيات ثم أعد تسجيل الدخول","tls-failure":"فشل التحقق من شهادة TLS؛ راجع شهادة الخادم وساعة Windows؛ لا تتجاوز التحقق","provider-authorization-denied":"المفتاح لا يمنح صلاحية المزود المطلوبة (HTTP 403)","provider-service-error":"خطأ في خدمة المزود؛ لم يُعد التطبيق الطلب تلقائيًا","unknown-cost-policy-required":"التكلفة غير معروفة. اطلب إذن العمليات مجهولة التكلفة من الإدارة؛ يجب أن يوافق مدير مستقل ثم تفعّله بحسابك","spending-limit-reached":"تم بلوغ حد الإرسال الذي حُدد مسبقًا؛ راجع حد الجلسة أو حد المؤسسة","active-case-required":"افتح قضية مسموحًا لهذا الحساب بالوصول إليها قبل إرسال البحث","invalid-request-limit":"اختر حدًا لعدد الطلبات بين صفر و100","disconnected":"لم يتم إعداد الاتصال بعد","configured":"تم حفظ الإعدادات؛ اختبر الاتصال","connecting":"جارٍ التحقق من الاتصال وقراءة الأدوات…","connected":"متصل بخادم MCP؛ لم يُنفّذ بحث تلقائي","running":"جارٍ تنفيذ الأداة التي اخترتها…","completed":"وصلت نتيجة من المزود","provider-tool-error":"أعاد المزود نتيجة بخطأ؛ راجع النتيجة","cancelled-local":"توقف انتظار النتيجة محليًا؛ قد يستمر الطلب لدى المزود","sign-in-required":"سجّل الدخول أولًا لحفظ إعدادات حسابك","invalid-endpoint":"تعذر إعداد اتصال الاستخبارات؛ راجع دعم التطبيق","invalid-key":"أدخل مفتاح API صالحًا دون مسافات أو أسطر جديدة","configuration-required":"أدخل مفتاح API ثم احفظ الإعدادات","connection-consent-required":"أكد الاتصال بالمزود أولًا","request-consent-required":"أكد تنفيذ الطلب وتكلفته قبل الإرسال","authentication-failed":"رفض المزود المصادقة؛ راجع المفتاح وصلاحيات الحساب","payment-required":"رفض المزود الطلب بسبب الدفع أو الرصيد؛ راجع حسابك لديه","rate-limited":"حد الطلبات لدى المزود؛ أعد المحاولة لاحقًا","redirect-refused":"رُفض تحويل الرابط لحماية المفتاح؛ راجع دعم التطبيق","endpoint-or-session-expired":"الرابط غير متاح أو انتهت الجلسة؛ اختبر الاتصال مجددًا","network-error":"تعذر الاتصال بالخادم؛ راجع اتصال الإنترنت","timeout":"انتهت مهلة انتظار المزود","response-too-large":"تجاوزت الاستجابة الحد المدعوم","unsupported-protocol":"إصدار MCP لدى الخادم غير مدعوم","invalid-response":"استجابة الخادم لا تطابق MCP المدعوم","invalid-catalog":"كتالوج الأدوات غير صالح أو تجاوز الحد","catalog-limit":"تجاوز كتالوج الأدوات حد الصفحات","invalid-session":"معرّف جلسة الخادم غير صالح","provider-rpc-error":"رفض المزود الطلب؛ راجع الأداة والمعاملات وصلاحيات الحساب","invalid-arguments":"اختر أداة وأدخل معاملاتها بالشكل المطلوب","investigator-required":"استخدم حساب محقق أو مراجع مخوّل لتنفيذ البحث؛ حساب المدير لإدارة الحسابات فقط","not-connected":"اختبر الاتصال أولًا قبل تنفيذ أداة","external-analysis-approval-required":"يلزم تفعيل صلاحية التحليل الخارجي من الإدارة للاتصال؛ وتنفيذ البحث يحتاج حسابًا مخوّلًا","policy-revoked":"انتهى التفعيل أو سُحبت صلاحية الاتصال؛ لم تُعتمد نتيجة الطلب","protected-storage-required":"تعذر حفظ المفتاح محميًا؛ التخزين الدائم يتطلب حماية Windows","credential-read":"تعذر قراءة بيانات الاتصال المحمية؛ أعد إدخالها","export-consent-required":"اختر حفظ نسخة لتصدير النتيجة","no-result":"لا توجد نتيجة للحفظ","choose-new-file":"اختر اسم ملف جديد؛ لن يُستبدل ملف موجود","export-write":"تعذر حفظ الملف في المكان المحدد","credential-write":"تعذر حفظ أو حذف بيانات الاتصال المحمية"};
  return app.ar?(a[code]||code):code.replace(/-/g," ");
 }
 OsintDesktopPane {
  Layout.fillWidth:true;visible:panel.showSetup;title:panel.app.trx("إعداد استخبارات نيكسفاري","NEXVARY Intelligence Setup");iconName:"settings"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.text;text:panel.app.trx("أدخل مفتاح API الخاص بخدمات الاستخبارات. عنوان الاتصال مضبوط داخليًا؛ كلمة مرور التطبيق أو مفتاح ترخيصه ليسا مفتاح API.","Enter your Intelligence API key. The connection is configured internally; an application password or desktop license is not an API key.")}
  Label {objectName:"lampyreLocalChecks";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.text;text:panel.app.trx("عمل مباشر — لا حساب تطبيق ولا موافقة من حساب آخر. المفتاح محمي بملف مستخدم Windows.","Direct work — no application account or approval by another account. The key is protected by your Windows user profile.") }
  Label {objectName:"lampyreCredentialSaved";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.connection&&panel.connection.credentialSaved?panel.app.trx("المفتاح محفوظ محميًا لهذا الحساب","Key saved in protected storage for this account"):panel.app.trx("لا يوجد مفتاح محفوظ على القرص لهذا الحساب","No key saved on disk for this account")}
  Label {text:"API key";color:panel.app.design.muted}
  EntryField {id:key;validationKind:"apiKey";configuredValue:!!panel.connection&&panel.connection.hasCredential;objectName:"lampyreApiKey";Layout.fillWidth:true;maximumLength:4096;echoMode:TextInput.Password;LayoutMirroring.enabled:false;selectByMouse:true;color:panel.app.design.text;placeholderTextColor:panel.app.design.muted;placeholderText:panel.connection&&panel.connection.hasCredential?panel.app.trx("مفتاح محفوظ؛ اترك الحقل فارغًا للاحتفاظ به","Saved key; leave empty to retain it"):panel.app.trx("أدخل مفتاح API","Enter API key");background:Rectangle {color:panel.app.design.surface;border.color:panel.app.design.border;radius:5} enabled:!panel.connection||!panel.connection.busy}
  NeonCheckBox {id:remember;objectName:"lampyreRememberKey";checked:true;text:panel.app.trx("حفظ المفتاح محميًا لهذا الحساب على Windows","Remember a protected key for this account on Windows")}
  GridLayout {Layout.fillWidth:true;columns:width>650?3:1;columnSpacing:8;rowSpacing:8
   OsintDesktopCommand {objectName:"lampyreSaveConnection";text:panel.app.trx("حفظ الإعدادات","Save settings");enabled:!!panel.connection&&!panel.connection.busy;onClicked:{panel.connection.configureCredential(key.text,remember.checked);key.clear()}}
   OsintDesktopCommand {objectName:"lampyreForgetConnection";text:panel.app.trx("حذف بيانات الاتصال","Forget connection");enabled:!!panel.connection&&!panel.connection.busy&&panel.connection.hasCredential;onClicked:{panel.connection.forget();key.clear()}}
  }
  NeonCheckBox {id:connectionConsent;objectName:"lampyreConnectionConsent";text:panel.app.trx("أوافق على الاتصال بهذا الخادم للتحقق من المفتاح وجلب قائمة الأدوات","Connect to this server to validate the key and retrieve its tool list")}
  RowLayout {Layout.fillWidth:true;spacing:8
   OsintDesktopCommand {objectName:"lampyreTestConnection";primary:true;text:panel.app.trx("اختبار الاتصال وجلب الأدوات","Test connection & load tools");enabled:!!panel.connection&&panel.connection.hasCredential&&!panel.connection.busy&&connectionConsent.checked;onClicked:{panel.connection.connectProvider(connectionConsent.checked);connectionConsent.checked=false}}
   OsintDesktopCommand {objectName:"lampyreStopWaiting";visible:!!panel.connection&&panel.connection.busy;text:panel.app.trx("إيقاف الانتظار","Stop waiting");onClicked:panel.connection.cancel()}
  }
  Label {objectName:"lampyreConnectionStatus";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.connection&&panel.connection.errorCode!==""?panel.app.design.danger:panel.app.design.accent;text:panel.connection?panel.message(panel.connection.errorCode||panel.connection.state):panel.app.trx("موصل الاتصال غير متاح","Connection client unavailable")}
 }
 OsintDesktopPane {
  Layout.fillWidth:true;visible:panel.showTools&&panel.catalogAvailable;title:panel.app.trx("خدمات استخبارات نيكسفاري","NEXVARY Intelligence Services");iconName:"search_visual"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("ابدأ البحث من بطاقة الفئة لربطه بفاتورة نيكسفاري. قيمة الفاتورة المستحقة منفصلة عن تكلفة الاتصال الداخلية، ولا تُعتبر مدفوعة تلقائيًا.","Start searches from a category card to link the NEXVARY invoice. The invoice due is separate from internal connection cost and is never automatically marked paid.")}

  Label {
   visible:catalog.count===0;Layout.fillWidth:true;wrapMode:Text.WordWrap
   color:panel.app.design.muted;text:panel.app.trx("لم يُرجع الخادم أدوات متاحة لهذا الحساب.","The server returned no tools for this account.")
  }
  GridView {
   id:catalog;objectName:"lampyreToolCatalog";Layout.fillWidth:true;clip:true
   visible:count>0
   Layout.preferredHeight:Math.min(3,Math.ceil(count/columns))*cellHeight
   readonly property int columns:width>=600?2:1
   cellWidth:Math.floor(width/columns);cellHeight:106
   model:panel.catalogAvailable?panel.connection.tools:[]
   layoutDirection:panel.app.ar?Qt.RightToLeft:Qt.LeftToRight
   boundsBehavior:Flickable.StopAtBounds
   ScrollBar.vertical:ScrollBar {}
   delegate:Button {
    id:toolCard
    required property var modelData
    width:catalog.cellWidth-12;height:catalog.cellHeight-8
    padding:10;hoverEnabled:true;focusPolicy:Qt.StrongFocus
    checked:panel.selectedToolName===modelData.name
    text:panel.connection.displayText(modelData.title||modelData.name)
    Accessible.name:text
    Accessible.description:panel.connection.displayText(modelData.description||"")
    enabled:!panel.connection.busy
    onClicked:panel.selectedToolName=modelData.name
    background:Rectangle {
     radius:8;color:toolCard.checked||toolCard.hovered?panel.app.design.raised:panel.app.design.surface
     border.width:toolCard.activeFocus?2:1
     border.color:toolCard.checked||toolCard.activeFocus?panel.app.design.accent:panel.app.design.border
    }
    contentItem:RowLayout {
     SemanticIcon {iconSource:"qrc:/assets/icons-professional/search_visual.svg";Layout.preferredWidth:30;Layout.preferredHeight:30;fillMode:Image.PreserveAspectFit}
     ColumnLayout {
      Layout.fillWidth:true;Layout.minimumWidth:0;spacing:3
      Label {Layout.fillWidth:true;text:toolCard.text;textFormat:Text.PlainText;color:panel.app.design.text;font.bold:true;elide:Text.ElideRight}
      Label {Layout.fillWidth:true;text:panel.connection.displayText(toolCard.modelData.description||"");textFormat:Text.PlainText;color:panel.app.design.muted;wrapMode:Text.Wrap;maximumLineCount:2;elide:Text.ElideRight}
     }
    }
   }
  }
  Label {visible:!!panel.selected.name;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.text;text:panel.connection.displayText(panel.selected.description||"")}
  Repeater {model:Object.keys(panel.schema.properties||{}).map(name=>({name:name,field:panel.schema.properties[name]}))
   delegate:ColumnLayout {id:parameterField;required property var modelData;readonly property string name:modelData.name;readonly property var field:modelData.field||({});Layout.fillWidth:true
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:panel.app.design.text;text:panel.connection.displayText(parameterField.field.title||parameterField.name)+((panel.schema.required||[]).indexOf(parameterField.name)>=0?" *":"")}
    EntryField {validationKind:parameterField.field.type==="integer"?"integer":parameterField.field.type==="number"?"number":"text";validateInput:function(value){return !panel.invalidFields[parameterField.name]};Layout.fillWidth:true;maximumLength:8192;placeholderText:panel.connection.displayText(parameterField.field.description||parameterField.field.type||"");color:panel.app.design.text;placeholderTextColor:panel.app.design.muted;background:Rectangle {color:panel.app.design.surface;border.color:panel.app.design.border;radius:5} onTextEdited:{
      try {let value=text;if(parameterField.field.type==="integer"||parameterField.field.type==="number"){value=Number(Rules.digits(text));if(!text.trim()||!isFinite(value)||parameterField.field.type==="integer"&&!Number.isInteger(value))throw new Error("number")}
       else if(parameterField.field.type==="boolean"){if(text!=="true"&&text!=="false")throw new Error("boolean");value=text==="true"}
       else if(parameterField.field.type==="array"||parameterField.field.type==="object")value=JSON.parse(text);
       if(parameterField.field.enum&&parameterField.field.enum.indexOf(value)<0)throw new Error("enum");panel.put(parameterField.name,value)
      }catch(e){const args=Object.assign({},panel.toolArguments);delete args[parameterField.name];panel.toolArguments=args;const errors=Object.assign({},panel.invalidFields);errors[parameterField.name]=true;panel.invalidFields=errors;panel.inputError=panel.app.trx("راجع نوع قيمة الحقول: ","Check field value types: ")+Object.keys(errors).join(", ");requestConsent.checked=false}
     }}
   }
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.danger;text:panel.connection?panel.connection.displayText(panel.inputError):panel.inputError;visible:panel.inputError!==""}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("التكلفة والرصيد غير مقروءين تلقائيًا. راجعهما في حساب المزود قبل التنفيذ؛ لا تعني قائمة الأدوات أن البحث مجاني.","Cost and balance are not read automatically. Check them in the provider account before execution; tool discovery does not make a search free.")}
  Label {objectName:"lampyrePhotonBalance";Layout.fillWidth:true;color:panel.app.design.danger;text:panel.app.trx("رصيد استخبارات نيكسفاري: غير متاح — تكلفة العملية غير معروفة","NEXVARY Intelligence Credits: balance unavailable — operation cost unknown")}
  RowLayout {Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.text;text:panel.app.trx("حد اختياري للإرسالات؛ صفر بلا حد إضافي (ليس حدًا ماليًا)","Optional submission limit; zero adds no ceiling (not a monetary limit)")}
   SpinBox {id:requestLimit;from:0;to:100;value:0;editable:true}
   OsintDesktopCommand {text:panel.app.trx("تطبيق الحد","Apply limit");onClicked:panel.connection.setRequestLimit(requestLimit.value)}
  }
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("عدد الإرسالات: ","Submitted requests: ")+(panel.connection?panel.connection.submittedRequests:0)+" / "+(panel.connection?panel.connection.requestLimit:0)+"\n"+panel.app.trx("لا يلزم حساب آخر أو تفعيل إداري. يلزم تأكيدك لكل إرسال قد يخصم رصيدًا؛ لا يُعاد الإرسال تلقائيًا.","No separate account or administrative activation is required. Confirm every potentially billable submission; failed requests are never retried automatically.")}
  NeonCheckBox {id:requestConsent;objectName:"lampyrePaidRequestConsent";Layout.fillWidth:true;text:panel.app.trx("أوافق على إرسال معاملات الأداة المحددة وأي خصم وفق حسابي لدى المزود","Send the selected tool arguments and accept any charge under my provider account")}
  OsintDesktopCommand {objectName:"lampyreExecuteTool";primary:true;text:panel.app.trx("تنفيذ الأداة المحددة","Run selected tool");enabled:!!panel.connection&&panel.connection.connected&&!panel.connection.busy&&!!panel.selected.name&&requestConsent.checked&&panel.inputError==="";onClicked:{
    const required=panel.schema.required||[];for(const key of required)if(panel.toolArguments[key]===undefined||panel.toolArguments[key]===""){panel.inputError=panel.app.trx("أكمل الحقل المطلوب: ","Complete required field: ")+key;requestConsent.checked=false;return}
    panel.connection.callTool(panel.selected.name,JSON.stringify(panel.toolArguments),requestConsent.checked);requestConsent.checked=false
   }}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("احفظ المفتاح واختبر الاتصال، ثم اختر الأداة وأكد تنفيذها. ربط النتائج بقضية اختياري.","Save the key and test the connection, then select and confirm an operation. Linking results to a case is optional.")}
 }
 OsintDesktopPane {
  Layout.fillWidth:true;visible:!!panel.connection&&panel.connection.resultText!=="";title:panel.app.trx("نتيجة المزود","Provider result");iconName:"document_text"
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("النتيجة لهذه الجلسة. احفظ نسخة عند الحاجة؛ لا تُرفق بقضية تلقائيًا.","This result belongs to the current session. Save a copy when needed; it is not automatically attached to a case.")}
  ComboBox {id:caseDevice;Layout.fillWidth:true;model:lawCaseBridge.devices;textRole:"model";visible:lawCaseBridge.activeCase.id!==undefined}
  NeonCheckBox {id:attachConsent;Layout.fillWidth:true;text:panel.app.trx("أوافق على إرفاق النتيجة كدليل غير متحقق منه بالقضية الأصلية","Attach result as unverified evidence to its original case")}
  OsintDesktopCommand {objectName:"lampyreAttachResult";text:panel.app.trx("إرفاق النتيجة بالقضية","Attach result to case");enabled:attachConsent.checked&&caseDevice.currentIndex>=0;onClicked:{const device=lawCaseBridge.devices[caseDevice.currentIndex]||({});const id=panel.connection.attachResult(device.id||"",attachConsent.checked);attachConsent.checked=false;panel.exportMessage=id!==""?panel.app.trx("أُرفقت النتيجة مع سجل تدقيق؛ محتواها غير متحقق منه","Result attached with an audit record; contents remain unverified"):panel.message(panel.connection.errorCode)}}
  OsintDesktopCommand {objectName:"lampyreExportResult";text:panel.app.trx("حفظ نسخة من النتيجة JSON","Save result copy as JSON");onClicked:{panel.exportMessage="";resultFile.open()}}
  Label {visible:panel.exportMessage!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.accent;text:panel.exportMessage}
  Repeater {model:panel.connection?panel.connection.resultItems:[];delegate:EntryArea {required property var modelData;Layout.fillWidth:true;Layout.preferredHeight:Math.min(360,Math.max(90,implicitHeight));readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:modelData.text;color:panel.app.design.text;background:Rectangle {color:panel.app.design.surface}}}
  NeonCheckBox {id:raw;objectName:"lampyreRawResponse";text:panel.app.trx("عرض الاستجابة التقنية الكاملة","Show full technical response")}
  EntryArea {visible:raw.checked;Layout.fillWidth:true;Layout.preferredHeight:240;readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:panel.connection?panel.connection.resultText:"";color:panel.app.design.text;background:Rectangle {color:panel.app.design.surface}}
 }
 FileDialog {id:resultFile;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:{const ok=panel.connection.exportResult(selectedFile.toString(),true);panel.exportMessage=ok?panel.app.trx("حُفظت نسخة النتيجة؛ الملف غير مشفر","Result copy saved; the file is not encrypted"):panel.message(panel.connection.errorCode)}}
}
