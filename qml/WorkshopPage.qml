import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 property int selectedId:-1
 property var selectedOrder: ({})
 property int existingCustomerId:-1
 property int existingDeviceId:-1
 function intakeForCustomer(id){resetIntake();existingCustomerId=id;const c=workshopBridge.customerProfile(id);customer.text=c.name||"";phone.text=c.phone||"";newOrder.open()}
 function intakeForDevice(id){const d=workshopBridge.deviceProfile(id);intakeForCustomer(d.customer_id);existingDeviceId=id;brand.text=d.brand;model.text=d.model;serial.text=d.serial}
 function resetIntake(){existingCustomerId=-1;existingDeviceId=-1;customer.clear();phone.clear();brand.clear();model.clear();serial.clear();symptom.clear();tech.clear();notes.clear()}

 ColumnLayout {
  anchors.fill:parent;spacing:12
  RowLayout {
   Layout.fillWidth:true
   ActionButton {objectName:"newOrder";text:app.trx("أمر صيانة جديد","New repair order");primary:true;enabled:workshopBridge.ready;onClicked:{page.resetIntake();newOrder.open()}}
   TextField {id:search;Layout.fillWidth:true;placeholderText:app.trx("ابحث بالعميل، الموديل، الحالة أو رقم الأمر","Search customer, model, status or order ID")}
   ActionButton {text:app.trx("تصدير CSV","Export CSV");onClicked:csvDialog.open()}
  }
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
    ComboBox {id:part;Layout.fillWidth:true;model:workshopBridge.inventory;textRole:"name";valueRole:"id"}
    SpinBox {id:qty;from:1;to:10000;value:1;editable:true}
    ActionButton {text:app.trx("تخصيص القطعة وخصم المخزون","Allocate and deduct stock");enabled:page.selectedId>0&&part.count>0;onClicked:app.notice(workshopBridge.usePart(page.selectedId,part.currentValue,qty.value),app.trx("تم تخصيص القطعة","Part allocated"))}
   }
  }
 }
 Dialog {
  id:newOrder;objectName:"newOrderDialog";title:app.trx("أمر صيانة جديد","New repair order");modal:true;anchors.centerIn:parent;width:Math.min(650,page.width-24);standardButtons:Dialog.Save|Dialog.Cancel
  contentItem:ColumnLayout {
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#69b7ff");text:page.existingDeviceId>0?app.trx("أمر جديد للهاتف المسجل #","New order for registered phone #")+page.existingDeviceId:page.existingCustomerId>0?app.trx("هاتف جديد للعميل #","New phone for customer #")+page.existingCustomerId:app.trx("عميل وهاتف جديدان. لإعادة استخدام بروفيل افتح تبويب البروفيلات.","New customer and phone. Reuse an existing profile from the Profiles tab.")}
   TextField {id:customer;enabled:page.existingCustomerId<0;objectName:"customerField";placeholderText:app.trx("اسم العميل *","Customer name *");Layout.fillWidth:true;maximumLength:240}
   RowLayout {TextField{id:phone;enabled:page.existingCustomerId<0;placeholderText:app.trx("الهاتف","Phone");Layout.fillWidth:true;maximumLength:80} TextField{id:tech;placeholderText:app.trx("الفني المسؤول","Technician");Layout.fillWidth:true;maximumLength:160}}
   RowLayout {TextField{id:brand;enabled:page.existingDeviceId<0;objectName:"brandField";placeholderText:app.trx("الشركة *","Brand *");Layout.fillWidth:true;maximumLength:240} TextField{id:model;enabled:page.existingDeviceId<0;objectName:"modelField";placeholderText:app.trx("الموديل *","Model *");Layout.fillWidth:true;maximumLength:240}}
   TextField {id:serial;enabled:page.existingDeviceId<0;placeholderText:app.trx("الرقم التسلسلي المصرح بقراءته","Authorized serial number");Layout.fillWidth:true;maximumLength:160}
   TextField {id:symptom;objectName:"symptomField";placeholderText:app.trx("وصف العطل *","Symptom *");Layout.fillWidth:true;maximumLength:2000}
   TextArea {id:notes;placeholderText:app.trx("الملاحظات وحالة الجهاز عند الاستلام","Notes and intake condition");Layout.fillWidth:true;Layout.preferredHeight:100;wrapMode:TextEdit.Wrap}
   Label {text:app.trx("بيانات العملاء محفوظة محليًا. احتفظ بنسخة احتياطية آمنة.","Customer data is stored locally. Keep a secure backup.");color:app.tone("#93a9bb");wrapMode:Text.WordWrap;Layout.fillWidth:true}
  }
  onAccepted:{const id=page.existingDeviceId>0?workshopBridge.createOrderForDevice(page.existingDeviceId,symptom.text,tech.text,notes.text):page.existingCustomerId>0?workshopBridge.createOrderForCustomer(page.existingCustomerId,brand.text,model.text,serial.text,symptom.text,tech.text,notes.text):workshopBridge.createOrder(customer.text,phone.text,brand.text,model.text,serial.text,symptom.text,tech.text,notes.text);app.notice(id>0,app.trx("تم إنشاء أمر الصيانة #","Created repair order #")+id);if(id<=0)newOrder.open()}
 }
 Dialog {
  id:editDialog;modal:true;title:app.trx("تحديث أمر الصيانة","Update repair order");anchors.centerIn:parent;width:Math.min(560,page.width-24);standardButtons:Dialog.Save|Dialog.Cancel
  contentItem:ColumnLayout {
   ComboBox {id:state;model:app.ar?app.statesAr:app.states;Layout.fillWidth:true}
   TextField {id:editTech;placeholderText:app.trx("الفني","Technician");Layout.fillWidth:true}
   Label {text:app.trx("أجرة العمل × 100 (مثال: 1250 = 12.50)","Labor in minor units (1250 = 12.50)");color:app.tone("#b3c8dc")}
   SpinBox {id:labor;from:0;to:100000000;editable:true;stepSize:100;Layout.fillWidth:true}
   TextArea {id:editNotes;Layout.fillWidth:true;Layout.preferredHeight:140;wrapMode:TextEdit.Wrap}
  }
  onAccepted:app.notice(workshopBridge.updateOrder(page.selectedId,app.states[state.currentIndex],editTech.text,labor.value,editNotes.text),app.trx("تم تحديث الأمر","Order updated"))
 }
 FileDialog {id:csvDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"csv";nameFilters:["CSV (*.csv)"];onAccepted:app.notice(workshopBridge.exportOrders(selectedFile.toString()),app.trx("تم التصدير","Exported"))}
 FileDialog {id:pdfDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"pdf";nameFilters:["PDF (*.pdf)"];onAccepted:app.notice(workshopBridge.exportInvoice(page.selectedId,selectedFile.toString()),app.trx("تم حفظ الفاتورة؛ يمكن طباعتها من قارئ PDF","Invoice saved; print using your PDF reader"))}
}
