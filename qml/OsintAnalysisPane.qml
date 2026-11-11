import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
 id: analysis
 objectName: "osintAnalysisPane"
 property var app
 readonly property var bridge: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 readonly property var records: bridge && bridge.result.records ? bridge.result.records : []
 property int viewMode: 0
 property int selectedIndex: -1
 property bool showInspector:true
 property bool showRequests:true
 property bool descending: false
 property string filterText: ""
 readonly property var selected: selectedIndex >= 0 && selectedIndex < records.length ? records[selectedIndex] : ({})
 readonly property var filtered: {
  let rows=[];
  for(let i=0;i<records.length;i++) {
   const row=records[i];
   if((row.value+" "+row.type).toLowerCase().indexOf(filterText.toLowerCase())>=0)rows.push({index:i,row:row});
  }
  rows.sort(function(a,b){return descending?b.row.value.localeCompare(a.row.value):a.row.value.localeCompare(b.row.value)});
  return rows;
 }
 function pick(index){selectedIndex=index}
 LayoutMirroring.enabled: app.ar
 LayoutMirroring.childrenInherit: true
 padding:8
 background:Rectangle {color:"#040507"}
 contentItem:ColumnLayout {spacing:8
 Connections {target:analysis.bridge;function onChanged(){analysis.selectedIndex=-1}}
 RowLayout {
  Layout.fillWidth:true
  Label {text:analysis.app.trx("مستكشف النتائج", "Results explorer");font.pixelSize:15;font.bold:true;color:analysis.app.design.text;Layout.fillWidth:true}
  StateBadge {text:analysis.app.trx("اصطناعي • دون اتصال", "SYNTHETIC • OFFLINE");tint:analysis.app.design.accent}
 }
 Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("تجربة محلية لعرض البيانات؛ ليست نتائج من Lampyre ولا أدلة قضية. تحديد عنصر يفتح مرجعه وتفاصيله.", "Local data-viewing fixture; not Lampyre findings or case evidence. Select an entity to inspect its reference and details.")}
 SplitView {
  objectName:"osintAnalysisSplit"
  Layout.fillWidth:true;Layout.preferredHeight:width>1060?510:1150
  orientation:width>1060?Qt.Horizontal:Qt.Vertical
  handle:Rectangle {implicitWidth:5;implicitHeight:5;color:SplitHandle.hovered?"#23B9E7":"#153D52"}
  OsintDesktopPane {
   visible:analysis.showRequests;SplitView.preferredWidth:210;SplitView.minimumWidth:165;SplitView.preferredHeight:240;Layout.fillWidth:analysis.width<=1060;Layout.fillHeight:true;Layout.minimumWidth:0
   title:analysis.app.trx("الطلبات", "Requests");iconName:"history"
   StateBadge {text:analysis.app.trx("جلسة محلية", "LOCAL SESSION");tint:analysis.app.design.silver}
   Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.text;font.bold:true;text:"Lampyre / UI fixture"}
   Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.muted;text:"research@example.invalid";textFormat:Text.PlainText}
   Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.records.length?analysis.app.trx("اكتمل • 3 عناصر اصطناعية", "Completed • 3 synthetic entities"):analysis.app.trx("لم يُشغّل الاختبار", "Fixture not run")}
   OsintDesktopCommand {objectName:"osintLoadAnalysisFixture";Layout.fillWidth:true;primary:true;text:analysis.app.trx("تحميل مثال محلي", "Load local example");enabled:!!analysis.bridge&&!analysis.bridge.busy;onClicked:analysis.bridge.runSynthetic(0)}
   Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("0 Photon\nلا اتصال بالمزوّد", "0 Photons\nNo provider connection")}
   Item {Layout.fillHeight:true;Layout.minimumHeight:1}
  }
  OsintDesktopPane {
   wallpaper:true
   SplitView.fillWidth:true;SplitView.fillHeight:true;SplitView.minimumWidth:340;SplitView.preferredHeight:510
   title:analysis.app.trx("مساحة التحليل", "Analysis workspace");iconName:"search_visual"
   RowLayout {
    Layout.fillWidth:true
    OsintDesktopCommand {text:analysis.app.trx("جدول", "Table");primary:analysis.viewMode===0;onClicked:analysis.viewMode=0}
    OsintDesktopCommand {text:analysis.app.trx("علاقات", "Relationships");primary:analysis.viewMode===1;onClicked:analysis.viewMode=1}
   }
   Label {font.pixelSize:12;visible:!analysis.records.length;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("حمّل المثال المحلي لعرض الجدول ورسم العلاقات. لن تُنفّذ أي عملية بحث خارجية.", "Load the local example to inspect its table and relationship graph. No external search is performed.")}
   ColumnLayout {
    visible:analysis.viewMode===0;Layout.fillWidth:true;spacing:8
    TextField {objectName:"osintEntityFilter";Layout.fillWidth:true;placeholderText:analysis.app.trx("تصفية العناصر…", "Filter entities…");onTextChanged:analysis.filterText=text;maximumLength:200;selectByMouse:true;color:"#E5EDF3";palette.base:"#08121B";palette.text:"#E5EDF3";palette.placeholderText:"#9EB4C1"}
    RowLayout {
     Layout.fillWidth:true
     Label {font.pixelSize:12;text:analysis.app.trx("العنصر / النوع", "Entity / type");color:analysis.app.design.muted;Layout.fillWidth:true}
     OsintDesktopCommand {text:analysis.descending?analysis.app.trx("ترتيب تنازلي", "Descending"):analysis.app.trx("ترتيب تصاعدي", "Ascending");onClicked:analysis.descending=!analysis.descending}
    }
    Rectangle {Layout.fillWidth:true;implicitHeight:27;color:"#0D2A3D";border.color:"#24556D"
     RowLayout {anchors.fill:parent;anchors.leftMargin:8;anchors.rightMargin:8
      Label {text:analysis.app.trx("القيمة", "Value");Layout.fillWidth:true;color:"#BCCED8";font.pixelSize:11;font.bold:true}
      Label {text:analysis.app.trx("النوع", "Type");Layout.preferredWidth:85;color:"#BCCED8";font.pixelSize:11;font.bold:true}
      Label {text:analysis.app.trx("المصدر", "Source");Layout.preferredWidth:70;color:"#BCCED8";font.pixelSize:11;font.bold:true}
     }
    }
    Repeater {
     model:analysis.filtered
     delegate:Rectangle {
      required property var modelData
      Layout.fillWidth:true;implicitHeight:34;radius:0
      color:analysis.selectedIndex===modelData.index?"#0C435C":analysis.app.design.raised
      border.color:analysis.selectedIndex===modelData.index?analysis.app.design.accent:analysis.app.design.border
      RowLayout {
       anchors.fill:parent;anchors.margins:7;spacing:8
       Image {source:"qrc:/assets/icons/"+modelData.row.icon+".svg";Layout.preferredWidth:17;Layout.preferredHeight:17;sourceSize:Qt.size(20,20)}
       Label {text:modelData.row.value;textFormat:Text.PlainText;color:analysis.app.design.text;Layout.fillWidth:true;elide:Text.ElideMiddle;font.pixelSize:12}
       Label {text:modelData.row.type;color:analysis.app.design.muted;font.pixelSize:11;Layout.preferredWidth:85;elide:Text.ElideRight}
       Label {text:analysis.app.trx("اصطناعي", "Synthetic");color:analysis.app.design.muted;font.pixelSize:11;Layout.preferredWidth:70;elide:Text.ElideRight}
      }
      MouseArea {anchors.fill:parent;onClicked:analysis.pick(parent.modelData.index)}
      Accessible.role:Accessible.Button;Accessible.name:modelData.row.value
      activeFocusOnTab:true;Keys.onReturnPressed:analysis.pick(modelData.index);Keys.onSpacePressed:analysis.pick(modelData.index)
     }
    }
    Label {font.pixelSize:12;visible:analysis.records.length>0&&analysis.filtered.length===0;text:analysis.app.trx("لا توجد عناصر مطابقة", "No matching entities");color:analysis.app.design.muted}
   }
   ColumnLayout {
    visible:analysis.viewMode===1;Layout.fillWidth:true;spacing:8
    RowLayout {
     OsintDesktopCommand {text:"−";enabled:graph.zoom>0.6;onClicked:graph.zoom=Math.max(0.6,graph.zoom-0.1);Accessible.name:analysis.app.trx("تصغير", "Zoom out")}
     OsintDesktopCommand {text:"+";enabled:graph.zoom<1.8;onClicked:graph.zoom=Math.min(1.8,graph.zoom+0.1);Accessible.name:analysis.app.trx("تكبير", "Zoom in")}
     OsintDesktopCommand {text:analysis.app.trx("إعادة ضبط", "Reset");onClicked:{graph.zoom=1;graph.contentX=0;graph.contentY=0}}
    }
    Flickable {
     id:graph;objectName:"osintRelationshipGraph";property real zoom:1
     Layout.fillWidth:true;Layout.preferredHeight:350;clip:true
     contentWidth:720*zoom;contentHeight:380*zoom;boundsBehavior:Flickable.StopAtBounds
     Rectangle {width:720;height:380;scale:graph.zoom;transformOrigin:Item.TopLeft;color:"transparent"
      Repeater {model:analysis.records.length?2:0
       delegate:Rectangle {required property int index;x:260;y:190;width:226;height:2;color:"#2F8DB1";rotation:index===0?-28.1:27.7;transformOrigin:Item.Left}
      }
      Repeater {model:analysis.records
       delegate:Rectangle {
        required property int index;required property var modelData
        x:index===0?50:460;y:index===0?145:index===1?38:250;width:210;height:76;radius:3
        color:analysis.selectedIndex===index?"#0C435C":"#0A2231";border.color:analysis.selectedIndex===index?"#23B9E7":"#24556D"
        Column {anchors.fill:parent;anchors.margins:12;spacing:8
         Label {font.pixelSize:12;text:parent.parent.modelData.type;color:"#70D8F5";font.bold:true}
         Label {font.pixelSize:12;text:parent.parent.modelData.value;textFormat:Text.PlainText;color:"#E5EDF3";width:parent.width;elide:Text.ElideMiddle}
        }
        MouseArea {anchors.fill:parent;onClicked:analysis.pick(parent.index)}
        Accessible.role:Accessible.Button;Accessible.name:modelData.value;activeFocusOnTab:true;Keys.onReturnPressed:analysis.pick(index)
       }
      }
     }
     ScrollBar.horizontal:ScrollBar {}
     ScrollBar.vertical:ScrollBar {}
    }
    Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;text:analysis.app.trx("الروابط جزء من المثال الاصطناعي؛ لا تثبت علاقة حقيقية بين أشخاص أو حسابات.", "Links belong to the synthetic example; they do not establish real relationships between people or accounts.");color:analysis.app.design.muted}
   }
  }
  OsintDesktopPane {
   visible:analysis.showInspector;SplitView.preferredWidth:245;SplitView.minimumWidth:180;SplitView.preferredHeight:320;Layout.fillWidth:analysis.width<=1060;Layout.fillHeight:true;Layout.minimumWidth:0
   title:analysis.app.trx("الخصائص والمصدر", "Properties & source");iconName:"document_text"
   Label {text:analysis.selected.value||analysis.app.trx("حدد عنصرًا", "Select an entity");textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.text;font.pixelSize:13;font.bold:true}
   Label {font.pixelSize:12;text:analysis.app.trx("مرجع المصدر", "Source reference");color:analysis.app.design.muted}
   Label {font.pixelSize:12;text:analysis.selected.source||"—";textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.accent}
   Label {font.pixelSize:12;text:analysis.app.trx("حالة التحقق", "Verification status");color:analysis.app.design.muted}
   StateBadge {text:analysis.app.trx("اصطناعي • ليس دليلًا", "SYNTHETIC • NOT EVIDENCE");tint:analysis.app.design.silver}
   Label {font.pixelSize:12;text:analysis.app.trx("الوقت المدوّن", "Recorded time");color:analysis.app.design.muted}
   Label {font.pixelSize:12;text:analysis.selected.at||"—";Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.text}
   Label {font.pixelSize:12;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("عرض محلي فقط. ربط الأدلة والتصدير ينتظر تنفيذًا موثقًا ومختبرًا.", "Local view only. Evidence linkage and export await documented, tested implementation.")}
   Item {Layout.fillHeight:true;Layout.minimumHeight:1}
  }
 }
 }
}
