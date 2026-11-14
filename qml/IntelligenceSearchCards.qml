import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "IntelligenceCategories.js" as Categories
import "InputRules.js" as Rules
ColumnLayout {
    id:search
    objectName:"intelligenceSearchCards"
    property var app
    property var connection:typeof lampyreConnection!=="undefined"?lampyreConnection:null
    signal setupRequested()
    readonly property string retailPrice:"€"+(connection?connection.retailPriceEur:14).toFixed(2)
    property bool showSearchResult:false
    property string selectedCard:""
    property bool autoCatalogRequested:false
    property string selectedService:""
    property bool specializedSearch:false
    readonly property bool unifiedSearch:!specializedSearch&&["email","phone","username"].indexOf(selectedCard)>=0
    property var values:({})
    property string validationError:""
    readonly property var currentInvoice:connection&&connection.currentInvoice?connection.currentInvoice:({})
    readonly property var service:connection?(connection.services.find(row=>row.tool_name===selectedService)||{}):({})
    readonly property var fields:unifiedSearch?[{name:selectedCard,type:"string"}]:service.args?Categories.fields(service.args):["email","phone","username","domain","ip"].indexOf(selectedCard)>=0?[{name:selectedCard,type:"string"}]:[]
    readonly property var categories:{const rows=connection&&connection.categories.length>0?connection.categories.slice():Object.keys(Categories.titles).map(name=>({name:name}));for(const name of ["email","phone","username"])if(!rows.some(row=>row.name===name))rows.push({name:name});return rows}
    readonly property var orderedCategories:{const priority=["email","phone","username","person","domain","ip","image","company_name"];return categories.slice().sort((a,b)=>{const x=priority.indexOf(a.name),y=priority.indexOf(b.name);return (x<0?99:x)-(y<0?99:y)||a.name.localeCompare(b.name)})}
    property string filter:""
    readonly property var shownCategories:orderedCategories.filter(row=>!filter||Categories.title(row.name,app.ar).toLowerCase().indexOf(filter.toLowerCase())>=0)
    function errorMessage(code){
        const ar={"unified-connection-required":"اختبر اتصال البحث الموحد من إعداد الاتصال أولًا؛ نجاح الخدمات المتخصصة لا يكفي.","invalid-lookup-response":"تعذر تأكيد استجابة البحث الموحد. قد يكون قد خُصم؛ لا تعِد إرساله تلقائيًا.","provider-payment-required":"بعض المصادر تتطلب صلاحية أو دفعًا إضافيًا؛ لم نُرسل طلب دفع آخر.","results-pending":"الطلب السابق قيد المعالجة؛ اجلب نتيجته دون إعادة البحث.","provider-task-error":"أفادت الخدمة بفشل الفحص. راجع النتيجة؛ لم نُعد تشغيله.","result-collection-error":"تعذر جمع النتيجة النهائية. قد يكون البحث قد خُصم بالفعل؛ لا تعِد إرساله.","invalid-task-response":"استجابة النتيجة غير مكتملة أو غير مدعومة؛ لا تعِد إرسال البحث المدفوع.","invoice-storage-error":"تعذر حفظ الفاتورة محمية؛ راجع صلاحية مجلد البيانات قبل إعادة التنفيذ.","retail-invoice-required":"نفّذ البحث من بطاقة الفئة لربطه بفاتورته.","quote-expired":"انتهت صلاحية بيانات الخدمة أو تغيرت؛ حدّث الفئة واختر الخدمة مجددًا.","unsupported-service-form":"هذه الخدمة تحتاج مدخلات مركبة؛ استخدم الإعدادات المتقدمة.","metadata-not-free":"لم يؤكد دليل الخدمة أن قراءة هذه البيانات مجانية؛ لم يُرسل الطلب.","invalid-category":"حدّث دليل فئات البحث واختر بطاقة متاحة.","metadata-response-error":"تعذر قراءة دليل الخدمات؛ حدّث الاتصال.","invalid-arguments":"راجع البيانات المطلوبة وصيغتها قبل التنفيذ.","payment-required":"رصيد الاتصال الداخلي غير كافٍ لتنفيذ الخدمة.","not-connected":"افتح إعداد الاتصال واحفظ المفتاح ثم اختبر الاتصال.","request-consent-required":"أكد سعر البحث وإرسال البيانات قبل التنفيذ."};
        return app.ar?(ar[code]||"تعذر إتمام العملية؛ راجع حالة الاتصال والإعدادات."):code.replace(/-/g," ");
    }
    function clearSelection(){showSearchResult=false;selectedService="";values=({});validationError="";paidConsent.checked=false}
    onSelectedServiceChanged:{showSearchResult=false;const keep={};for(const field of fields)if(values[field.name])keep[field.name]=values[field.name];values=keep;validationError="";paidConsent.checked=false}
    Connections {target:search.connection;function onChanged(){
        if(!search.connection.connected&&!search.connection.lookupConnected&&!(search.connection.pendingResultCount>0)){search.autoCatalogRequested=false;search.clearSelection();return}
        if(search.connection.connected&&search.connection.liveProfile&&!search.connection.busy&&search.connection.categories.length===0&&!search.autoCatalogRequested){search.autoCatalogRequested=true;search.connection.refreshCatalog();return}
        if(!search.unifiedSearch&&search.connection.connected&&search.selectedCard&&!search.connection.busy&&search.connection.categories.some(row=>row.name===search.selectedCard)&&search.connection.selectedCategory!==search.selectedCard)search.connection.discoverServices(search.selectedCard)
        if(search.selectedService!==""&&!search.connection.services.some(row=>row.tool_name===search.selectedService))search.clearSelection()
    }}
    spacing:4
    RowLayout {
        Layout.fillWidth:true
        Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;font.bold:true;font.pixelSize:20;color:app.design.text;text:app.trx("NEXVARY Intelligence","Intelligence Search")}
        OsintDesktopCommand {text:search.app.trx("إعداد الاتصال","Connection setup");onClicked:search.setupRequested()}
    }
    Label {objectName:"intelligenceRetailPrice";visible:false;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.success;text:app.trx("سعر نيكسفاري الموحد: ","NEXVARY fixed price: ")+search.retailPrice+app.trx(" لكل عملية بحث"," per search") }
    Label {objectName:"intelligenceTotalDue";visible:!!connection&&connection.totalDueMinorUnits>0;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.success;text:app.trx("إجمالي المستحق غير المدفوع: ","Total unpaid amount due: ")+"€"+((connection&&connection.totalDueMinorUnits?connection.totalDueMinorUnits:0)/100).toFixed(2)}
    RowLayout {
        visible:false;Layout.fillWidth:true
        OsintDesktopCommand {objectName:"intelligenceLoadCategories";text:search.app.trx("تحديث فئات البحث — دون رسوم","Refresh search categories — free");enabled:!!search.connection&&search.connection.connected&&!search.connection.busy;onClicked:search.connection.refreshCatalog()}
        BusyIndicator {running:!!search.connection&&search.connection.busy;visible:running;Layout.preferredWidth:30;Layout.preferredHeight:30}
    }
    Label {visible:!connection||!connection.connected;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("احفظ المفتاح واختبر الاتصال ثم حدّث الفئات. تظهر البطاقات من دليل الخدمة الحقيقي؛ لا توجد نتائج افتراضية.","Save the key, test the connection, then refresh categories. Cards come from the live service catalog; no default findings are generated.")}
    EntryField {objectName:"intelligenceCategoryFilter";implicitHeight:32;visible:search.categories.length>0;Layout.fillWidth:true;requiredInput:false;placeholderTextColor:app.design.muted;placeholderText:app.trx("تصفية بطاقات البحث","Filter search cards");onTextEdited:search.filter=text;color:app.design.text;background:Rectangle {color:search.app.design.surface;border.color:search.app.design.border;radius:5}}
    OsintDesktopCommand {objectName:"intelligenceResumeLookup";visible:!!search.connection&&search.connection.pendingResultCount>0;text:search.app.trx("متابعة الطلب السابق — دون بحث جديد","Resume existing request — no new search");onClicked:{search.selectedCard=search.connection.selectedCategory;search.showSearchResult=true;serviceDialog.open()}}
    GridLayout {
        objectName:"intelligenceCategoryGrid"
        id:categoryGrid
        readonly property int rowCount:Math.max(1,Math.ceil(search.shownCategories.length/columns))
        readonly property real viewportHeight:typeof app.intelligenceViewportHeight==="number"?app.intelligenceViewportHeight:700
        readonly property int cardHeight:Math.max(50,Math.min(78,Math.floor((viewportHeight-108-(rowCount-1)*4)/rowCount)))
        Layout.fillWidth:true;columns:width>=1400?10:width>=900?9:width>=760?8:width>=640?7:width>=520?6:width>=400?4:width>=300?3:2;columnSpacing:4;rowSpacing:4
        Repeater {
            model:search.shownCategories
            delegate:Button {
                id:card
                required property var modelData
                objectName:"intelligenceCategory-"+modelData.name
                Layout.fillWidth:true;Layout.preferredWidth:1;Layout.minimumWidth:0;Layout.preferredHeight:categoryGrid.cardHeight;padding:4
                enabled:!search.connection||!search.connection.busy
                text:Categories.title(modelData.name,search.app.ar)
                Accessible.name:text
                onClicked:{search.specializedSearch=false;search.clearSelection();search.selectedCard=modelData.name;if(!search.unifiedSearch&&search.connection&&search.connection.connected&&search.connection.categories.some(row=>row.name===modelData.name))search.connection.discoverServices(modelData.name);serviceDialog.open()}
                background:Rectangle {radius:12;gradient:Gradient {GradientStop {position:0;color:card.hovered?"#103F53":"#102C3B"} GradientStop {position:0.6;color:"#081019"} GradientStop {position:1;color:"#04080D"}} border.color:search.connection&&search.connection.selectedCategory===card.modelData.name?search.app.design.accent:search.app.design.border;border.width:card.activeFocus?2:1}
                contentItem:RowLayout {
                    spacing:4
                    SemanticIcon {name:Categories.icon(card.modelData.name);Layout.preferredWidth:24;Layout.preferredHeight:24}
                    ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:2
                     Label {Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter;textFormat:Text.PlainText;maximumLineCount:2;elide:Text.ElideRight;color:search.app.design.text;font.pixelSize:categoryGrid.cardHeight<58?12:14;font.bold:true;text:card.text}
                     Rectangle {objectName:"categoryRetailPrice";Layout.alignment:Qt.AlignHCenter;implicitWidth:66;implicitHeight:17;radius:8;border.color:"#39FF14";color:"#082F23";Label {objectName:"cardRetailPriceText";anchors.centerIn:parent;text:search.retailPrice;color:"#F0FFF4";font.pixelSize:12;font.bold:true}}
                    }
                }
            }
        }
    }
    OsintDesktopPane {
        objectName:"intelligenceInvoices"
        Layout.fillWidth:true
        visible:!!search.connection&&!!search.connection.invoices&&search.connection.invoices.length>0
        title:search.app.trx("الفواتير المستحقة","Invoices due");iconName:"document_text"
        Repeater {model:search.connection&&search.connection.invoices?search.connection.invoices:[]
            delegate:Label {
                required property var modelData
                objectName:"intelligenceInvoice-"+modelData.id
                Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:search.app.design.text
                text:Categories.title(modelData.category,search.app.ar)+" · "+search.app.trx("فاتورة ","Invoice ")+modelData.id+" · €"+(Number(modelData.amountDueMinorUnits)/100).toFixed(2)+" · "+(modelData.status==="due"?search.app.trx("مستحقة — غير مدفوعة","Due — unpaid"):modelData.status==="void"?search.app.trx("ملغاة — لا مبلغ مستحق","Voided — nothing due"):search.app.trx("قيد التحقق — لم تُعتمد المطالبة","Pending verification — not invoiced yet"))
            }
        }
    }
    Label {visible:!!connection&&connection.errorCode!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.danger;text:app.trx("تعذر إتمام الطلب: ","Request could not complete: ")+(connection?search.errorMessage(connection.errorCode):"")}
    Dialog {
        id:serviceDialog
        objectName:"intelligenceServiceDialog"
        parent:Overlay.overlay
        modal:true
        width:Math.min(740,parent?parent.width-32:740)
        height:Math.min(690,parent?parent.height-32:690)
        x:parent?(parent.width-width)/2:0
        y:parent?(parent.height-height)/2:0
        title:search.app.trx("بحث نيكسفاري — ","NEXVARY Search — ")+Categories.title(search.selectedCard,search.app.ar)
        header:ColumnLayout {
            Label {Layout.fillWidth:true;Layout.margins:12;wrapMode:Text.WordWrap;color:search.app.design.text;font.bold:true;text:serviceDialog.title}
            Label {objectName:"intelligenceCurrentInvoice";Layout.fillWidth:true;Layout.margins:12;wrapMode:Text.WordWrap;textFormat:Text.PlainText;visible:search.showSearchResult&&!!search.currentInvoice.id;color:search.currentInvoice.status==="due"?search.app.design.success:search.app.design.danger;text:search.currentInvoice.status==="due"?search.app.trx("فاتورة مستحقة: ","Invoice due: ")+"€"+(Number(search.currentInvoice.amountDueMinorUnits)/100).toFixed(2)+search.app.trx(" — غير مدفوعة"," — unpaid"):search.currentInvoice.status==="void"?search.app.trx("لم يُسجل مبلغ مستحق لهذه العملية","No amount due for this operation"):search.app.trx("العملية قيد التحقق — لا مطالبة معتمدة حتى الآن","Operation pending verification — no confirmed amount due")}
        }
        standardButtons:Dialog.NoButton
        footer:DialogButtonBox {Button {text:search.app.trx("رجوع","Back");onClicked:serviceDialog.close()}}
        onClosed:paidConsent.checked=false
        background:Rectangle {radius:12;color:search.app.design.surface;border.color:search.app.design.accent;border.width:2}
        contentItem:ScrollView {
            id:serviceScroll
            clip:true;contentWidth:availableWidth
            ColumnLayout {
                width:serviceScroll.availableWidth
    Loader {Layout.fillWidth:true;active:search.showSearchResult&&!!search.connection&&(search.connection.resultText!==""||search.connection.pendingResultCount>0);visible:active;sourceComponent:LampyreConnectionPanel {app:search.app;showSetup:false;showTools:false}}
    OsintDesktopPane {
        Layout.fillWidth:true;visible:search.selectedCard!==""
        title:search.app.trx("خدمات ","Services: ")+Categories.title(search.selectedCard,search.app.ar);iconName:"search_visual"
        Label {visible:search.unifiedSearch;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:search.app.design.accent;text:search.app.trx("بحث موحد عبر المصادر المتاحة — لا يشغّل خدمة متخصصة منفردة.","Unified search across available sources — distinct from an individual specialized service.")}
        NeonCheckBox {objectName:"intelligenceSpecializedMode";visible:["email","phone","username"].indexOf(search.selectedCard)>=0;checked:search.specializedSearch;text:search.app.trx("استخدام خدمة متخصصة بدل البحث الموحد","Use an individual specialized service instead");onToggled:{search.specializedSearch=checked;search.clearSelection();if(checked&&search.connection&&search.connection.connected)search.connection.discoverServices(search.selectedCard)}}
        OsintDesktopCommand {visible:!search.connection||(search.unifiedSearch?!search.connection.lookupConnected:!search.connection.connected);text:search.app.trx("إعداد الاتصال","Connection setup");onClicked:{serviceDialog.close();search.setupRequested()}}
        Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:search.app.design.muted;text:search.unifiedSearch?search.app.trx("أدخل المعرّف ثم أكد عملية واحدة بالسعر المعروض. البحث لا يبدأ بمجرد فتح البطاقة أو إدخال البيانات.","Enter the identifier and confirm one operation at the displayed price. Opening a card or entering data never starts a search."):search.app.trx("اختر الخدمة، أدخل بياناتها، ثم راجع التكلفة قبل التنفيذ. فتح البطاقة وجلب دليلها لا يشغّلان بحثًا مدفوعًا.","Choose a service, enter its inputs and review its price. Opening a card and loading its catalog do not start a paid search.")}
        Label {visible:!!search.service.description;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:search.app.design.muted;text:search.connection?search.connection.displayText(search.service.description||""):""}
        Label {visible:search.unifiedSearch||search.selectedService!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:search.app.design.muted;text:search.connection?search.connection.displayText(search.service.args||""):""}
        Repeater {
            model:search.fields
            delegate:ColumnLayout {
                required property var modelData
                Layout.fillWidth:true
                Label {Layout.fillWidth:true;color:search.app.design.text;text:Categories.title(modelData.name,search.app.ar)}
                EntryField {
                    id:argument
                    objectName:"intelligenceArgument-"+modelData.name
                    Layout.fillWidth:true;maximumLength:8192
                    text:search.values[modelData.name]||""
                    placeholderText:search.unifiedSearch&&modelData.name==="phone"?search.app.trx("رقم دولي، مثل +12025550123","International number, e.g. +12025550123"):""
                    requiredInput:search.fields.length===1||String(search.service.args||"").indexOf("(all required)")>=0
                    validationKind:modelData.name==="phone"?"phone":modelData.name==="email"?"email":modelData.type==="integer"?"integer":modelData.type==="number"?"number":"text"
                    echoMode:/password|api_key|token|secret/i.test(modelData.name)?TextInput.Password:TextInput.Normal
                    color:search.app.design.text
                    background:Rectangle {color:search.app.design.surface;border.color:search.app.design.border;radius:5}
                    onTextEdited:{const next=Object.assign({},search.values);if(text.trim())next[modelData.name]=text;else delete next[modelData.name];search.values=next;search.showSearchResult=false;paidConsent.checked=false;search.validationError=""}
                }
            }
        }
        ComboBox {
            id:services
            objectName:"intelligenceServiceSelector";visible:!search.unifiedSearch
            Layout.fillWidth:true;textRole:"title";model:search.connection?search.connection.services:[];currentIndex:-1
            enabled:!!search.connection&&!search.connection.busy
            displayText:search.selectedService?search.connection.displayText(search.service.title||search.selectedService):search.app.trx("اختر خدمة البحث","Choose a search service")
            onActivated:index=>search.selectedService=model[index].tool_name
            palette.text:search.app.design.text;palette.buttonText:search.app.design.text
        }
        Label {objectName:"intelligenceServiceCost";visible:search.unifiedSearch||search.selectedService!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:search.app.design.accent;text:search.app.trx("السعر النهائي لعملية البحث: ","Search price: ")+search.retailPrice+search.app.trx(" — لا يعني تأكيد البحث أن التطبيق حصّل المبلغ."," — search confirmation does not record payment collection.")}
        NeonCheckBox {id:paidConsent;objectName:"intelligenceServiceConsent";visible:search.unifiedSearch||search.selectedService!=="";Layout.fillWidth:true;text:search.app.trx("أؤكد عملية البحث بسعر نيكسفاري المبين وأوافق على إرسال البيانات وتنفيذ الخدمة","Confirm this search at the displayed NEXVARY price and authorize sending the inputs and executing the service");contentItem:Label {text:paidConsent.text;wrapMode:Text.WordWrap;color:search.app.design.text;leftPadding:paidConsent.mirrored?0:paidConsent.indicator.width+paidConsent.spacing;rightPadding:paidConsent.mirrored?paidConsent.indicator.width+paidConsent.spacing:0}}
        OsintDesktopCommand {
            objectName:"intelligenceExecuteService";primary:true
            text:search.app.trx("تنفيذ البحث","Execute search")
            enabled:!!search.connection&&(search.unifiedSearch?search.connection.lookupConnected:search.connection.connected)&&!search.connection.busy&&!(search.connection.pendingResultCount>0)&&(search.unifiedSearch||search.selectedService!==""&&search.fields.length>0&&typeof search.service.photons==="number")&&paidConsent.checked
            onClicked:{
                if(search.unifiedSearch){const value=search.values[search.selectedCard]||"";if(!value.trim()||!Rules.valid(value,search.selectedCard==="phone"?"phone":search.selectedCard==="email"?"email":"text")){search.validationError=search.app.trx("راجع صيغة بيانات البحث؛ رقم الهاتف يجب أن يكون دوليًا.","Check the identifier format; phone numbers must be international.");paidConsent.checked=false;return}const started=search.connection.runLookup(search.selectedCard,value,paidConsent.checked);if(started)search.showSearchResult=true;paidConsent.checked=false;return}
                const all=String(search.service.args||"").indexOf("(all required)")>=0;
                for(const field of search.fields){const value=search.values[field.name]||"";if((all||search.fields.length===1)&&!value.trim()||value&&!Rules.valid(value,field.name==="phone"?"phone":field.name==="email"?"email":field.type==="integer"?"integer":field.type==="number"?"number":"text")){search.validationError=search.app.trx("راجع الحقل: ","Check input: ")+Categories.title(field.name,search.app.ar);paidConsent.checked=false;return}}
                if(Object.keys(search.values).length===0){search.validationError=search.app.trx("أدخل بيانات البحث أولًا","Enter search inputs first");paidConsent.checked=false;return}
                const started=search.connection.runService(search.selectedService,JSON.stringify(search.values),search.service.photons,paidConsent.checked);if(started)search.showSearchResult=true;paidConsent.checked=false
            }
        }
        Label {visible:search.validationError!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:search.app.design.danger;text:search.validationError}
    }
    Label {visible:!!connection&&connection.errorCode!=="";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.danger;text:app.trx("تعذر إتمام الطلب: ","Request could not complete: ")+(connection?search.errorMessage(connection.errorCode):"")}
            }
        }
    }
}