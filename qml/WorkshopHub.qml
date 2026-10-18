import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:hub
 objectName:"workshopHub"
 property var app
 property var customerData:({})
 property var deviceData:({})
 property string issuedLink:""
 function refresh(){customerData=workshopBridge.customerProfile(customerData.id||-1);deviceData=workshopBridge.deviceProfile(deviceData.id||-1)}
 Connections {target:workshopBridge;function onChanged(){hub.refresh()}}
 ColumnLayout {
  anchors.fill:parent;spacing:10
  TabBar {id:tabs;objectName:"workshopTabs";Layout.fillWidth:true
   TabButton {text:app.trx("أوامر الصيانة","Repair orders")}
   TabButton {objectName:"profilesTab";text:app.trx("بروفيلات العملاء والهواتف","Customer & phone profiles")}
   TabButton {objectName:"trackingTab";text:app.trx("متابعة العميل","Customer tracking")}
  }
  StackLayout {Layout.fillWidth:true;Layout.fillHeight:true;currentIndex:tabs.currentIndex
   WorkshopPage {id:ordersPage;app:hub.app}
   ScrollView {id:profileScroll;objectName:"profilesScroll";contentWidth:availableWidth;clip:true
    ColumnLayout {width:profileScroll.availableWidth;spacing:12
     TextField {id:profileSearch;objectName:"profileSearch";Layout.fillWidth:true;placeholderText:app.trx("بحث بالعميل أو رقم الهاتف أو موديل الجهاز أو الرقم التسلسلي","Search customer, phone, device model or serial")}
     Panel {Layout.fillWidth:true;Layout.preferredHeight:200;title:app.trx("العملاء — اختر البروفيل","Customers — select a profile")
      ListView {objectName:"customerProfiles";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:workshopBridge.customers;spacing:6
       delegate:ActionButton {required property var modelData;property bool match:JSON.stringify(modelData).toLowerCase().indexOf(profileSearch.text.toLowerCase())>=0;width:ListView.view.width;height:match?48:0;visible:match;text:modelData.name+" · "+modelData.phone+" · "+modelData.device_count+app.trx(" هاتف"," phone(s)");onClicked:{hub.customerData=workshopBridge.customerProfile(modelData.id);hub.deviceData=({})}}
      }
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:175;title:app.trx("بروفيل العميل","Customer profile")
      Label {Layout.fillWidth:true;textFormat:Text.PlainText;wrapMode:Text.WrapAnywhere;color:app.tone("#e0ecfa");text:hub.customerData.id?("#"+hub.customerData.id+" · "+hub.customerData.name+" · "+hub.customerData.phone+"\n"+hub.customerData.order_count+app.trx(" أمر صيانة"," repair order(s)")):app.trx("اختر عميلًا من القائمة","Select a customer")}
      RowLayout {ActionButton {objectName:"editCustomerProfile";text:app.trx("تعديل الاسم ورقم التواصل","Edit name and contact");enabled:!!hub.customerData.id;onClicked:{customerName.text=hub.customerData.name;customerPhone.text=hub.customerData.phone;editCustomer.open()}}ActionButton {text:app.trx("تسجيل هاتف جديد لهذا العميل","New phone for this customer");enabled:!!hub.customerData.id;onClicked:{ordersPage.intakeForCustomer(hub.customerData.id);tabs.currentIndex=0}}}
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:220;title:app.trx("الهواتف المسجلة","Registered phones")
      ListView {objectName:"deviceProfiles";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:workshopBridge.devices;spacing:6
       delegate:ActionButton {required property var modelData;property bool match:(!hub.customerData.id||modelData.customer_id===hub.customerData.id)&&JSON.stringify(modelData).toLowerCase().indexOf(profileSearch.text.toLowerCase())>=0;width:ListView.view.width;height:match?48:0;visible:match;text:"#"+modelData.id+" · "+modelData.brand+" "+modelData.model+" · "+modelData.serial+" · "+modelData.owner;onClicked:hub.deviceData=workshopBridge.deviceProfile(modelData.id)}
      }
      ActionButton {visible:!!hub.customerData.id;text:app.trx("عرض هواتف جميع العملاء","Show all customers' phones");onClicked:hub.customerData=({})}
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:170;title:app.trx("بروفيل الهاتف","Phone profile")
      Label {Layout.fillWidth:true;textFormat:Text.PlainText;wrapMode:Text.WrapAnywhere;color:app.tone("#e0ecfa");text:hub.deviceData.id?("#"+hub.deviceData.id+" · "+hub.deviceData.brand+" "+hub.deviceData.model+"\n"+hub.deviceData.owner+" · "+hub.deviceData.phone+"\n"+hub.deviceData.serial):app.trx("اختر هاتفًا لعرض سجله","Select a phone to view its history")}
      ActionButton {objectName:"repeatRepair";text:app.trx("أمر جديد لنفس الهاتف","New order for the same phone");enabled:!!hub.deviceData.id;onClicked:{ordersPage.intakeForDevice(hub.deviceData.id);tabs.currentIndex=0}}
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:260;title:app.trx("سجل الصيانة والتكاليف","Repair history and costs")
      ListView {Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:hub.deviceData.orders||hub.customerData.orders||[]
       delegate:Label {required property var modelData;width:ListView.view.width;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#c6daeb");text:"#"+modelData.id+" · "+app.orderStatus(modelData.status)+" · "+modelData.total_display+"\n"+modelData.symptom+"\n"+modelData.created_at+" UTC"}
      }
     }
    }
   }
   ScrollView {id:trackingScroll;objectName:"trackingScroll";contentWidth:availableWidth;clip:true
    ColumnLayout {width:trackingScroll.availableWidth;spacing:12
     Panel {Layout.fillWidth:true;Layout.preferredHeight:310;title:app.trx("بوابة متابعة العميل — شبكة المركز","Customer status portal — workshop network")
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("هذه بوابة HTTP لشبكة موثوقة فقط. ليعمل الرابط على هاتف العميل، اختر عنوان شبكة المركز واجعله على الشبكة نفسها. التطبيق والكمبيوتر يجب أن يبقيا قيد التشغيل. الوصول من خارج المركز يحتاج استضافة HTTPS منفصلة؛ لم تُنشر بوابة إنترنت بعد.","Trusted-network HTTP portal only. Choose the workshop LAN address and connect the customer's phone to that network. Keep this application and computer running. Access outside the workshop needs separate HTTPS hosting; no internet portal has been deployed.")}
      RowLayout {Layout.fillWidth:true;ComboBox {id:bindAddress;Layout.fillWidth:true;model:trackingPortal.addresses;enabled:!trackingPortal.running}SpinBox {id:port;from:1024;to:65535;value:8085;editable:true;enabled:!trackingPortal.running}}
      CheckBox {id:networkConsent;objectName:"portalConsent";Layout.fillWidth:true;text:app.trx("أوافق على تشغيل البوابة على الشبكة الموثوقة المختارة","Approve serving on the selected trusted network")}
      RowLayout {ActionButton {objectName:"startPortal";text:app.trx("تشغيل البوابة","Start portal");enabled:!trackingPortal.running&&networkConsent.checked;onClicked:trackingPortal.start(bindAddress.currentText,port.value,networkConsent.checked)}ActionButton {objectName:"stopPortal";text:app.trx("إيقاف","Stop");enabled:trackingPortal.running;onClicked:{trackingPortal.stop();hub.issuedLink=""}}Label {text:trackingPortal.baseUrl;color:app.tone("#51d6a2");textFormat:Text.PlainText}}
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:420;title:app.trx("رابط خاص لكل أمر صيانة","Private link for each repair order")
      ComboBox {id:trackingOrder;objectName:"trackingOrder";Layout.fillWidth:true;model:workshopBridge.orders;textRole:"id";valueRole:"id";onCurrentValueChanged:hub.issuedLink=""}
      Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8c1d7");text:app.trx("الرابط يعرض الموديل والحالة ومراحلها ورسالتك للعميل فقط. لا يعرض الاسم أو الرقم أو التسلسل أو العطل أو التكلفة أو ملاحظات الفني. كل من يملك الرابط يستطيع فتحه حتى إلغائه أو انتهاء صلاحيته.","The link shows model, status/history and your public message only. Customer name, contact, serial, fault, costs and technician notes stay private. Anyone holding the link can use it until revoked or expired.")}
      RowLayout {Label {text:app.trx("الصلاحية بالأيام","Expiry in days");color:app.tone("#c6daeb")}SpinBox {id:expiry;from:1;to:90;value:30}CheckBox {id:shareConsent;objectName:"trackingShareConsent";text:app.trx("أوافق على مشاركة هذه الحالة","Approve sharing this status")}}
      RowLayout {ActionButton {objectName:"issueTracking";text:app.trx("إنشاء / تغيير الرابط","Issue / rotate link");primary:true;enabled:trackingPortal.running&&trackingOrder.currentValue>0&&shareConsent.checked;onClicked:hub.issuedLink=trackingPortal.link(trackingOrder.currentValue,expiry.value,shareConsent.checked)}ActionButton {objectName:"revokeTracking";text:app.trx("إلغاء رابط هذا الأمر","Revoke order link");enabled:trackingOrder.currentValue>0;onClicked:{app.notice(workshopBridge.revokeTracking(trackingOrder.currentValue),app.trx("أُلغي الرابط","Link revoked"));hub.issuedLink=""}}}
      TextField {objectName:"trackingLink";Layout.fillWidth:true;readOnly:true;selectByMouse:true;text:hub.issuedLink;placeholderText:app.trx("الرابط يظهر مرة واحدة عند إنشائه؛ تغييره يلغي القديم","Link appears when issued; rotating invalidates the previous link")}
      RowLayout {ActionButton {text:app.trx("نسخ الرابط","Copy link");enabled:hub.issuedLink!=="";onClicked:trackingPortal.copyLink(hub.issuedLink)}ActionButton {text:app.trx("معاينة صفحة العميل","Preview customer page");enabled:hub.issuedLink!=="";onClicked:Qt.openUrlExternally(hub.issuedLink)}}
     }
     Panel {Layout.fillWidth:true;Layout.preferredHeight:220;title:app.trx("رسالة يراها العميل","Customer-visible message")
      TextArea {id:publicMessage;objectName:"publicTrackingMessage";Layout.fillWidth:true;Layout.fillHeight:true;wrapMode:TextEdit.Wrap;placeholderText:app.trx("مثال: تم فحص الجهاز، وننتظر وصول القطعة. لا تكتب بيانات شخصية.","Example: Diagnosis complete; waiting for the replacement part. Avoid personal data.");onTextChanged:if(text.length>1000)text=text.slice(0,1000)}
      ActionButton {text:app.trx("نشر الرسالة لهذا الأمر","Publish message for this order");enabled:trackingOrder.currentValue>0;onClicked:app.notice(workshopBridge.setPublicMessage(trackingOrder.currentValue,publicMessage.text),app.trx("حُدّثت رسالة العميل","Customer message updated"))}
     }
     Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:trackingPortal.error||workshopBridge.error;color:app.tone("#eab775")}
    }
   }
  }
 }
 Dialog {id:editCustomer;modal:true;anchors.centerIn:parent;width:Math.min(540,hub.width-20);title:app.trx("تعديل بروفيل العميل","Edit customer profile");standardButtons:Dialog.Save|Dialog.Cancel
  contentItem:ColumnLayout {TextField {id:customerName;Layout.fillWidth:true;maximumLength:240}TextField {id:customerPhone;Layout.fillWidth:true;maximumLength:80}}
  onAccepted:app.notice(workshopBridge.updateCustomer(hub.customerData.id,customerName.text,customerPhone.text),app.trx("حُدث بروفيل العميل","Customer profile updated"))
 }
}
