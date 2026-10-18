import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 objectName:"recoveryWorkspace"
 property var app
 property int step:0
 property string sourcePath:""
 property int selectedEvidence:-1
 property var lossKeys:["deleted","filesystem-damage","factory-reset","backup"]
 property var encryptionKeys:["unknown","fbe","fde","none"]
 property var steps:app.ar?["1  المصدر","2  الجهاز","3  نوع الفقد","4  التقييم والمخاطر","5  الموافقة والقضية","6  التحليل","7  معاينة النتائج","8  التصدير"]:["1  Source","2  Device","3  Loss type","4  Assessment & risks","5  Consent & case","6  Analysis","7  Review results","8  Export"]
 function notify(ok){app.notice(ok,app.trx("اكتملت العملية المحلية","Local operation completed"),recoveryBridge.error)}
 TabBar {id:recoveryTabs;objectName:"recoveryTabs";anchors.top:parent.top;anchors.left:parent.left;anchors.right:parent.right;TabButton{text:app.trx("تقييم المصدر","Source assessment")}TabButton{text:app.trx("القضايا والتحليل المحلي","Cases & local analysis")}}
 RecoveryAssessment {app:page.app;anchors.top:recoveryTabs.bottom;anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;visible:recoveryTabs.currentIndex===0}
 ColumnLayout {
  visible:recoveryTabs.currentIndex===1
  anchors.top:recoveryTabs.bottom;anchors.left:parent.left;anchors.right:parent.right;anchors.bottom:parent.bottom;spacing:10
  Rectangle {
   Layout.fillWidth:true;Layout.preferredHeight:58;radius:6;color:app.tone("#282615");border.color:app.tone("#706032")
   Label {anchors.fill:parent;anchors.margins:10;wrapMode:Text.WordWrap;color:app.tone("#e1c87b");font.pixelSize:12;text:app.trx("قيّم أثر الكتابة قبل تثبيت أي مساعد. لا يوجد Root أو فتح Bootloader أو ضبط مصنع. استعادة ذاكرة داخلية فقدت مفاتيحها بعد الضبط غير مثبتة.","Assess write impact before installing an agent. No root, bootloader unlock or reset. Internal recovery after reset with lost keys is not demonstrated.")}
  }
  RowLayout {
   Layout.fillWidth:true;Layout.fillHeight:true;spacing:10
   Panel {
    title:app.trx("مسار الاستعادة","RECOVERY WORKFLOW");Layout.preferredWidth:180;Layout.fillHeight:true
    ScrollView {
     id:workflowScroll;Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
     ColumnLayout {width:workflowScroll.availableWidth;spacing:8;Repeater {model:page.steps;delegate:ActionButton{required property int index;required property string modelData;objectName:"recoveryStep"+index;text:modelData;primary:page.step===index;Layout.fillWidth:true;enabled:!recoveryBridge.busy;onClicked:{page.step=index;recoveryScroll.contentItem.contentY=0}}}}
    }
   }
   ScrollView {
    id:recoveryScroll;objectName:"recoveryWizardScroll";Layout.fillWidth:true;Layout.fillHeight:true;contentWidth:availableWidth;clip:true
    ColumnLayout {
     width:recoveryScroll.availableWidth;spacing:12
     Label {visible:recoveryBridge.active;text:app.trx("القضية: ","Case: ")+recoveryBridge.caseTitle;Layout.fillWidth:true;color:app.tone("#6bb8ec");textFormat:Text.PlainText;elide:Text.ElideRight}
     Label {text:page.steps[page.step];font.pixelSize:21;font.bold:true;color:app.tone("#dbeaff");Layout.fillWidth:true}
     ColumnLayout {
      visible:page.step===0;Layout.fillWidth:true;spacing:12
      Label {text:app.trx("اختر مصدرًا متاحًا ومصرحًا به. تحليل الصور لا يجمع أقسام الهاتف تلقائيًا.","Choose an accessible authorized source. Image analysis does not acquire phone partitions automatically.");color:app.tone("#9ebbd3");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      ComboBox {id:method;objectName:"recoveryMethod";Layout.fillWidth:true;model:app.ar?["صورة خام: PNG / JPEG متجاورة","ملف مقروء أو نسخة احتياطية محلية","تقرير NEXVARY Recovery Agent"]:["Raw image: contiguous PNG / JPEG","Readable file or local backup copy","NEXVARY Recovery Agent report"];enabled:!recoveryBridge.busy}
      ActionButton {text:app.trx("اختيار ملف المصدر","Choose source file");onClicked:sourcePicker.open()}
      Label {text:page.sourcePath||app.trx("لم يُختر مصدر","No source selected");textFormat:Text.PlainText;wrapMode:Text.WrapAnywhere;color:app.tone("#70b7f0");Layout.fillWidth:true}
      Label {text:app.trx("صور .dd / .img / .raw / .bin فقط؛ حتى 2 GiB. لا يدعم EWF أو الأقسام المشفرة أو الملفات المتفرقة. نسخ الملفات المقروءة حتى 8 MiB للملف؛ لا تفك أرشيفات النسخ الاحتياطية.","Raw .dd / .img / .raw / .bin images only, up to 2 GiB. No EWF, encrypted-partition decoding or fragmented reconstruction. Readable-file copies are limited to 8 MiB each; backup archives are not unpacked.");color:app.tone("#99aec0");wrapMode:Text.WordWrap;Layout.fillWidth:true}
     }
     ColumnLayout {
      visible:page.step===1;Layout.fillWidth:true;spacing:10
      Label {text:app.trx("هوية يدوّنها الفني؛ القيم المجهولة تبقى مجهولة. تقرير Agent غير موقّع ويُراجع كمصدر مستقل.","Technician-supplied identity; unknowns remain unknown. An unsigned Agent report is reviewed as a separate source.");color:app.tone("#9ebbd3");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      TextField {id:brand;objectName:"recoveryBrand";Layout.fillWidth:true;maximumLength:120;placeholderText:app.trx("الشركة *","Manufacturer *");enabled:!recoveryBridge.active}
      TextField {id:model;objectName:"recoveryModel";Layout.fillWidth:true;maximumLength:120;placeholderText:app.trx("الموديل الدقيق أو غير معروف *","Exact model or unknown *");enabled:!recoveryBridge.active}
      TextField {id:androidVersion;Layout.fillWidth:true;maximumLength:80;placeholderText:app.trx("إصدار Android إن عُرف","Android version if known");enabled:!recoveryBridge.active}
      ActionButton {text:app.trx("استخدام الخصائص المقروءة حاليًا","Use currently read properties");enabled:!recoveryBridge.active&&!!toolBridge.deviceInfo["ro.product.model"];onClicked:{brand.text=toolBridge.deviceInfo["ro.product.manufacturer"]||"Unknown";model.text=toolBridge.deviceInfo["ro.product.model"];androidVersion.text=toolBridge.deviceInfo["ro.build.version.release"]||""}}
      Label {visible:recoveryBridge.active;text:JSON.stringify(recoveryBridge.device,null,2);textFormat:Text.PlainText;wrapMode:Text.WrapAnywhere;color:app.tone("#8fbddd");Layout.fillWidth:true}
     }
     ColumnLayout {
      visible:page.step===2;Layout.fillWidth:true;spacing:12
      Label {text:app.trx("نوع فقد البيانات","Type of data loss");color:app.tone("#b5cee4")}
      ComboBox {id:loss;objectName:"recoveryLoss";Layout.fillWidth:true;model:app.ar?["حذف ملفات","تلف نظام ملفات","إعادة ضبط المصنع","استرجاع من نسخة احتياطية"]:["Deleted files","Filesystem damage","Factory reset","Restore from a backup"];enabled:!recoveryBridge.active}
      Label {text:app.trx("التشفير: استخدم غير معروف ما لم توجد أدلة","Encryption: choose unknown unless supported by evidence");color:app.tone("#b5cee4");Layout.fillWidth:true;wrapMode:Text.WordWrap}
      ComboBox {id:encryption;Layout.fillWidth:true;model:[app.trx("غير معروف","Unknown"),"FBE","FDE",app.trx("غير مشفر وفق دليل مستقل","Unencrypted, independently verified")];enabled:!recoveryBridge.active}
      CheckBox {id:external;text:app.trx("توجد بطاقة أو ذاكرة خارجية","External storage may be available")}
      CheckBox {id:backup;text:app.trx("توجد نسخة احتياطية أو جهاز مرتبط","A backup or linked device may exist")}
     }
     TextArea {
      visible:page.step===3;objectName:"recoveryAssessment";Layout.fillWidth:true;Layout.preferredHeight:420;readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;text:recoveryBridge.assess(recoveryBridge.active?recoveryBridge.lossKind:page.lossKeys[loss.currentIndex],recoveryBridge.active?recoveryBridge.encryptionKind:page.encryptionKeys[encryption.currentIndex],external.checked,backup.checked,app.ar);color:app.tone("#c6daeb");background:Rectangle{color:app.tone("#0b2234");radius:6}
     }
     ColumnLayout {
      visible:page.step===4;Layout.fillWidth:true;spacing:12
      Label {text:recoveryBridge.protection;color:recoveryBridge.vaultReady?app.tone("#58dcb7"):app.tone("#ecb774");Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText}
      Label {text:app.trx("القضايا مشفرة بحساب Windows وجهازه. احمِ الحساب واحتفظ بنسخة مُدارة؛ نسخ الحاوية وحدها لا يضمن فكها على جهاز آخر. بيانات الورشة القديمة منفصلة وغير مشفرة.","Cases are protected by the Windows account/computer. Protect that account and plan managed recovery; copying the container alone does not guarantee decryption elsewhere. Existing workshop data remains separate and unencrypted.");color:app.tone("#9ebbd3");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      TextField {id:title;objectName:"recoveryTitle";Layout.fillWidth:true;maximumLength:160;placeholderText:app.trx("عنوان القضية *","Case title *");enabled:!recoveryBridge.active}
      TextField {id:owner;objectName:"recoveryOwner";Layout.fillWidth:true;maximumLength:160;placeholderText:app.trx("مرجع موافقة المالك / أمر الصيانة *","Owner consent / repair order reference *");enabled:!recoveryBridge.active}
      CheckBox {id:consent;objectName:"caseOwnerConsent";Layout.fillWidth:true;enabled:!recoveryBridge.active;contentItem:Label{text:app.trx("سُجلت موافقة صريحة من المالك لجمع المصادر المختارة وتحليلها، وجرى تقييم أثر الكتابة قبل تثبيت أي مساعد.","Explicit owner consent is recorded for selected-source collection/analysis, and installation write impact has been assessed.");color:app.tone("#d0e2f3");wrapMode:Text.WordWrap;leftPadding:consent.indicator.width+8;verticalAlignment:Text.AlignVCenter}}
      RowLayout {
       Layout.fillWidth:true
       ActionButton {objectName:"createRecoveryCase";text:app.trx("إنشاء قضية محلية","Create local case");primary:true;enabled:recoveryBridge.vaultReady&&!recoveryBridge.active&&!recoveryBridge.busy&&consent.checked&&title.text.trim()!==""&&owner.text.trim()!==""&&brand.text.trim()!==""&&model.text.trim()!=="";onClicked:{const ok=recoveryBridge.createCase(title.text,owner.text,brand.text,model.text,androidVersion.text,page.lossKeys[loss.currentIndex],page.encryptionKeys[encryption.currentIndex],consent.checked);page.notify(ok);if(ok)page.step=5}}
       ActionButton {text:app.trx("فتح قضية مشفرة","Open encrypted case");enabled:!recoveryBridge.busy;onClicked:casePicker.open()}
       ActionButton {objectName:"closeRecoveryCase";text:app.trx("إغلاق القضية","Close case");enabled:recoveryBridge.active&&!recoveryBridge.busy;onClicked:{page.notify(recoveryBridge.closeCase());consent.checked=false;sourceConsent.checked=false;page.selectedEvidence=-1}}
      }
      Label {visible:recoveryBridge.active;text:recoveryBridge.caseTitle+"\n"+recoveryBridge.casePath;wrapMode:Text.WrapAnywhere;color:app.tone("#6bb8ed");textFormat:Text.PlainText;Layout.fillWidth:true}
     }
     ColumnLayout {
      visible:page.step===5;Layout.fillWidth:true;spacing:14
      Label {text:app.trx("يُقرأ ملف المصدر فقط. تُحفظ النتائج داخل القضية بعد اكتمال التحقق؛ الإلغاء لا يعتمد نتائج جزئية.","Only the source file is read. Results are committed to the case after verification; cancellation does not commit partial results.");color:app.tone("#a2bdd4");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      Label {text:page.sourcePath||app.trx("اختر مصدرًا في الخطوة 1","Choose a source in step 1");color:app.tone("#71b8ee");wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;Layout.fillWidth:true}
      CheckBox {id:sourceConsent;objectName:"recoverySourceConsent";text:app.trx("هذا المصدر محدد ومصرح بقراءته","This selected source is authorized for reading")}
      RowLayout {
       ActionButton {objectName:"runRecovery";text:app.trx("بدء الجمع أو التحليل","Start collection or analysis");primary:true;enabled:recoveryBridge.active&&!recoveryBridge.busy&&sourceConsent.checked&&page.sourcePath!=="";onClicked:{const ok=method.currentIndex===0?recoveryBridge.scanImage(page.sourcePath,sourceConsent.checked):method.currentIndex===1?recoveryBridge.importVisibleFile(page.sourcePath,sourceConsent.checked):recoveryBridge.importAgent(page.sourcePath,sourceConsent.checked);if(!ok)page.notify(false)}}
       ActionButton {text:app.trx("إلغاء العملية","Cancel operation");enabled:recoveryBridge.busy;onClicked:recoveryBridge.cancel()}
      }
      ProgressBar {Layout.fillWidth:true;visible:recoveryBridge.busy&&method.currentIndex===0;value:recoveryBridge.progress}
      BusyIndicator {running:recoveryBridge.busy;visible:running;Layout.preferredWidth:40;Layout.preferredHeight:40}
      Label {text:app.trx("تقدم الصورة يقيس البايتات المفحوصة خلال التحقق والمسح وإعادة البصمة، وليس نسبة نجاح الاستعادة.","Image progress measures bytes checked across hashing, scanning and re-hashing; it is not a recovery success rate.");color:app.tone("#8ea9bf");wrapMode:Text.WordWrap;Layout.fillWidth:true}
     }
     ColumnLayout {
      visible:page.step===6;Layout.fillWidth:true;spacing:10
      Label {text:app.trx("الملفات المستخرجة مرشّحات متجاورة؛ لا يثبت وجودها أنها محذوفة أو تعود لما قبل الضبط. النسخ المنطقية وتقارير المساعد تظهر بنوع مستقل.","Carved files are contiguous candidates; their presence does not prove deletion or pre-reset origin. Logical copies and Agent reports have separate types.");color:app.tone("#a2bdd4");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      Label {text:app.trx("عدد الأدلة: ","Evidence items: ")+recoveryBridge.evidence.length;color:app.tone("#63d7b1");font.pixelSize:20}
      ListView {
       objectName:"recoveryEvidence";Layout.fillWidth:true;Layout.preferredHeight:250;clip:true;spacing:8;model:recoveryBridge.evidence
       delegate:Rectangle {
        required property var modelData;required property int index;width:ListView.view.width;height:98;radius:6;color:page.selectedEvidence===index?app.tone("#173b57"):app.tone("#0d2538");border.color:app.tone("#284b65")
        ColumnLayout {anchors.fill:parent;anchors.margins:10;Label{text:modelData.name+" · "+modelData.kind;color:app.tone("#d3e8fa");textFormat:Text.PlainText;elide:Text.ElideMiddle;Layout.fillWidth:true}Label{text:modelData.size+" bytes · offset "+modelData.offset;color:app.tone("#8cb2ce");font.pixelSize:12}Label{text:"SHA256 "+modelData.sha256;color:app.tone("#77b7e5");font.pixelSize:11;elide:Text.ElideMiddle;Layout.fillWidth:true;textFormat:Text.PlainText}}
        MouseArea {anchors.fill:parent;onClicked:page.selectedEvidence=index}
       }
      }
      Label {text:app.trx("سجل التدقيق المحلي","LOCAL AUDIT");color:app.tone("#9bbbd3")}
      Repeater {model:recoveryBridge.audit;delegate:Label{required property var modelData;text:modelData.atUtc+" · "+modelData.operation+" · "+modelData.outcome;color:app.tone("#789ab4");font.pixelSize:11;wrapMode:Text.WordWrap;textFormat:Text.PlainText;Layout.fillWidth:true}}
     }
     ColumnLayout {
      visible:page.step===7;Layout.fillWidth:true;spacing:12
      Label {text:app.trx("الملفات المصدّرة والتقرير غير مشفرة. اختر مكانًا محميًا؛ لا يرفع البرنامج البيانات إلى الإنترنت. لا يُستبدل أي ملف موجود.","Exported evidence and reports are unencrypted. Choose a protected location; no data is uploaded. Existing files are never replaced.");color:app.tone("#e0b677");wrapMode:Text.WordWrap;Layout.fillWidth:true}
      CheckBox {id:plaintext;objectName:"recoveryPlaintext";text:app.trx("أوافق على تصدير نسخة غير مشفرة","I authorize an unencrypted export")}
      RowLayout {
       ActionButton {text:app.trx("تصدير تقرير JSON","Export JSON report");enabled:recoveryBridge.active&&!recoveryBridge.busy&&plaintext.checked;onClicked:reportPicker.open()}
       ActionButton {text:app.trx("تصدير الدليل المحدد","Export selected evidence");enabled:recoveryBridge.active&&!recoveryBridge.busy&&plaintext.checked&&page.selectedEvidence>=0&&page.selectedEvidence<recoveryBridge.evidence.length;onClicked:evidencePicker.open()}
      }
      ActionButton {text:app.trx("حذف حاوية القضية","Delete case container");enabled:recoveryBridge.active&&!recoveryBridge.busy;onClicked:deleteConfirm.open()}
      Label {text:app.trx("الحذف يزيل الحاوية المختارة فقط؛ المصادر والنسخ المصدرة تبقى. لا يُدّعى مسح آمن لشريحة التخزين.","Deletion removes this container only; sources and exports remain. Secure storage-media erasure is not claimed.");color:app.tone("#92adc3");wrapMode:Text.WordWrap;Layout.fillWidth:true}
     }
     Rectangle {Layout.fillWidth:true;Layout.preferredHeight:70;color:recoveryBridge.error?app.tone("#392329"):app.tone("#102b3d");radius:6;Label{anchors.fill:parent;anchors.margins:10;text:recoveryBridge.status;textFormat:Text.PlainText;wrapMode:Text.WordWrap;color:app.tone("#b9d4e8");font.pixelSize:12}}
     RowLayout {
      Layout.fillWidth:true
      ActionButton {objectName:"recoveryBack";text:app.trx("السابق","Back");enabled:page.step>0&&!recoveryBridge.busy;onClicked:{page.step--;recoveryScroll.contentItem.contentY=0}}
      Item{Layout.fillWidth:true}
      ActionButton {objectName:"recoveryNext";text:app.trx("التالي","Next");primary:true;enabled:page.step<7&&!recoveryBridge.busy;onClicked:{page.step++;recoveryScroll.contentItem.contentY=0}}
     }
    }
   }
  }
 }
 FileDialog {id:sourcePicker;onAccepted:{page.sourcePath=selectedFile.toString();sourceConsent.checked=false}}
 FileDialog {id:casePicker;nameFilters:["NEXVARY encrypted case (*.nmrcase)"];onAccepted:if(!recoveryBridge.openCase(selectedFile.toString()))page.notify(false)}
 FileDialog {id:reportPicker;fileMode:FileDialog.SaveFile;defaultSuffix:"json";nameFilters:["JSON report (*.json)"];onAccepted:page.notify(recoveryBridge.exportReport(selectedFile.toString(),plaintext.checked))}
 FileDialog {id:evidencePicker;fileMode:FileDialog.SaveFile;onAccepted:page.notify(recoveryBridge.exportEvidence(page.selectedEvidence,selectedFile.toString(),plaintext.checked))}
 Dialog {id:deleteConfirm;anchors.centerIn:parent;width:440;modal:true;title:app.trx("حذف حاوية القضية؟","Delete case container?");standardButtons:Dialog.Yes|Dialog.No;Label{width:390;text:app.trx("سيتم حذف الحاوية المحلية المختارة. المصادر والملفات المصدرة لن تُحذف. هل تريد المتابعة؟","Delete the selected local case container? Sources and exported files remain. Continue?");wrapMode:Text.WordWrap}onAccepted:{page.notify(recoveryBridge.deleteCase(true));page.selectedEvidence=-1;consent.checked=false;sourceConsent.checked=false}}
}
