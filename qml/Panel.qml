import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlossSurface {
 id:card
 readonly property var themeWindow:ApplicationWindow.window
 readonly property var tokens:themeWindow?themeWindow.design:null
 function tone(value){return themeWindow?themeWindow.tone(value):value}
 property string title:""
 property string subtitle:""
 property string stateText:""
 property string accent:"silver"
 property string iconName: {
  const s=title.toLowerCase()
  if(/firmware|avb|النظام|الحزم|أقسام/.test(s))return "arrow_square_down"
  if(/case|custody|evidence|forensic|أدلة|قضية|قضايا|الحيازة/.test(s))return "document_lock"
  if(/backup|database|stock|inventory|مخزون|قطع|قاعدة|احتياطي/.test(s))return "database"
  if(/device|phone|هاتف|جهاز|أجهزة/.test(s))return "phone"
  if(/tool|setting|أدوات|المحرك|إعداد/.test(s))return "settings"
  if(/order|customer|workshop|عميل|العملاء|أوامر|ورشة/.test(s))return "clipboard_task"
  if(/report|تقرير|التقارير/.test(s))return "document_text"
  if(/log|timeline|سجل|السجلات|زمني/.test(s))return "history"
  if(/board|مخطط|لوحة/.test(s))return "board"
  return "gauge"
 }
 readonly property bool dark:!themeWindow||themeWindow.darkTheme
 readonly property color accentColor:tokens?(accent==="blue"?tokens.accent:accent==="green"?tokens.success:tokens.silver):"#B6C4D5"
 default property alias body:container.data
 color:tokens?tokens.surface:"#11151B";border.color:tokens?tokens.border:"#3C4859";border.width:1;radius:12;reflection:0.035
 implicitHeight:column.implicitHeight+32
 ColumnLayout {
  id:column;anchors.fill:parent;anchors.margins:16;spacing:12
  RowLayout {
   visible:card.title!=="";Layout.fillWidth:true;spacing:10
   Image {visible:card.iconName!=="";source:visible?"qrc:/assets/icons/"+card.iconName+".svg":"";sourceSize.width:40;sourceSize.height:40;Layout.preferredWidth:30;Layout.preferredHeight:30;fillMode:Image.PreserveAspectFit}
   ColumnLayout {Layout.fillWidth:true;Layout.minimumWidth:0;spacing:3
    Label {text:card.title;color:card.tokens?card.tokens.text:"#EEF4FC";font.pixelSize:14;font.bold:true;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText}
    Label {visible:card.subtitle!=="";text:card.subtitle;color:card.tokens?card.tokens.muted:"#9AAABD";font.pixelSize:11;Layout.fillWidth:true;wrapMode:Text.WordWrap;textFormat:Text.PlainText}
   }
   StateBadge {visible:card.stateText!=="";text:card.stateText;tint:card.accentColor}
  }
  Rectangle {visible:card.title!=="";Layout.fillWidth:true;height:1;color:card.tokens?card.tokens.border:"#3C4859";opacity:0.55}
  ColumnLayout {id:container;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;Layout.fillHeight:true;spacing:10}
 }
}
