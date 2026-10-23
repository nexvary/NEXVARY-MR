import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"workshopPage"
 property var intakeCard:({})
 property var intakeHistory:[]
 property string barcodeError:""
 function showDeviceCard(card){if(!card.order_id){barcodeError=workshopBridge.error;return;}barcodeError="";selectedId=card.order_id;selectedOrder=workshopBridge.invoice(selectedId);intakeCard=card;intakeHistory=workshopBridge.orderHistory(selectedId);cardPrivacy.checked=false;deviceCardDialog.open()}
 function lookupDeviceBarcode(){const card=workshopBridge.lookupBarcode(barcodeInput.text);if(!card.order_id){barcodeError=workshopBridge.error;return;}search.text="";showDeviceCard(card);list.positionViewAtIndex(workshopBridge.orders.findIndex(function(o){return o.id===card.order_id}),ListView.Contain)}
 property var app
 property int selectedId:-1
 property var selectedOrder: ({})
 property int existingCustomerId:-1
 property int existingDeviceId:-1
 property string intakeError:""
 function intakeForCustomer(id){resetIntake();existingCustomerId=id;const c=workshopBridge.customerProfile(id);customer.text=c.name||"";phone.text=c.phone||"";newOrder.open()}
 function intakeForDevice(id){const d=workshopBridge.deviceProfile(id);intakeForCustomer(d.customer_id);existingDeviceId=id;brand.text=d.brand;model.text=d.model;serial.text=d.serial}
 function resetIntake(){intakeError="";existingCustomerId=-1;existingDeviceId=-1;customer.clear();phone.clear();brand.clear();model.clear();serial.clear();symptom.clear();tech.clear();notes.clear()}

 ColumnLayout {
  anchors.fill:parent;spacing:12
  RowLayout {
   Layout.fillWidth:true
   ActionButton {objectName:"newOrder";text:app.trx("أمر صيانة جديد","New repair order");primary:true;enabled:workshopBridge.ready;onClicked:{page.resetIntake();newOrder.open()}}
   TextField {id:search;Layout.fillWidth:true;placeholderText:app.trx("ابحث بالعميل، الموديل، الحالة أو رقم الأمر","Search customer, model, status or order ID")}
   ActionButton {text:app.trx("تصدير CSV","Export CSV");onClicked:csvDialog.open()}
  }
  RowLayout {Layout.fillWidth:true
   TextField {id:barcodeInput;objectName:"barcodeLookup";Layout.fillWidth:true;maximumLength:64;placeholderText:app.trx("امسح باركود بطاقة الجهاز أو اكتبه","Scan or enter the device-card barcode");onAccepted:page.lookupDeviceBarcode()}
   ActionButton {objectName:"findBarcode";text:app.trx("بحث بالباركود","Find barcode");enabled:barcodeInput.text.trim()!=="";onClicked:page.lookupDeviceBarcode()}
   ActionButton {objectName:"issueDeviceCard";text:app.trx("بطاقة الجهاز والاستلام","Device / intake card");enabled:page.selectedId>0;onClicked:page.showDeviceCard(workshopBridge.issueDeviceCard(page.selectedId))}
  }
  Label {objectName:"barcodeLookupError";Layout.fillWidth:true;visible:page.barcodeError!=="";text:page.barcodeError;textFormat:Text.PlainText;wrapMode:Text.WordWrap;color:app.tone("#e0787b")}
  Label {text:app.trx("المبالغ بالعملة المحلية؛ تُخزن الأجزاء النقدية كأعداد صحيحة. لا تُحتسب ضريبة تلقائيًا.","Amounts use local currency, stored as integer minor units. Tax is not calculated.");color:app.tone("#879fb7");Layout.fillWidth:true;wrapMode:Text.WordWrap}
  ListView {
   id:list;Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:workshopBridge.orders
   delegate:Rectangle {
    required property var modelData
    property bool matches: JSON.stringify(modelData).toLowerCase().indexOf(search.text.toLowerCase())>=0
    width:list.width;height:matches?84:0;visible:matches;color:page.selectedId===modelData.id?app.tone("#123453"):app.tone("#0b1e2d");border.color:app.tone("#203d52");radius:7
    MouseArea {anchors.fill:parent;onClicked:{page.selectedId=modelData.id;page.selectedOrder=modelData}}
    RowLayout {
     anchors.fill:parent;anchors.margins:14;spacing:18
     Label {text:"#"+modelData.id;color:app.tone("#68afff");font.pixelSize:22;Layout.preferredWidth:64}
     ColumnLayout {Layout.fillWidth:true;Label{text:modelData.customer+"  /  "+modelData.brand+" "+modelData.model;color:app.tone("#e7f1ff");font.bold:true;textFormat:Text.PlainText} Label{text:modelData.symptom;color:app.tone("#8da6bc");textFormat:Text.PlainText;elide:Text.ElideRight;Layout.fillWidth:true}}
     Label {text:app.orderStatus(modelData.status);color:modelData.status==="Ready"?app.tone("#51d6a2"):app.tone("#78b7ff")}
     Label {text:modelData.total_display;color:app.tone("#e6eef8");font.pixelSize:18}
     ActionButton {text:app.trx("تحرير","Edit");onClicked:{page.selectedId=modelData.id;page.selectedOrder=modelData;editTech.text=modelData.technician;editNotes.text=modelData.notes;labor.value=modelData.labor_cents;state.currentIndex=app.states.indexOf(modelData.status);editDialog.open()}}
     ActionButton {text:"PDF";onClicked:{page.selectedId=modelData.id;pdfDialog.open()}}
    }
   }
   Label {anchors.centerIn:parent;visible:workshopBridge.orders.length===0;text:app.trx("لا توجد أوامر بعد. أنشئ أول أمر صيانة.","No repair orders yet. Create your first order.");color:app.tone("#819cb4")}
  }
  Panel {
   title:app.trx("تخصيص قطع الغيار للأمر المحدد","Allocate parts to selected order");Layout.fillWidth:true;Layout.preferredHeight:126
   RowLayout {
    Label {text:page.selectedId>0?"#"+page.selectedId:app.trx("اختر أمرًا أولًا","Select an order");color:app.tone("#93adc4")}
    NeonComboBox {id:part;Layout.fillWidth:true;model:workshopBridge.inventory;textRole:"name";valueRole:"id"}
    SpinBox {id:qty;from:1;to:10000;value:1;editable:true}
    ActionButton {text:app.trx("تخصيص القطعة وخصم المخزون","Allocate and deduct stock");enabled:page.selectedId>0&&part.count>0;onClicked:app.notice(workshopBridge.usePart(page.selectedId,part.currentValue,qty.value),app.trx("تم تخصيص القطعة","Part allocated"))}
   }
  }
 }
 Dialog {
  id:newOrder;background:Rectangle {color:app.tone("#0a0a0a");radius:10;border.color:"#39ff14";border.width:1} objectName:"newOrderDialog";title:app.trx("أمر صيانة جديد","New repair order");modal:true;anchors.centerIn:parent;width:Math.min(650,page.width-24)
  contentItem:ColumnLayout {
   Label {objectName:"intakeError";visible:page.intakeError.length>0;text:page.intakeError;textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#ff7777")}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#69b7ff");text:page.existingDeviceId>0?app.trx("أمر جديد للهاتف المسجل #","New order for registered phone #")+page.existingDeviceId:page.existingCustomerId>0?app.trx("هاتف جديد للعميل #","New phone for customer #")+page.existingCustomerId:app.trx("عميل وهاتف جديدان. لإعادة استخدام بروفيل افتح تبويب البروفيلات.","New customer and phone. Reuse an existing profile from the Profiles tab.")}
   TextField {id:customer;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:customer.activeFocus?2:1;opacity:customer.enabled?1:0.5} enabled:page.existingCustomerId<0;objectName:"customerField";placeholderText:app.trx("اسم العميل *","Customer name *");Layout.fillWidth:true;maximumLength:240}
   RowLayout {TextField{id:phone;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:phone.activeFocus?2:1;opacity:phone.enabled?1:0.5} enabled:page.existingCustomerId<0;placeholderText:app.trx("الهاتف","Phone");Layout.fillWidth:true;maximumLength:80} TextField{id:tech;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:tech.activeFocus?2:1;opacity:tech.enabled?1:0.5} placeholderText:app.trx("الفني المسؤول","Technician");Layout.fillWidth:true;maximumLength:160}}
   RowLayout {TextField{id:brand;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:brand.activeFocus?2:1;opacity:brand.enabled?1:0.5} enabled:page.existingDeviceId<0;objectName:"brandField";placeholderText:app.trx("الشركة *","Brand *");Layout.fillWidth:true;maximumLength:240} TextField{id:model;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:model.activeFocus?2:1;opacity:model.enabled?1:0.5} enabled:page.existingDeviceId<0;objectName:"modelField";placeholderText:app.trx("الموديل *","Model *");Layout.fillWidth:true;maximumLength:240}}
   TextField {id:serial;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:serial.activeFocus?2:1;opacity:serial.enabled?1:0.5} enabled:page.existingDeviceId<0;placeholderText:app.trx("الرقم التسلسلي المصرح بقراءته","Authorized serial number");Layout.fillWidth:true;maximumLength:160}
   TextField {id:symptom;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:symptom.activeFocus?2:1;opacity:symptom.enabled?1:0.5} objectName:"symptomField";placeholderText:app.trx("وصف العطل *","Symptom *");Layout.fillWidth:true;maximumLength:2000}
   TextArea {id:notes;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:notes.activeFocus?2:1;opacity:notes.enabled?1:0.5} placeholderText:app.trx("الملاحظات وحالة الجهاز عند الاستلام","Notes and intake condition");Layout.fillWidth:true;Layout.preferredHeight:100;wrapMode:TextEdit.Wrap}
   Label {text:app.trx("بيانات العملاء محفوظة محليًا. احتفظ بنسخة احتياطية آمنة.","Customer data is stored locally. Keep a secure backup.");color:app.tone("#93a9bb");wrapMode:Text.WordWrap;Layout.fillWidth:true}
  }
  footer:ToolBar {
   contentItem:RowLayout {
    ActionButton {objectName:"saveIntakeButton";text:app.trx("حفظ","Save");primary:true;onClicked:newOrder.saveIntake()}
    ActionButton {text:app.trx("إلغاء","Cancel");onClicked:newOrder.reject()}
   }
  }
  function saveIntake(){
   page.intakeError="";
   if(!customer.text.trim()||!brand.text.trim()||!model.text.trim()||!symptom.text.trim()){
    page.intakeError=app.trx("أكمل الحقول المطلوبة: اسم العميل، الشركة، الموديل ووصف العطل.","Complete the required fields: customer name, brand, model and symptom.");return;
   }
   if(notes.text.length>20000){page.intakeError=app.trx("الملاحظات تتجاوز الحد المسموح: 20000 حرف.","Notes exceed the 20000 character limit.");return;}
const id=page.existingDeviceId>0?workshopBridge.createOrderForDevice(page.existingDeviceId,symptom.text,tech.text,notes.text):page.existingCustomerId>0?workshopBridge.createOrderForCustomer(page.existingCustomerId,brand.text,model.text,serial.text,symptom.text,tech.text,notes.text):workshopBridge.createOrder(customer.text,phone.text,brand.text,model.text,serial.text,symptom.text,tech.text,notes.text);app.notice(id>0,app.trx("تم إنشاء أمر الصيانة #","Created repair order #")+id);if(id>0){page.selectedId=id;search.clear();newOrder.close()}else{page.intakeError=app.trx("تعذر حفظ الأمر: ","Could not save order: ")+workshopBridge.error}}

 }
 Dialog {
  id:editDialog;background:Rectangle {color:app.tone("#0a0a0a");radius:10;border.color:"#39ff14";border.width:1} modal:true;title:app.trx("تحديث أمر الصيانة","Update repair order");anchors.centerIn:parent;width:Math.min(560,page.width-24);standardButtons:Dialog.Save|Dialog.Cancel
  contentItem:ColumnLayout {
   NeonComboBox {id:state;model:app.ar?app.statesAr:app.states;Layout.fillWidth:true}
   TextField {id:editTech;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:editTech.activeFocus?2:1;opacity:editTech.enabled?1:0.5} placeholderText:app.trx("الفني","Technician");Layout.fillWidth:true}
   Label {text:app.trx("أجرة العمل × 100 (مثال: 1250 = 12.50)","Labor in minor units (1250 = 12.50)");color:app.tone("#b3c8dc")}
   SpinBox {id:labor;from:0;to:100000000;editable:true;stepSize:100;Layout.fillWidth:true}
   TextArea {id:editNotes;background:Rectangle {implicitWidth:180;implicitHeight:40;radius:6;color:app.tone("#0a0a0a");border.color:"#39ff14";border.width:editNotes.activeFocus?2:1;opacity:editNotes.enabled?1:0.5} Layout.fillWidth:true;Layout.preferredHeight:140;wrapMode:TextEdit.Wrap}
  }
  onAccepted:app.notice(workshopBridge.updateOrder(page.selectedId,app.states[state.currentIndex],editTech.text,labor.value,editNotes.text),app.trx("تم تحديث الأمر","Order updated"))
 }
 Dialog {id:deviceCardDialog;objectName:"deviceCardDialog";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;width:Math.min(680,app.width-48);title:app.trx("بطاقة الجهاز وأمر الصيانة","Device and repair-order card");standardButtons:Dialog.Close
  contentItem:ScrollView {id:cardScroll;objectName:"deviceCardScroll";implicitHeight:Math.min(520,app.height-200);contentWidth:availableWidth;clip:true
   ColumnLayout {width:cardScroll.availableWidth;spacing:12
    Label {objectName:"deviceCardOrder";Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#edf4ff");text:"#"+(page.intakeCard.order_id||"")+" · "+(page.intakeCard.customer||"")+" · "+app.orderStatus(page.intakeCard.status||"")}
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8bfd4");text:(page.intakeCard.brand||"")+" "+(page.intakeCard.model||"")+" · "+(page.intakeCard.serial||"")}
    Rectangle {Layout.fillWidth:true;Layout.preferredHeight:116;color:"white";Image {objectName:"deviceCardBarcode";anchors.fill:parent;anchors.margins:8;source:page.intakeCard.barcodeImage||"";fillMode:Image.PreserveAspectFit;smooth:false}}
    Label {objectName:"deviceCardCode";Layout.fillWidth:true;horizontalAlignment:Text.AlignHCenter;textFormat:Text.PlainText;text:page.intakeCard.barcode||"";font.family:"monospace";color:app.tone("#edf4ff")}
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("الباركود يحدد أمرًا داخل قاعدة الورشة المحلية، ولا يمنح دخولًا لبوابة التتبع. راجع بيانات الجهاز بعد المسح.","The barcode identifies an order in this local database; it grants no tracking access. Review device details after scanning.")}
    Label {Layout.fillWidth:true;text:app.trx("آخر 200 عملية لأمر الصيانة","Latest 200 repair-order events");font.bold:true;color:app.tone("#c7deef")}
    ListView {objectName:"deviceCardHistory";Layout.fillWidth:true;Layout.preferredHeight:125;clip:true;model:page.intakeHistory;spacing:5;ScrollBar.vertical:ScrollBar {}
     delegate:Label {required property var modelData;width:ListView.view.width;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8bfd4");text:modelData.created_at+" UTC · "+modelData.action}
    }
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("ملف PDF يتضمن اسم العميل وبيانات الجهاز والعَرَض. راجعه قبل تسليمه. الطباعة من قارئ PDF؛ اختبار الطابعة الفعلية معلق.","The PDF includes customer name, device details and symptom. Review before handing it over. Print through a PDF reader; physical printer acceptance is pending.")}
    NeonCheckBox {id:cardPrivacy;objectName:"deviceCardPrivacy";text:app.trx("أوافق على تصدير بيانات بطاقة الاستلام","I approve intake-card data export")}
    ActionButton {objectName:"exportDeviceCard";Layout.fillWidth:true;text:app.trx("حفظ بطاقة الاستلام PDF للطباعة","Save printable intake PDF");enabled:cardPrivacy.checked&&!!page.intakeCard.order_id;onClicked:intakePdf.open()}
   }
  }
 }
 FileDialog {id:intakePdf;fileMode:FileDialog.SaveFile;defaultSuffix:"pdf";nameFilters:["Intake receipt (*.pdf)"];onAccepted:app.notice(workshopBridge.exportIntakeCard(page.intakeCard.order_id,selectedFile.toString(),cardPrivacy.checked),app.trx("حُفظت بطاقة الاستلام؛ راجعها واطبعها من قارئ PDF","Intake card saved; review and print from your PDF reader"))}
 FileDialog {id:csvDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"csv";nameFilters:["CSV (*.csv)"];onAccepted:app.notice(workshopBridge.exportOrders(selectedFile.toString()),app.trx("تم التصدير","Exported"))}
 FileDialog {id:pdfDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"pdf";nameFilters:["PDF (*.pdf)"];onAccepted:app.notice(workshopBridge.exportInvoice(page.selectedId,selectedFile.toString()),app.trx("تم حفظ الفاتورة؛ يمكن طباعتها من قارئ PDF","Invoice saved; print using your PDF reader"))}
}
