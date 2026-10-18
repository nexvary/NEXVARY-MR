import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:12
  Panel {
   title:app.trx("إضافة قطعة إلى المخزون","Add inventory item");Layout.fillWidth:true;Layout.preferredHeight:176
   RowLayout {TextField{id:sku;placeholderText:"SKU";maximumLength:100;Layout.fillWidth:true} TextField{id:name;placeholderText:app.trx("اسم القطعة","Part name");maximumLength:240;Layout.fillWidth:true}}
   RowLayout {
    Label {text:app.trx("الكمية","Quantity");color:app.tone("#a5bbce")} SpinBox{id:quantity;from:0;to:1000000;editable:true}
    Label {text:app.trx("سعر الوحدة ×100","Unit price ×100");color:app.tone("#a5bbce")} SpinBox{id:price;from:0;to:100000000;stepSize:100;editable:true}
    Item {Layout.fillWidth:true}
    ActionButton {text:app.trx("إضافة للمخزون","Add to stock");primary:true;enabled:workshopBridge.ready;onClicked:{if(workshopBridge.addPart(sku.text,name.text,quantity.value,price.value)){sku.clear();name.clear();app.notice(true,app.trx("تمت إضافة القطعة","Stock item added"))}else app.notice(false,"")}}
   }
  }
  Panel {
   title:app.trx("توريد مخزون لقطعة موجودة","Restock an existing item");Layout.fillWidth:true;Layout.preferredHeight:130
   RowLayout {
    ComboBox{id:restockItem;Layout.fillWidth:true;model:workshopBridge.inventory;textRole:"name";valueRole:"id"}
    SpinBox{id:restockQuantity;from:1;to:1000000;value:1;editable:true}
    Label{text:app.trx("السعر الجديد ×100","New price ×100");color:app.tone("#8da8bd")}
    SpinBox{id:restockPrice;from:0;to:100000000;editable:true;stepSize:100}
    ActionButton{text:app.trx("تسجيل التوريد","Receive stock");enabled:restockItem.count>0;onClicked:app.notice(workshopBridge.restockPart(restockItem.currentValue,restockQuantity.value,restockPrice.value),app.trx("تم التوريد؛ أسعار الفواتير السابقة محفوظة","Stock received; earlier invoice prices are preserved"))}
   }
  }
  ListView {
   Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:workshopBridge.inventory
   delegate:Rectangle {
    required property var modelData
    width:ListView.view.width;height:66;color:app.tone("#0c2130");border.color:app.tone("#203d52");radius:6
    RowLayout {anchors.fill:parent;anchors.margins:14;Label{text:modelData.sku;color:app.tone("#64acff");Layout.preferredWidth:140;textFormat:Text.PlainText} Label{text:modelData.name;color:app.tone("#e0ecfa");Layout.fillWidth:true;textFormat:Text.PlainText} Label{text:app.trx("المتوفر: ","In stock: ")+modelData.quantity;color:modelData.quantity>0?app.tone("#46d7a5"):app.tone("#e8af68")} Label{text:(modelData.unit_cents/100).toFixed(2);color:app.tone("#d4e5f5");Layout.preferredWidth:100}}
   }
   Label {anchors.centerIn:parent;visible:workshopBridge.inventory.length===0;text:app.trx("المخزون فارغ","Inventory is empty");color:app.tone("#839fb6")}
  }
 }
}
