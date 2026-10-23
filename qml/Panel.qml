import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
GlossSurface {
 id: card
 reflection:0.17
 readonly property var themeWindow:ApplicationWindow.window
 function tone(value){return themeWindow?themeWindow.tone(value):value}
 property string title: ""
 // Silver is the default; accent colors identify selected functional panels.
 property string accent: "silver"
 readonly property bool dark: !themeWindow || themeWindow.darkTheme
 readonly property color accentColor: accent === "blue" ? (dark ? "#29B6FF" : "#006BA6") :
     accent === "green" ? (dark ? "#39FF14" : "#247A16") : (dark ? "#C8D2DD" : "#667586")
 default property alias body: container.data
 color: card.dark ? "#191919" : "#EBF0F4"
 border.color: card.accentColor
 border.width: 2
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
    color: card.accentColor
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
   gradient: Gradient {
    orientation: Gradient.Horizontal
    GradientStop { position: 0; color: Qt.rgba(card.accentColor.r, card.accentColor.g, card.accentColor.b, 0.25) }
    GradientStop { position: 0.45; color: card.accentColor }
    GradientStop { position: 1; color: Qt.rgba(card.accentColor.r, card.accentColor.g, card.accentColor.b, 0.25) }
   }
  }
  ColumnLayout {
   id: container
   Layout.fillWidth: true
   Layout.fillHeight: true
   spacing: 10
  }
 }
}
