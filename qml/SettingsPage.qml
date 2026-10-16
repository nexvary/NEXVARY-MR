import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:14
  Panel {
   title:app.trx("الأدوات المحلية","Local tools");Layout.fillWidth:true;Layout.fillHeight:true
   Repeater {model:toolBridge.tools;delegate:RowLayout{required property var modelData;Layout.fillWidth:true;Rectangle{width:8;height:8;radius:4;color:modelData.available?"#48dba5":"#f0b770"}Label{text:modelData.name;color:"#e1ebf9";font.bold:true;Layout.preferredWidth:100}Label{text:modelData.available?modelData.path:app.trx("غير مثبت · استخدم المصدر الرسمي","Not installed · use the official source");color:"#8da8bd";elide:Text.ElideMiddle;Layout.fillWidth:true;textFormat:Text.PlainText}}}
   RowLayout {ActionButton{text:app.trx("إعادة فحص الأدوات","Refresh tools");onClicked:toolBridge.refreshTools()} ActionButton{text:app.trx("مصدر Platform Tools","Platform Tools source");onClicked:Qt.openUrlExternally("https://developer.android.com/tools/releases/platform-tools")} ActionButton{text:app.trx("مصدر scrcpy","scrcpy source");onClicked:Qt.openUrlExternally("https://github.com/Genymobile/scrcpy")}}
   Label {text:app.trx("لا يتم تنزيل أو تثبيت التعريفات تلقائيًا. فحص Windows يعرض أجهزة PnP دون تعديلها.","Drivers are not downloaded or installed automatically. Windows inspection lists PnP devices without changing them.");color:"#8da8bd";wrapMode:Text.WordWrap;Layout.fillWidth:true}
   Item {Layout.fillHeight:true}
  }
  Panel {
   title:app.trx("قاعدة البيانات والنسخ الاحتياطي","Database and backup");Layout.fillWidth:true;Layout.fillHeight:true
   Label {text:workshopBridge.databasePath;color:"#8da8bd";textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere}
   RowLayout {
    ActionButton{text:app.trx("فحص سلامة البيانات","Check integrity");onClicked:app.notice(workshopBridge.checkIntegrity()==="ok",app.trx("قاعدة البيانات سليمة","Database integrity is valid"))}
    ActionButton{text:app.trx("إنشاء نسخة احتياطية","Create backup");onClicked:backupDialog.open()}
    ActionButton{text:app.trx("استعادة نسخة","Restore backup");onClicked:restoreDialog.open()}
   }
   Label {text:app.trx("النسخ غير مشفرة. احفظها في مكان محمي. قبل الاستعادة تُحفظ نسخة من الحالة الحالية تلقائيًا.","Backups are not encrypted. Store them securely. Restore preserves a copy of the current database automatically.");color:"#ecb774";wrapMode:Text.WordWrap;Layout.fillWidth:true}
   Item {Layout.fillHeight:true}
  }
  Label {text:"NEXVARY MR 0.3.0 · Qt 6 / C++20 · "+app.trx("نسخة تطوير غير موقعة","Unsigned development build");color:"#66869d";Layout.fillWidth:true}
 }
 FileDialog {id:backupDialog;fileMode:FileDialog.SaveFile;defaultSuffix:"db";nameFilters:["SQLite (*.db)"];onAccepted:app.notice(workshopBridge.backup(selectedFile.toString()),app.trx("تم إنشاء النسخة الاحتياطية","Backup created"))}
 FileDialog {id:restoreDialog;nameFilters:["SQLite (*.db)"];onAccepted:restoreConfirm.open()}
 Dialog {id:restoreConfirm;width:460;modal:true;anchors.centerIn:parent;title:app.trx("تأكيد استعادة قاعدة البيانات","Confirm database restore");standardButtons:Dialog.Yes|Dialog.No;Label{text:app.trx("سيتم استبدال البيانات الحالية بالنسخة المختارة. هل تريد المتابعة؟","The selected backup will replace current data. Continue?");wrapMode:Text.WordWrap;width:400}onAccepted:app.notice(workshopBridge.restore(restoreDialog.selectedFile.toString(),true),app.trx("تمت الاستعادة","Backup restored"))}
}
