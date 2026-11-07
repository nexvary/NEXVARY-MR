import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ColumnLayout {
 id:page
 objectName:"supportedDevicesPage"
 property var app
 property string catalogFile:""
 spacing:10
 RowLayout {Layout.fillWidth:true
  NavIcon {kind:6;Layout.preferredWidth:32;Layout.preferredHeight:32}
  Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("كتالوج الأجهزة ومصادر الدعم","Devices and support provenance");color:app.tone("#edf4ff");font.pixelSize:20;font.bold:true}
  ActionButton {objectName:"importDeviceCatalog";text:app.trx("تحديث كتالوج البيانات","Update data catalog");onClicked:{page.catalogFile="";digest.text="";reviewed.checked=false;catalogDialog.open()}}
 }
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#eab775");text:app.trx("هذه قاعدة مراجع وليست قائمة هواتف اجتازت الاختبار. لا يُفعّل اسم الشركة أو الموديل أي أمر. القدرات تُقرأ من الاتصال الحالي، وأدوات OEM التجارية غير مدمجة.","This is a reference database, not a list of tested phones. Brand/model names cannot enable commands. Capabilities come from the current connection; commercial OEM tools are not integrated.")}
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8bfd4");text:(serviceCatalog.deviceCatalog.revision||"")+" · "+(serviceCatalog.deviceCatalog.chipsets||[]).join(" / ")}
 RowLayout {Layout.fillWidth:true
  NeonComboBox {id:brand;objectName:"catalogBrand";Layout.preferredWidth:220;model:[app.trx("جميع الشركات","All brands")].concat(serviceCatalog.deviceCatalog.brands||[])}
  TextField {id:search;objectName:"deviceCatalogSearch";Layout.fillWidth:true;placeholderText:app.trx("بحث بالموديل أو المعالج","Search model or chipset")}
 }
 ListView {id:rows;objectName:"deviceCatalogList";Layout.fillWidth:true;Layout.fillHeight:true;spacing:8;clip:true
  model:(serviceCatalog.deviceCatalog.models||[]).filter(function(row){const query=search.text.toLowerCase().trim();return (brand.currentIndex===0||row.brand===brand.currentText)&&(!query||(row.model+" "+row.chipset).toLowerCase().includes(query))})
  ScrollBar.vertical:ScrollBar {}
  delegate:GlossSurface {required property var modelData;width:rows.width;implicitHeight:details.implicitHeight+24;color:app.tone("#0b2131");radius:8;border.color:app.tone("#234a63");reflection:0.06
   ColumnLayout {id:details;anchors.top:parent.top;anchors.left:parent.left;anchors.right:parent.right;anchors.margins:12;spacing:6
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:modelData.brand+" · "+modelData.model;color:app.tone("#edf4ff");font.bold:true}
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:(modelData.evidence==="requested"?app.trx("مرجع مطلوب — غير متحقق","Requested reference — unverified"):app.trx("ادعاء موثق من مورّد — غير مختبر لدينا","Documented vendor claim — not tested by us"))+" · Android: "+(modelData.android||"?")+" · SoC: "+(modelData.chipset||"?");color:app.tone("#eab775")}
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;text:modelData.requirements;color:app.tone("#a8bfd4");font.pixelSize:12}
    ActionButton {visible:!!modelData.sourceUrl;text:app.trx("فتح المصدر","Open source");onClicked:app.openExternal(modelData.sourceUrl)}
   }
  }
 }
 Dialog {id:catalogDialog;objectName:"deviceCatalogDialog";parent:Overlay.overlay;anchors.centerIn:parent;modal:true;width:Math.min(620,app.width-48);title:app.trx("استيراد بيانات مرجعية","Import reference data");standardButtons:Dialog.Ok|Dialog.Cancel
  onOpened:standardButton(Dialog.Ok).enabled=Qt.binding(function(){return reviewed.checked&&page.catalogFile!==""&&/^[a-fA-F0-9]{64}$/.test(digest.text.trim())})
  onAccepted:serviceCatalog.importDeviceCatalog(page.catalogFile,digest.text.trim(),reviewed.checked)
  ColumnLayout {width:catalogDialog.availableWidth;spacing:12
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:app.trx("يمكن تحديث المراجع دون إعادة بناء البرنامج. لا يقبل الكتالوج أكوادًا أو أوامر أو اعتماد اختبارات أجهزة. محركات العمليات تحتاج إصدارًا مختبرًا من البرنامج.","Reference data can update without rebuilding. Catalogs cannot contain code, commands or hardware-test certification. Operation engines require a tested application release.");color:app.tone("#edf4ff")}
   ActionButton {text:app.trx("اختيار JSON","Choose JSON");onClicked:inputFile.open()}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;text:page.catalogFile;color:app.tone("#a8bfd4")}
   TextField {id:digest;Layout.fillWidth:true;placeholderText:"Reviewed SHA-256"}
   NeonCheckBox {id:reviewed;text:app.trx("راجعت مصدر الكتالوج وترخيصه وبصمته","I reviewed the catalog source, license and hash")}
  }
 }
 FileDialog {id:inputFile;fileMode:FileDialog.OpenFile;nameFilters:["Reference catalog (*.json)"];onAccepted:page.catalogFile=selectedFile.toString()}
 Label {Layout.fillWidth:true;visible:serviceCatalog.error!=="";text:serviceCatalog.error;textFormat:Text.PlainText;wrapMode:Text.WordWrap;color:app.tone("#e0787b")}
}
