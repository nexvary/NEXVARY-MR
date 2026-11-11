import QtQuick
QtObject {
 property bool dark:true
 readonly property color background:dark?"#040507":"#EDF1F7"
 readonly property color surface:dark?"#08121B":"#FFFFFF"
 readonly property color raised:dark?"#0A2231":"#E7EDF5"
 readonly property color border:dark?"#24556D":"#BCC8D8"
 readonly property color silver:dark?"#BCCED8":"#52647B"
 readonly property color text:dark?"#E5EDF3":"#172A43"
 readonly property color muted:dark?"#9EB4C1":"#536880"
 readonly property color accent:dark?"#23B9E7":"#096DC0"
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
