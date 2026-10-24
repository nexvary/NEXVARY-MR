import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:page
 property var app
 signal opened()
 ColumnLayout {anchors.fill:parent;spacing:12
  Panel {title:app.trx("فتح وبحث القضايا المحلية","Open / search local cases");iconName:"search_visual";Layout.fillWidth:true
   RowLayout {Layout.fillWidth:true
    TextField {id:query;objectName:"lawCaseSearch";Layout.fillWidth:true;maximumLength:256;placeholderText:app.trx("بحث برقم القضية أو الجهة أو المحقق","Search case number, agency or investigator");onAccepted:lawCaseBridge.searchCases(text)}
    ActionButton {objectName:"lawSearchCases";text:app.trx("بحث","Search");enabled:lawCaseBridge.ready&&lawCaseBridge.permissions.read;onClicked:lawCaseBridge.searchCases(query.text)}
   }
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("تُفك القراءات داخل الذاكرة للحساب الحالي؛ لا يوجد فهرس أسماء أو أرقام قضايا مكشوف في SQLite. الصلاحيات المؤسسية الموثقة ما زالت بوابة نشر مستقلة.","Search decrypts in memory for the current account; no plaintext name/case-number index is stored in SQLite. Authenticated enterprise role provisioning remains a separate deployment gate.")}
  }
  ListView {id:list;objectName:"lawCaseList";Layout.fillWidth:true;Layout.fillHeight:true;clip:true;spacing:10;model:lawCaseBridge.cases
   ScrollBar.vertical:ScrollBar {}
   delegate:ItemDelegate {required property var modelData;width:ListView.view.width;height:104
    background:Rectangle {color:page.app.design.surface;radius:10;border.color:page.app.design.border}
    contentItem:ColumnLayout {spacing:5
     Label {text:modelData.metadata.caseNumber;Layout.fillWidth:true;elide:Text.ElideRight;color:page.app.design.text;font.bold:true;textFormat:Text.PlainText}
     Label {text:modelData.metadata.investigatingAgency+" · "+modelData.metadata.investigatorName;Layout.fillWidth:true;elide:Text.ElideRight;color:page.app.design.muted;textFormat:Text.PlainText}
     Label {text:modelData.status+" · "+page.app.trx("أجهزة: ","Devices: ")+modelData.deviceCount+" · revision "+modelData.revision;Layout.fillWidth:true;color:page.app.design.muted;font.pixelSize:12;textFormat:Text.PlainText}
    }
    onClicked:if(lawCaseBridge.openCase(modelData.id))page.opened()
   }
   EmptyState {anchors.centerIn:parent;width:Math.min(parent.width-20,500);visible:list.count===0;app:page.app;iconName:"document_lock";title:page.app.trx("لا توجد قضايا مطابقة متاحة لهذا الدور","No matching cases available to this role");description:page.app.trx("أنشئ قضية إذا كانت صلاحيتك والتخزين المحمي متاحين، أو غيّر عبارة البحث.","Create a case when permissions and protected storage are available, or change the query.")}
  }
 }
}
