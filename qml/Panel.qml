import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlossSurface {
 id: card
 reflection:0.10
 readonly property var themeWindow:ApplicationWindow.window
 function tone(value){return themeWindow?themeWindow.tone(value):value}
 property string title: ""
 default property alias body: container.data
 color: card.tone("#0b2030")
 border.color: card.tone("#29485f")
 border.width: 1
 radius: 12
 implicitHeight: column.implicitHeight + 32
 ColumnLayout {
  id: column
  anchors.fill: parent
  anchors.margins: 16
  spacing: 13
  RowLayout {
   visible: card.title.length > 0
   Layout.fillWidth: true
   spacing: 10
   Rectangle {
    width: 4
    height: 18
    radius: 2
    color: card.tone("#40b9e6")
   }
   Label {
    text: card.title
    font.pixelSize: 15
    font.bold: true
    color: card.tone("#eff6ff")
    Layout.fillWidth: true
    elide: Text.ElideRight
   }
  }
  Rectangle {
   visible: card.title.length > 0
   Layout.fillWidth: true
   height: 1
   color: card.tone("#244358")
  }
  ColumnLayout {
   id: container
   Layout.fillWidth: true
   Layout.fillHeight: true
   spacing: 10
  }
 }
}
