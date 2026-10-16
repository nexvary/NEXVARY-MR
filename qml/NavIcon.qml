import QtQuick
Item {
    id: icon
    property int kind: 0
    property color stroke: "#91bfff"
    implicitWidth: 28
    implicitHeight: 28
    readonly property var glyphs: ["⌂", "▣", "⌕", "▦", "⇅", "☷", "▤", "⚙"]
    readonly property var accents: ["#54c7ec", "#73a9ff", "#40d6b2", "#b4a0ff", "#f6b85d", "#55c7e8", "#ef92c1", "#9aaaff"]
    Rectangle {
        anchors.centerIn: parent
        width: 27
        height: 27
        radius: 7
        color: Qt.rgba(0.12, 0.22, 0.34, 0.75)
        border.color: Qt.rgba(0.33, 0.49, 0.64, 0.5)
    }
    Text {
        anchors.centerIn: parent
        text: icon.glyphs[Math.max(0, Math.min(7, icon.kind))]
        color: icon.accents[Math.max(0, Math.min(7, icon.kind))]
        font.family: "Segoe UI Symbol"
        font.pixelSize: 21
        font.bold: true
        renderType: Text.NativeRendering
    }
}
