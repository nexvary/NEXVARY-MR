import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
 id: card
 property string title: ""
 default property alias body: container.data
 color: "#081824"
 border.color: "#1e3649"
 radius: 8
 implicitHeight: column.implicitHeight + 28
 ColumnLayout {
  id: column
  anchors.fill: parent; anchors.margins: 14; spacing: 12
  Label { visible: card.title.length>0; text: card.title; font.pixelSize: 15; font.bold: true; color: "#e1ecfa"; Layout.fillWidth: true }
  ColumnLayout { id:container; Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10 }
 }
}
