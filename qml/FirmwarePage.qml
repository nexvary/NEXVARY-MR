import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:14
  TabBar {id:firmwareTabs;objectName:"firmwareTabs";Layout.fillWidth:true
   TabButton {text:app.trx("مطابقة الحزمة","Package matching")}
   TabButton {text:app.trx("تحليل AVB المحلي","Local AVB inspection")}
   TabButton {text:app.trx("بنية ZIP والمخاطر","ZIP structure and risks")}
  }
  StackLayout {currentIndex:firmwareTabs.currentIndex;Layout.fillWidth:true;Layout.fillHeight:true
  ColumnLayout {spacing:14
  Panel {
   title:app.trx("فحص توافق ملف النظام · دون تفليش","Firmware compatibility preflight · no flashing");Layout.fillWidth:true;Layout.fillHeight:true
   Label {text:app.trx("فحص SHA-256 وهوية الموديل المعلنة فقط. لا يُثبت أصالة الملف أو توقيعه؛ يجب التحقق من مصدر الشركة الرسمي.","Checks SHA-256 and declared model identity only. It does not authenticate firmware or its signature; verify official OEM provenance.");color:app.tone("#f0bc75");wrapMode:Text.WordWrap;Layout.fillWidth:true;font.pixelSize:16}
   Label {text:app.trx("موديل الجهاز المقروء: ","Detected device model: ")+(toolBridge.deviceInfo["ro.product.model"]||"—");color:app.tone("#9fb8ce");textFormat:Text.PlainText}
   RowLayout {TextField{id:manifest;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("ملف بيانات التوافق JSON","Compatibility manifest JSON");Layout.fillWidth:true} ActionButton{text:app.trx("اختيار البيان","Choose manifest");onClicked:manifestDialog.open()}}
   RowLayout {TextField{id:firmware;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("ملف النظام المحلي","Local firmware package");Layout.fillWidth:true} ActionButton{text:app.trx("اختيار ملف النظام","Choose firmware");onClicked:firmwareDialog.open()}}
   RowLayout {
    ActionButton {text:app.trx("بدء التحقق المحلي","Verify local files");primary:true;enabled:!firmwareBridge.busy&&manifest.text!==""&&firmware.text!=="";onClicked:firmwareBridge.inspect(manifest.text,firmware.text,toolBridge.deviceInfo["ro.product.brand"]||"",toolBridge.deviceInfo["ro.product.model"]||"")}
    ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
    Label {text:firmwareBridge.progress+"%";color:app.tone("#93b8d7")}
   }
   ProgressBar {Layout.fillWidth:true;from:0;to:100;value:firmwareBridge.progress;visible:firmwareBridge.busy}
   TextArea {Layout.fillWidth:true;Layout.fillHeight:true;readOnly:true;wrapMode:TextEdit.Wrap;text:firmwareBridge.status;color:firmwareBridge.compatible?app.tone("#4dd9a8"):app.tone("#a9bfd2");background:Rectangle{color:app.tone("#0b2030");radius:6}}
   Label {text:app.trx("عمليات الاستعادة والتفليش غير مفعلة في هذا الإصدار. احتفظ بنسخة احتياطية واستخدم أدوات الشركة الرسمية وفقًا للموديل.","Recovery and flashing are not enabled in this build. Back up data and use official OEM tools for the exact device model.");color:app.tone("#89a3bb");wrapMode:Text.WordWrap;Layout.fillWidth:true}
  }
 }
  Panel {
   title:app.trx("Firmware Intelligence · بيانات AVB غير الموثوقة","Firmware Intelligence · untrusted AVB metadata")
   Label {objectName:"avbScope";text:app.trx("صورة vbmeta خام فقط، حتى 16 MiB. قراءة محلية دون هاتف. لا يشمل التحليل ZIP أو Sparse أو صور الأقسام ذات Footer.","Raw vbmeta images only, up to 16 MiB. Local inspection without a phone. ZIP, sparse images and partition footer discovery are unsupported.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#a9bfd2")}
   RowLayout {
    TextField {id:avbPath;objectName:"avbPath";readOnly:true;Layout.fillWidth:true;placeholderText:app.trx("اختر صورة vbmeta محلية","Choose a local vbmeta image");onTextChanged:firmwareBridge.clearAvbReport()}
    ActionButton {objectName:"chooseAvb";text:app.trx("اختيار الصورة","Choose image");enabled:!firmwareBridge.busy;onClicked:avbDialog.open()}
   }
   RowLayout {
    ActionButton {objectName:"inspectAvb";text:app.trx("تحليل البيانات","Inspect metadata");primary:true;enabled:!firmwareBridge.busy&&avbPath.text!=="";onClicked:firmwareBridge.inspectAvb(avbPath.text)}
    ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
    ActionButton {objectName:"exportAvb";text:app.trx("تصدير JSON","Export JSON");enabled:!firmwareBridge.busy&&firmwareBridge.avbJson!=="";onClicked:avbExport.open()}
   }
   Label {objectName:"avbTrustNotice";text:app.trx("الحقول التالية تصريحات داخل الملف، وليست إثباتًا للتوقيع أو أصالة الشركة أو أمان الرجوع أو توافق الجهاز. لا تمنح هذه النتيجة إذنًا بالتفليش.","These are file declarations. Signature, OEM authenticity, rollback safety and device compatibility remain unverified. This result does not authorize flashing.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#f0bc75")}
   Label {text:firmwareBridge.avbReport.status==="metadata-only"?app.trx("مؤشر الرجوع المعلن: ","Declared rollback index: ")+firmwareBridge.avbReport.declaredRollbackIndex+" · "+app.trx("أعلام الحماية المعلنة: ","Declared flags: ")+firmwareBridge.avbReport.declaredFlags:"";wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#c0c8d0")}
   Flickable {id:avbScroll;objectName:"avbScroll";Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumHeight:100;contentWidth:width;contentHeight:avbText.height;clip:true;boundsBehavior:Flickable.StopAtBounds;ScrollBar.vertical:ScrollBar {}
   TextArea {id:avbText;objectName:"avbResult";width:avbScroll.width;height:Math.max(avbScroll.height,contentHeight+topPadding+bottomPadding);readOnly:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.Wrap;text:firmwareBridge.avbJson||firmwareBridge.status;color:app.tone("#a9bfd2");background:Rectangle{color:app.tone("#0b2030");radius:6}}
   }
   Label {id:avbExportStatus;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a9bfd2")}
  }
  Panel {
   title:app.trx("فحص بنية حزمة محلية · دون استخراج","Local package structure · no extraction")
   Label {objectName:"packageScope";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#f0bc75");text:app.trx("ZIP محلي حتى 128 MiB و4096 مدخلًا. قراءة الأسماء والأحجام وبصمة النسخة المقروءة فقط؛ لا يُفك الضغط أو تُفحص محتويات الأقسام. الحزم الكبيرة وZIP64 والمشفرة غير مدعومة.","Local ZIP up to 128 MiB and 4096 entries. Reads names, sizes and snapshot SHA-256; no decompression or partition payload analysis. Large, ZIP64 and encrypted packages are unsupported.")}
   RowLayout {Layout.fillWidth:true
    TextField {id:packagePath;objectName:"firmwarePackagePath";Layout.fillWidth:true;readOnly:true;placeholderText:app.trx("اختر نسخة ZIP محلية للعمل عليها","Choose a local ZIP working copy");onTextChanged:{packagePrivacy.checked=false;packageExportStatus.text="";firmwareBridge.clearPackageReport()}}
    ActionButton {objectName:"chooseFirmwarePackage";text:app.trx("اختيار الحزمة","Choose package");enabled:!firmwareBridge.busy;onClicked:packageDialog.open()}
   }
   RowLayout {Layout.fillWidth:true
    ActionButton {objectName:"inspectFirmwarePackage";text:app.trx("فحص البنية","Inspect structure");primary:true;enabled:!firmwareBridge.busy&&packagePath.text!=="";onClicked:{packagePrivacy.checked=false;packageExportStatus.text="";firmwareBridge.inspectPackage(packagePath.text)}}
    ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
    ActionButton {objectName:"exportFirmwarePackage";text:app.trx("تصدير JSON","Export JSON");enabled:packagePrivacy.checked&&!firmwareBridge.busy&&firmwareBridge.packageJson!=="";onClicked:packageExport.open()}
   }
   Label {objectName:"packageTrustNotice";Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#f0bc75");text:app.trx("أسماء الصور ليست خريطة أقسام. توقيع OEM وسلامة محتوى الملفات وتوافق الهاتف غير متحقق منها. لا يمنح التقرير إذنًا للتفليش.","Image names are not a partition map. OEM signatures, payload integrity and phone compatibility remain unverified. This report never authorizes flashing.")}
   Flickable {id:packageScroll;objectName:"firmwarePackageScroll";Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumHeight:100;contentWidth:width;contentHeight:packageText.height;clip:true;ScrollBar.vertical:ScrollBar {}
    TextArea {id:packageText;objectName:"firmwarePackageResult";width:packageScroll.width;height:Math.max(packageScroll.height,contentHeight+topPadding+bottomPadding);readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:firmwareBridge.packageJson||firmwareBridge.status;color:app.tone("#a9bfd2");background:Rectangle {color:app.tone("#0b2030");radius:6}}
   }
   NeonCheckBox {id:packagePrivacy;objectName:"firmwarePackagePrivacy";Layout.fillWidth:true;text:app.trx("أوافق على تصدير أسماء الملفات والبصمة في تقرير محلي","I approve exporting filenames and hash in a local report")}
   Label {id:packageExportStatus;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a9bfd2")}
  }
  }
 }
 FileDialog {id:packageDialog;nameFilters:["ZIP (*.zip)"];onAccepted:packagePath.text=selectedFile.toString()}
 FileDialog {id:packageExport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:packageExportStatus.text=firmwareBridge.exportPackageReport(selectedFile.toString(),packagePrivacy.checked)?app.trx("حُفظ التقرير في ملف جديد","Report saved to a new file"):app.trx("تعذر الحفظ؛ اختر اسمًا محليًا جديدًا وراجع الموافقة","Could not save; choose a new local filename and review approval")}
 Connections {target:toolBridge;function onDeviceInfoChanged(){firmwareBridge.invalidate()} function onSelectedSerialChanged(){firmwareBridge.invalidate()}}
 FileDialog {id:manifestDialog;nameFilters:["JSON (*.json)"];onAccepted:manifest.text=selectedFile.toString()}
 FileDialog {id:firmwareDialog;onAccepted:firmware.text=selectedFile.toString()}
 FileDialog {id:avbDialog;onAccepted:{avbExportStatus.text="";avbPath.text=selectedFile.toString()}}
 FileDialog {id:avbExport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:avbExportStatus.text=firmwareBridge.exportAvbReport(selectedFile.toString())?app.trx("حُفظ التقرير محليًا","Report saved locally"):app.trx("تعذر حفظ التقرير؛ لا يمكن استبدال الصورة الأصلية","Could not save report; the source image cannot be replaced")}
}
