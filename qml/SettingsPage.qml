import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 id:page
 property var app
 property string toolName:""
 ScrollView {
  id:settingsScroll;objectName:"settingsScroll";anchors.fill:parent;contentWidth:availableWidth;clip:true
  ColumnLayout {
   width:settingsScroll.availableWidth;spacing:16
   Label {text:app.trx("تهيئة محطة العمل","WORKSTATION CONFIGURATION");Layout.fillWidth:true;color:app.tone("#e0ecfa");font.pixelSize:20;font.bold:true}
   Label {text:app.trx("حدد أدواتك المحلية، راجع جاهزيتها، واحفظ نسخة من بيانات الورشة قبل الاستعادة.","Select local tools, review readiness, and protect workshop data before restoring a backup.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3")}
   GovernmentNetworkPanel {app:page.app;Layout.fillWidth:true}
   GridLayout {
    Layout.fillWidth:true;columns:settingsScroll.availableWidth>=1020?3:settingsScroll.availableWidth>=640?2:1;columnSpacing:16;rowSpacing:16
    Repeater {
     model:toolBridge.tools
     delegate:Panel {
      required property var modelData
      Layout.fillWidth:true;Layout.alignment:Qt.AlignTop
      iconName:modelData.name==="scrcpy"?"phone":"settings"
      title:modelData.name
      subtitle:app.trx("ملف تنفيذي محلي منفصل","Separately installed local executable")
      stateText:modelData.available?app.trx("متاح","Available"):app.trx("يلزم الإعداد","Setup required")
      Label {text:modelData.available?modelData.path:app.trx("اختر الملف التنفيذي من النسخة الرسمية المثبتة.","Choose the executable from your installed official distribution.");Layout.fillWidth:true;Layout.minimumWidth:0;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#a8c1d7");font.pixelSize:12}
      RowLayout {
       Layout.fillWidth:true
       ActionButton {text:app.trx("اختيار الأداة","Choose tool");primary:!modelData.available;enabled:!toolBridge.busy;Layout.fillWidth:true;onClicked:{page.toolName=modelData.name;toolPicker.open()}}
       ActionButton {text:app.trx("تلقائي","Auto");enabled:modelData.configured&&!toolBridge.busy;onClicked:{const ok=toolBridge.clearTool(modelData.name);app.notice(ok,toolBridge.status,toolBridge.status)}}
      }
     }
    }
   }
   Panel {
    iconName:"settings";title:app.trx("جاهزية الأدوات والمصادر","Tool readiness and official sources");Layout.fillWidth:true
    Flow {
     Layout.fillWidth:true;spacing:8
     ActionButton {text:app.trx("إعادة فحص الأدوات","Refresh tools");enabled:!toolBridge.busy;onClicked:toolBridge.refreshTools()}
     ActionButton {text:app.trx("مصدر Platform Tools","Platform Tools source");onClicked:app.openExternal("https://developer.android.com/tools/releases/platform-tools")}
     ActionButton {text:app.trx("مصدر scrcpy","scrcpy source");onClicked:app.openExternal("https://github.com/Genymobile/scrcpy")}
    }
    Label {text:app.trx("اختيار الملف يحفظ مساره. لا يتم تنزيل أو تثبيت التعريفات تلقائيًا؛ فحص Windows يعرض أجهزة PnP دون تعديلها.","Selecting an executable saves its path. Drivers are not downloaded or installed automatically; Windows inspection lists PnP devices without changing them.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#9bbbd3");font.pixelSize:12}
   }
   Panel {
    iconName:"database";title:app.trx("بيانات الورشة والنسخ الاحتياطي","Workshop data and backup");Layout.fillWidth:true
    stateText:workshopBridge.ready?app.trx("قاعدة البيانات جاهزة","Database ready"):app.trx("راجع خطأ قاعدة البيانات","Review database error")
    Label {text:workshopBridge.databasePath;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.tone("#a8c1d7");font.pixelSize:12}
    Flow {
     Layout.fillWidth:true;spacing:8
     ActionButton {text:app.trx("فحص سلامة البيانات","Check integrity");onClicked:app.notice(workshopBridge.checkIntegrity()==="ok",app.trx("قاعدة البيانات سليمة","Database integrity is valid"))}
     ActionButton {text:app.trx("إنشاء نسخة احتياطية","Create backup");primary:true;onClicked:backupDialog.open()}
     ActionButton {text:app.trx("استعادة نسخة","Restore backup");onClicked:restoreDialog.open()}
    }
    Label {text:app.trx("النسخ غير مشفرة. احفظها في مكان محمي. قبل الاستعادة تُحفظ نسخة من الحالة الحالية تلقائيًا.","Backups are not encrypted. Store them securely. Restore preserves a copy of the current database automatically.");Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.tone("#ecb774");font.pixelSize:12}
   }
   Label {text:"NEXVARY MR 0.4.0 · Qt 6 / C++20 · "+app.trx("نسخة تطوير غير موقعة","Unsigned development build");color:app.tone("#829bb5");Layout.fillWidth:true;wrapMode:Text.WordWrap;font.pixelSize:12}
  }
 }
 FileDialog {id:toolPicker;title:app.trx("اختيار الملف التنفيذي لأداة ","Choose executable for ")+page.toolName;nameFilters:["Executable (*.exe)","All files (*)"];onAccepted:{const ok=toolBridge.configureTool(page.toolName,selectedFile.toString());app.notice(ok,toolBridge.status,toolBridge.status)}}
 FileDialog {id:backupDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"db";nameFilters:["SQLite (*.db)"];onAccepted:app.notice(workshopBridge.backup(selectedFile.toString()),app.trx("تم إنشاء النسخة الاحتياطية","Backup created"))}
 FileDialog {id:restoreDialog;nameFilters:["SQLite (*.db)"];onAccepted:restoreConfirm.open()}
 Dialog {id:restoreConfirm;width:Math.min(500,page.width-32);modal:true;anchors.centerIn:parent;title:app.trx("تأكيد استعادة قاعدة البيانات","Confirm database restore");standardButtons:Dialog.Yes|Dialog.No;Label {width:restoreConfirm.availableWidth;text:app.trx("سيتم استبدال البيانات الحالية بالنسخة المختارة. هل تريد المتابعة؟","The selected backup will replace current data. Continue?");wrapMode:Text.WordWrap}onAccepted:app.notice(workshopBridge.restore(restoreDialog.selectedFile.toString(),true),app.trx("تمت الاستعادة","Backup restored"))}
}
