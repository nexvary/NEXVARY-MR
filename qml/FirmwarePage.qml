import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:14
  TabBar {id:firmwareTabs;objectName:"firmwareTabs";Layout.fillWidth:true
   TabButton {text:app.trx("مطابقة الحزمة","Package matching")}
   TabButton {text:app.trx("تحليل AVB المحلي","Local AVB inspection")}
   TabButton {text:app.trx("بنية ZIP والمخاطر","ZIP structure and risks")}
  }
  StackLayout {currentIndex:firmwareTabs.currentIndex;Layout.fillWidth:true;Layout.fillHeight:true
  ScrollView {
   id:matchingScroll;objectName:"firmwareMatchingScroll";contentWidth:availableWidth;clip:true
   GridLayout {
    width:matchingScroll.availableWidth;columns:matchingScroll.availableWidth>=1000?2:1;columnSpacing:16;rowSpacing:16
    Panel {
     iconName:"arrow_square_down";title:app.trx("مطابقة حزمة النظام","Firmware package matching");Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     subtitle:app.trx("هوية الجهاز وبصمة SHA-256","Device identity and SHA-256")
     stateText:firmwareBridge.busy?app.trx("جارٍ التحقق","Checking"):app.trx("فحص محلي دون تفليش","Local check · no flashing")
     Label {text:app.trx("حدد البيان وملف النظام المراد مقارنته. لا تُكتب بيانات على الهاتف.","Select a manifest and the firmware file to compare. Phone data is not written.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a8c1d7")}
     Label {text:app.trx("بيان التوافق JSON","Compatibility manifest JSON");Layout.fillWidth:true;color:app.tone("#d0e0ef");font.bold:true}
     RowLayout {Layout.fillWidth:true;TextField {id:manifest;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("لم يُحدد بيان بعد","No manifest selected");Layout.fillWidth:true;Layout.minimumWidth:0}ActionButton {text:app.trx("اختيار البيان","Choose manifest");enabled:!firmwareBridge.busy;onClicked:manifestDialog.open()}}
     Label {text:app.trx("ملف النظام المحلي","Local firmware file");Layout.fillWidth:true;color:app.tone("#d0e0ef");font.bold:true}
     RowLayout {Layout.fillWidth:true;TextField {id:firmware;onTextChanged:firmwareBridge.invalidate();readOnly:true;placeholderText:app.trx("لم يُحدد ملف بعد","No file selected");Layout.fillWidth:true;Layout.minimumWidth:0}ActionButton {text:app.trx("اختيار الملف","Choose file");enabled:!firmwareBridge.busy;onClicked:firmwareDialog.open()}}
     Flow {
      Layout.fillWidth:true;spacing:8
      ActionButton {objectName:"verifyFirmwareMatch";text:app.trx("بدء التحقق المحلي","Verify local files");primary:true;enabled:!firmwareBridge.busy&&manifest.text!==""&&firmware.text!=="";onClicked:firmwareBridge.inspect(manifest.text,firmware.text,toolBridge.deviceInfo["ro.product.brand"]||"",toolBridge.deviceInfo["ro.product.model"]||"")}
      ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
     }
     ProgressBar {Layout.fillWidth:true;from:0;to:100;value:firmwareBridge.progress;visible:firmwareBridge.busy}
     Label {visible:firmwareBridge.busy;text:firmwareBridge.progress+"%";Layout.fillWidth:true;color:app.tone("#8ac7f5")}
    }
    Panel {
     iconName:"phone";title:app.trx("سياق الجهاز المحدد","Selected-device context");Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
     stateText:toolBridge.selectedSerial!==""?app.trx("جهاز محدد","Device selected"):app.trx("لم يُحدد جهاز","No device selected")
     Label {text:(toolBridge.deviceInfo["ro.product.manufacturer"]||"—")+" / "+(toolBridge.deviceInfo["ro.product.model"]||"—");Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#e0ecfa");font.bold:true;font.pixelSize:20}
     Label {text:app.trx("Android: ","Android: ")+(toolBridge.deviceInfo["ro.build.version.release"]||"—")+" · API "+(toolBridge.deviceInfo["ro.build.version.sdk"]||"—");Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:app.tone("#a8c1d7")}
     Label {text:toolBridge.deviceInfo["ro.build.fingerprint"]||app.trx("اقرأ معلومات الهاتف من مدير الأجهزة لعرض هوية البناء.","Read phone information in Device Manager to display its build identity.");Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#a8c1d7");font.pixelSize:12}
     Label {text:app.trx("المطابقة تقارن الهوية المعلنة في البيان؛ لا تُثبت أن الحزمة صادرة من الشركة أو مناسبة للتفليش.","Matching compares identity declared in the manifest; it does not authenticate OEM origin or approve flashing.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#f0bc75")}
     ActionButton {text:app.trx("فتح مدير الأجهزة","Open Device Manager");onClicked:app.navigate(1)}
    }
    Panel {
     iconName:"document_lock";title:app.trx("نتيجة التحقق وحدودها","Verification result and scope");Layout.columnSpan:parent.columns;Layout.fillWidth:true
     stateText:firmwareBridge.compatible?app.trx("مطابقة البيانات والبصمة","Metadata and hash match"):app.trx("راجع النتيجة","Review result")
     TextArea {objectName:"firmwareMatchResult";Layout.fillWidth:true;Layout.preferredHeight:Math.max(80,Math.min(200,contentHeight+topPadding+bottomPadding));readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;textFormat:TextEdit.PlainText;text:firmwareBridge.status;color:firmwareBridge.compatible?app.tone("#4dd9a8"):app.tone("#a9bfd2");background:Rectangle {color:app.tone("#0b2030");radius:6}}
     Label {text:app.trx("SHA-256 ومطابقة الموديل وحدهما لا يتحققان من التوقيع أو سلسلة ثقة OEM. لا تُفعّل هذه النتيجة التفليش أو الاستعادة.","SHA-256 and model matching alone do not verify a signature or OEM trust chain. This result enables neither flashing nor restoration.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#ecb774");font.pixelSize:12}
    }
   }
  }
  Panel {
   iconName:"document_lock";title:app.trx("Firmware Intelligence · بيانات AVB غير الموثوقة","Firmware Intelligence · untrusted AVB metadata")
   Label {objectName:"avbScope";text:app.trx("صورة vbmeta خام فقط، حتى 16 MiB. قراءة محلية دون هاتف. لا يشمل التحليل ZIP أو Sparse أو صور الأقسام ذات Footer.","Raw vbmeta images only, up to 16 MiB. Local inspection without a phone. ZIP, sparse images and partition footer discovery are unsupported.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#a9bfd2")}
   RowLayout {
    TextField {id:avbPath;objectName:"avbPath";readOnly:true;Layout.fillWidth:true;placeholderText:app.trx("اختر صورة vbmeta محلية","Choose a local vbmeta image");onTextChanged:{avbPrivacy.checked=false;avbExportStatus.text="";firmwareBridge.clearAvbReport()}}
    ActionButton {objectName:"chooseAvb";text:app.trx("اختيار الصورة","Choose image");enabled:!firmwareBridge.busy;onClicked:avbDialog.open()}
   }
   RowLayout {
    ActionButton {objectName:"inspectAvb";text:app.trx("تحليل البيانات","Inspect metadata");primary:true;enabled:!firmwareBridge.busy&&avbPath.text!=="";onClicked:firmwareBridge.inspectAvb(avbPath.text)}
    ActionButton {text:app.trx("إلغاء","Cancel");enabled:firmwareBridge.busy;onClicked:firmwareBridge.cancel()}
    ActionButton {objectName:"exportAvb";text:app.trx("تصدير JSON","Export JSON");enabled:avbPrivacy.checked&&!firmwareBridge.busy&&firmwareBridge.avbJson!=="";onClicked:avbExport.open()}
   }
   Label {objectName:"avbTrustNotice";text:app.trx("الحقول التالية تصريحات داخل الملف، وليست إثباتًا للتوقيع أو أصالة الشركة أو أمان الرجوع أو توافق الجهاز. لا تمنح هذه النتيجة إذنًا بالتفليش.","These are file declarations. Signature, OEM authenticity, rollback safety and device compatibility remain unverified. This result does not authorize flashing.");wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#f0bc75")}
   Label {text:firmwareBridge.avbReport.status==="metadata-only"?app.trx("مؤشر الرجوع المعلن: ","Declared rollback index: ")+firmwareBridge.avbReport.declaredRollbackIndex+" · "+app.trx("أعلام الحماية المعلنة: ","Declared flags: ")+firmwareBridge.avbReport.declaredFlags:"";wrapMode:Text.WordWrap;Layout.fillWidth:true;color:app.tone("#c0c8d0")}
   Flickable {id:avbScroll;objectName:"avbScroll";Layout.fillWidth:true;Layout.fillHeight:true;Layout.minimumHeight:100;contentWidth:width;contentHeight:avbText.height;clip:true;boundsBehavior:Flickable.StopAtBounds;ScrollBar.vertical:ScrollBar {}
   TextArea {id:avbText;objectName:"avbResult";width:avbScroll.width;height:Math.max(avbScroll.height,contentHeight+topPadding+bottomPadding);readOnly:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.Wrap;text:firmwareBridge.avbJson||firmwareBridge.status;color:app.tone("#a9bfd2");background:Rectangle{color:app.tone("#0b2030");radius:6}}
   }
   NeonCheckBox {id:avbPrivacy;objectName:"avbExportPrivacy";Layout.fillWidth:true;text:app.trx("أوافق على تصدير بيانات الأقسام والبصمة في تقرير محلي","I approve exporting partition metadata and hash in a local report")}
   Label {id:avbExportStatus;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#a9bfd2")}
  }
  Panel {
   iconName:"database";title:app.trx("فحص بنية حزمة محلية · دون استخراج","Local package structure · no extraction")
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
 Connections {target:firmwareBridge
  function onBusyChanged(){if(firmwareBridge.busy){avbPrivacy.checked=false;packagePrivacy.checked=false}}
  function onAvbReportChanged(){if(firmwareBridge.avbJson==="")avbPrivacy.checked=false}
 }
 Connections {target:toolBridge;function onDeviceInfoChanged(){firmwareBridge.invalidate()} function onSelectedSerialChanged(){firmwareBridge.invalidate()}}
 FileDialog {id:manifestDialog;nameFilters:["JSON (*.json)"];onAccepted:manifest.text=selectedFile.toString()}
 FileDialog {id:firmwareDialog;onAccepted:firmware.text=selectedFile.toString()}
 FileDialog {id:avbDialog;onAccepted:{avbExportStatus.text="";avbPath.text=selectedFile.toString()}}
 FileDialog {id:avbExport;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON (*.json)"];onAccepted:avbExportStatus.text=firmwareBridge.exportAvbReport(selectedFile.toString(),avbPrivacy.checked)?app.trx("حُفظ التقرير في ملف جديد","Report saved to a new file"):app.trx("تعذر الحفظ؛ اختر اسمًا محليًا جديدًا وراجع الموافقة","Could not save; choose a new local filename and review approval")}
}
