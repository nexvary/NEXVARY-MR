import QtQuick
QtObject {
 property bool dark:true
 readonly property color background:dark?"#0A0A0A":"#EDF1F7"
 readonly property color surface:dark?"#11151B":"#FFFFFF"
 readonly property color raised:dark?"#191F28":"#E7EDF5"
 readonly property color border:dark?"#3C4859":"#BCC8D8"
 readonly property color silver:dark?"#B6C4D5":"#52647B"
 readonly property color text:dark?"#EEF4FC":"#172A43"
 readonly property color muted:dark?"#9AAABD":"#536880"
 readonly property color accent:dark?"#269AFF":"#096DC0"
 readonly property color success:dark?"#75FBA0":"#18713B"
 readonly property color danger:dark?"#FF6A7D":"#B4213F"
 readonly property int unit:4
 readonly property int gap:12
 readonly property int inset:16
 readonly property int radius:12
 readonly property int topBarHeight:72
 readonly property int expandedSidebar:224
 readonly property int collapsedSidebar:72
}
