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
    Label {text:app.trx("الكمية","Quantity");color:"#a5bbce"} SpinBox{id:quantity;from:0;to:1000000;editable:true}
    Label {text:app.trx("سعر الوحدة ×100","Unit price ×100");color:"#a5bbce"} SpinBox{id:price;from:0;to:100000000;stepSize:100;editable:true}
    Item {Layout.fillWidth:true}
    ActionButton {text:app.trx("إضافة للمخزون","Add to stock");primary:true;enabled:workshopBridge.ready;onClicked:{if(workshopBridge.addPart(sku.text,name.text,quantity.value,price.value)){sku.clear();name.clear();app.notice(true,app.trx("تمت إضافة القطعة","Stock item added"))}else app.notice(false,"")}}
   }
  }
  ListView {
   Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:8;model:workshopBridge.inventory
   delegate:Rectangle {
    required property var modelData
    width:ListView.view.width;height:66;color:"#0c2130";border.color:"#203d52";radius:6
    RowLayout {anchors.fill:parent;anchors.margins:14;Label{text:modelData.sku;color:"#64acff";Layout.preferredWidth:140;textFormat:Text.PlainText} Label{text:modelData.name;color:"#e0ecfa";Layout.fillWidth:true;textFormat:Text.PlainText} Label{text:app.trx("المتوفر: ","In stock: ")+modelData.quantity;color:modelData.quantity>0?"#46d7a5":"#e8af68"} Label{text:(modelData.unit_cents/100).toFixed(2);color:"#d4e5f5";Layout.preferredWidth:100}}
   }
   Label {anchors.centerIn:parent;visible:workshopBridge.inventory.length===0;text:app.trx("المخزون فارغ","Inventory is empty");color:"#839fb6"}
  }
 }
}
