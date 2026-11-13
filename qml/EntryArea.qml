import QtQuick
import QtQuick.Controls
import "InputRules.js" as Rules
TextArea {
    id:entry
    property bool requiredInput:true
    property bool configuredValue:false
    property string validationKind:"text"
    property var validateInput:null
    readonly property string inputState:readOnly?"neutral":!text.trim()&&configuredValue?"valid":Rules.state(text,requiredInput,validationKind)==="valid" && validateInput && !validateInput(text)?"invalid":Rules.state(text,requiredInput,validationKind)
    readonly property color inputColor:inputState==="empty"?"#FF3131":inputState==="valid"?"#39FF14":inputState==="invalid"?"#FFD166":"#C0C0C0"
    selectByMouse:true
    persistentSelection:true
    Rectangle {
        anchors.fill:parent
        z:10
        radius:5
        visible:!entry.readOnly && entry.enabled && entry.inputState!=="neutral"
        color:Qt.rgba(entry.inputColor.r,entry.inputColor.g,entry.inputColor.b,0.035)
        border.width:2
        border.color:entry.inputColor
    }
    EditMenu {id:editMenu;objectName:"entryEditMenu";editor:entry}
    TapHandler {acceptedButtons:Qt.RightButton;onTapped:(point,button)=>{entry.forceActiveFocus();editMenu.popup(point.position.x,point.position.y)}}
    ToolTip.delay:600
    ToolTip.visible:hovered && !readOnly && inputState!=="neutral"
    ToolTip.text:LayoutMirroring.enabled?(inputState==="empty"?"حقل مطلوب":inputState==="invalid"?"راجع صيغة البيانات":"الصيغة صحيحة"):(inputState==="empty"?"Required field":inputState==="invalid"?"Check the input format":"Valid format")
}
