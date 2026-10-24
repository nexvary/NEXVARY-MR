import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id:hub
 property alias currentTab:tabs.currentIndex
 property var app
 ColumnLayout {anchors.fill:parent;spacing:10
  TabBar {id:tabs;visible:app.activeSection!==4;objectName:"diagnosticTabs";Layout.fillWidth:true
   TabButton {text:app.trx("التشخيص والنتائج","Diagnosis & findings")}
   TabButton {objectName:"faultTreeTab";text:app.trx("شجرة الأعطال","Fault tree")}
   TabButton {objectName:"securityTab";text:app.trx("فحص الفيروسات","Malware scan")}
   TabButton {objectName:"driverCatalogTab";text:app.trx("تعريفات الشركات","OEM drivers")}
  }
  StackLayout {Layout.fillWidth:true;Layout.fillHeight:true;currentIndex:tabs.currentIndex
   DiagnosisPage {app:hub.app}
   FaultTreePage {app:hub.app}
   SecurityPage {app:hub.app}
   DriverCatalogPage {app:hub.app}
  }
 }
}
