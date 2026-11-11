import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 id: analysis
 objectName: "osintAnalysisPane"
 property var app
 readonly property var bridge: typeof lampyreEvaluation !== "undefined" ? lampyreEvaluation : null
 readonly property var records: bridge && bridge.result.records ? bridge.result.records : []
 property int viewMode: 0
 property int selectedIndex: -1
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
 spacing: 12
 Connections {target:analysis.bridge;function onChanged(){analysis.selectedIndex=-1}}
 RowLayout {
  Layout.fillWidth:true
  Label {text:analysis.app.trx("مستكشف النتائج", "Results explorer");font.pixelSize:23;font.bold:true;color:analysis.app.design.text;Layout.fillWidth:true}
  StateBadge {text:analysis.app.trx("اصطناعي • دون اتصال", "SYNTHETIC • OFFLINE");tint:analysis.app.design.accent}
 }
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("تجربة محلية لعرض البيانات؛ ليست نتائج من Lampyre ولا أدلة قضية. تحديد عنصر يفتح مرجعه وتفاصيله.", "Local data-viewing fixture; not Lampyre findings or case evidence. Select an entity to inspect its reference and details.")}
 GridLayout {
  Layout.fillWidth:true;columns:width>1060?3:1;columnSpacing:12;rowSpacing:12
  Panel {
   Layout.preferredWidth:210;Layout.fillWidth:analysis.width<=1060;Layout.fillHeight:true;Layout.minimumWidth:0
   title:analysis.app.trx("الطلبات", "Requests");iconName:"history"
   StateBadge {text:analysis.app.trx("جلسة محلية", "LOCAL SESSION");tint:analysis.app.design.silver}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.text;font.bold:true;text:"Lampyre / UI fixture"}
   Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.muted;text:"research@example.invalid";textFormat:Text.PlainText}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.records.length?analysis.app.trx("اكتمل • 3 عناصر اصطناعية", "Completed • 3 synthetic entities"):analysis.app.trx("لم يُشغّل الاختبار", "Fixture not run")}
   ActionButton {objectName:"osintLoadAnalysisFixture";Layout.fillWidth:true;primary:true;text:analysis.app.trx("تحميل مثال محلي", "Load local example");enabled:!!analysis.bridge&&!analysis.bridge.busy;onClicked:analysis.bridge.runSynthetic(0)}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("0 Photon\nلا اتصال بالمزوّد", "0 Photons\nNo provider connection")}
   Item {Layout.fillHeight:true;Layout.minimumHeight:1}
  }
  Panel {
   Layout.fillWidth:true;Layout.minimumWidth:0;Layout.fillHeight:true
   title:analysis.app.trx("مساحة التحليل", "Analysis workspace");iconName:"search_visual"
   RowLayout {
    Layout.fillWidth:true
    ActionButton {text:analysis.app.trx("جدول", "Table");primary:analysis.viewMode===0;onClicked:analysis.viewMode=0}
    ActionButton {text:analysis.app.trx("علاقات", "Relationships");primary:analysis.viewMode===1;onClicked:analysis.viewMode=1}
   }
   Label {visible:!analysis.records.length;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("حمّل المثال المحلي لعرض الجدول ورسم العلاقات. لن تُنفّذ أي عملية بحث خارجية.", "Load the local example to inspect its table and relationship graph. No external search is performed.")}
   ColumnLayout {
    visible:analysis.viewMode===0;Layout.fillWidth:true;spacing:8
    TextField {objectName:"osintEntityFilter";Layout.fillWidth:true;placeholderText:analysis.app.trx("تصفية العناصر…", "Filter entities…");onTextChanged:analysis.filterText=text;maximumLength:200;selectByMouse:true}
    RowLayout {
     Layout.fillWidth:true
     Label {text:analysis.app.trx("العنصر / النوع", "Entity / type");color:analysis.app.design.muted;Layout.fillWidth:true}
     ActionButton {text:analysis.descending?analysis.app.trx("ترتيب تنازلي", "Descending"):analysis.app.trx("ترتيب تصاعدي", "Ascending");onClicked:analysis.descending=!analysis.descending}
    }
    Repeater {
     model:analysis.filtered
     delegate:Rectangle {
      required property var modelData
      Layout.fillWidth:true;implicitHeight:76;radius:8
      color:analysis.selectedIndex===modelData.index?"#163653":analysis.app.design.raised
      border.color:analysis.selectedIndex===modelData.index?analysis.app.design.accent:analysis.app.design.border
      RowLayout {
       anchors.fill:parent;anchors.margins:12;spacing:12
       Image {source:"qrc:/assets/icons/"+modelData.row.icon+".svg";Layout.preferredWidth:28;Layout.preferredHeight:28;sourceSize:Qt.size(28,28)}
       ColumnLayout {Layout.fillWidth:true;spacing:5
        Label {text:modelData.row.value;textFormat:Text.PlainText;color:analysis.app.design.text;font.bold:true;Layout.fillWidth:true;elide:Text.ElideMiddle}
        Label {text:modelData.row.type+" · "+analysis.app.trx("اصطناعي غير متحقق", "Synthetic, unverified");color:analysis.app.design.muted;font.pixelSize:11;Layout.fillWidth:true;elide:Text.ElideRight}
       }
      }
      MouseArea {anchors.fill:parent;onClicked:analysis.pick(parent.modelData.index)}
      Accessible.role:Accessible.Button;Accessible.name:modelData.row.value
      activeFocusOnTab:true;Keys.onReturnPressed:analysis.pick(modelData.index);Keys.onSpacePressed:analysis.pick(modelData.index)
     }
    }
    Label {visible:analysis.records.length>0&&analysis.filtered.length===0;text:analysis.app.trx("لا توجد عناصر مطابقة", "No matching entities");color:analysis.app.design.muted}
   }
   ColumnLayout {
    visible:analysis.viewMode===1;Layout.fillWidth:true;spacing:8
    RowLayout {
     ActionButton {text:"−";enabled:graph.zoom>0.6;onClicked:graph.zoom=Math.max(0.6,graph.zoom-0.1);Accessible.name:analysis.app.trx("تصغير", "Zoom out")}
     ActionButton {text:"+";enabled:graph.zoom<1.8;onClicked:graph.zoom=Math.min(1.8,graph.zoom+0.1);Accessible.name:analysis.app.trx("تكبير", "Zoom in")}
     ActionButton {text:analysis.app.trx("إعادة ضبط", "Reset");onClicked:{graph.zoom=1;graph.contentX=0;graph.contentY=0}}
    }
    Flickable {
     id:graph;objectName:"osintRelationshipGraph";property real zoom:1
     Layout.fillWidth:true;Layout.preferredHeight:380;clip:true
     contentWidth:720*zoom;contentHeight:380*zoom;boundsBehavior:Flickable.StopAtBounds
     Rectangle {width:720;height:380;scale:graph.zoom;transformOrigin:Item.TopLeft;color:"#0C121C"
      Repeater {model:analysis.records.length?2:0
       delegate:Rectangle {required property int index;x:260;y:190;width:226;height:2;color:"#4C7AA3";rotation:index===0?-28.1:27.7;transformOrigin:Item.Left}
      }
      Repeater {model:analysis.records
       delegate:Rectangle {
        required property int index;required property var modelData
        x:index===0?50:460;y:index===0?145:index===1?38:250;width:210;height:90;radius:12
        color:analysis.selectedIndex===index?"#173C5B":"#142131";border.color:analysis.selectedIndex===index?"#65BAFF":"#3A617F"
        Column {anchors.fill:parent;anchors.margins:12;spacing:8
         Label {text:parent.parent.modelData.type;color:"#74BDFF";font.bold:true}
         Label {text:parent.parent.modelData.value;textFormat:Text.PlainText;color:"#EEF4FC";width:parent.width;elide:Text.ElideMiddle}
        }
        MouseArea {anchors.fill:parent;onClicked:analysis.pick(parent.index)}
        Accessible.role:Accessible.Button;Accessible.name:modelData.value;activeFocusOnTab:true;Keys.onReturnPressed:analysis.pick(index)
       }
      }
     }
     ScrollBar.horizontal:ScrollBar {}
     ScrollBar.vertical:ScrollBar {}
    }
    Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;text:analysis.app.trx("الروابط جزء من المثال الاصطناعي؛ لا تثبت علاقة حقيقية بين أشخاص أو حسابات.", "Links belong to the synthetic example; they do not establish real relationships between people or accounts.");color:analysis.app.design.muted}
   }
  }
  Panel {
   Layout.preferredWidth:245;Layout.fillWidth:analysis.width<=1060;Layout.fillHeight:true;Layout.minimumWidth:0
   title:analysis.app.trx("الخصائص والمصدر", "Properties & source");iconName:"document_text"
   Label {text:analysis.selected.value||analysis.app.trx("حدد عنصرًا", "Select an entity");textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.text;font.pixelSize:17;font.bold:true}
   Label {text:analysis.app.trx("مرجع المصدر", "Source reference");color:analysis.app.design.muted}
   Label {text:analysis.selected.source||"—";textFormat:Text.PlainText;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.accent}
   Label {text:analysis.app.trx("حالة التحقق", "Verification status");color:analysis.app.design.muted}
   StateBadge {text:analysis.app.trx("اصطناعي • ليس دليلًا", "SYNTHETIC • NOT EVIDENCE");tint:analysis.app.design.silver}
   Label {text:analysis.app.trx("الوقت المدوّن", "Recorded time");color:analysis.app.design.muted}
   Label {text:analysis.selected.at||"—";Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;color:analysis.app.design.text}
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:analysis.app.design.muted;text:analysis.app.trx("عرض محلي فقط. ربط الأدلة والتصدير ينتظر تنفيذًا موثقًا ومختبرًا.", "Local view only. Evidence linkage and export await documented, tested implementation.")}
   Item {Layout.fillHeight:true;Layout.minimumHeight:1}
  }
 }
}
