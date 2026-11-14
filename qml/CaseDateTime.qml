import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "InputRules.js" as Rules
ColumnLayout {
    id:picker
    property var app
    property string text:""
    property bool requiredInput:false
    property bool rangeValid:true
    property bool syncing:false
    property var parts:({year:2026,month:1,day:1,hour:0,minute:0,second:0})
    readonly property string inputState:Rules.state(text,requiredInput,"dateTime")==="valid"&&!rangeValid?"invalid":Rules.state(text,requiredInput,"dateTime")
    readonly property color inputColor:inputState==="empty"?"#FF3131":inputState==="valid"?"#39FF14":inputState==="invalid"?"#FFD166":"#C0C0C0"
    signal edited(string value)
    spacing:5
    function sync() {
        syncing=true;
        const parsed=Rules.parseUtc(text), now=new Date();
        parts=parsed||{year:now.getUTCFullYear(),month:now.getUTCMonth()+1,day:now.getUTCDate(),hour:now.getUTCHours(),minute:now.getUTCMinutes(),second:now.getUTCSeconds()};
        syncing=false;
    }
    function commit() {
        const p=parts;
        const value=Rules.utcDate(p.year,p.month,p.day,p.hour,p.minute,p.second).toISOString().replace(/\.\d{3}Z$/,"Z");
        edited(value);
    }
    function setPart(key,value) {
        if(syncing)return;
        const next=Object.assign({},parts);next[key]=value;
        next.day=Math.min(next.day,Rules.daysInMonth(next.year,next.month));
        syncing=true;parts=next;syncing=false;commit();
    }
    onTextChanged:sync()
    Component.onCompleted:sync()
    NeonCheckBox {
        objectName:"dateEnabled"
        checked:picker.text!==""
        text:picker.app.trx("تحديد التاريخ والوقت (UTC)","Set date and time (UTC)")
        onToggled:{if(checked)picker.commit();else picker.edited("")}
    }
    GridLayout {
        Layout.fillWidth:true
        columns:width>=540?6:3
        columnSpacing:8;rowSpacing:6
        Repeater {
            model:[{key:"year",ar:"السنة",en:"Year",min:1,max:9999},{key:"month",ar:"الشهر",en:"Month",min:1,max:12},{key:"day",ar:"اليوم",en:"Day",min:1,max:31},{key:"hour",ar:"الساعة",en:"Hour",min:0,max:23},{key:"minute",ar:"الدقيقة",en:"Minute",min:0,max:59},{key:"second",ar:"الثانية",en:"Second",min:0,max:59}]
            delegate:ColumnLayout {
                required property var modelData
                Layout.fillWidth:true;Layout.minimumWidth:0
                Label {text:picker.app.ar?modelData.ar:modelData.en;color:picker.app.design.muted}
                SpinBox {
                    id:spin
                    objectName:"datePart-"+modelData.key
                    Layout.fillWidth:true;Layout.minimumWidth:0;implicitWidth:110
                    enabled:picker.text!==""
                    editable:true
                    palette.text:picker.app.design.text
                    palette.buttonText:picker.app.design.text
                    from:modelData.min
                    to:modelData.key==="day"?Rules.daysInMonth(picker.parts.year,picker.parts.month):modelData.max
                    value:picker.parts[modelData.key]
                    onValueModified:picker.setPart(modelData.key,value)
                    textFromValue:(value,locale)=>String(value)
                    valueFromText:(text,locale)=>Number(Rules.digits(text))
                    background:Rectangle {color:picker.app.design.surface;border.color:picker.inputColor;radius:5}
                    up.indicator:Rectangle {x:spin.mirrored?0:spin.width-width;width:24;height:spin.height;color:spin.up.pressed?"#143A50":"#112331";Text {anchors.centerIn:parent;text:"+";color:"#E5EDF3"}}
                    down.indicator:Rectangle {x:spin.mirrored?spin.width-width:0;width:24;height:spin.height;color:spin.down.pressed?"#143A50":"#112331";Text {anchors.centerIn:parent;text:"-";color:"#E5EDF3"}}
                    EditMenu {id:spinMenu;editor:spin.contentItem}
                    TapHandler {acceptedButtons:Qt.RightButton;onTapped:(point,button)=>{spin.contentItem.forceActiveFocus();spinMenu.popup(point.position.x,point.position.y)}}
                }
            }
        }
    }
    Label {Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:picker.inputColor;text:picker.text||picker.app.trx("اختياري — لم يُحدد تاريخ","Optional — no date set")}
    Label {visible:!picker.rangeValid;Layout.fillWidth:true;wrapMode:Text.WordWrap;color:picker.inputColor;text:picker.app.trx("نهاية السريان يجب ألا تسبق البداية","The end must not precede the start")}
    Button {visible:picker.text!==""&&!Rules.parseUtc(picker.text);text:picker.app.trx("استبدال القيمة غير الصحيحة بالتاريخ المحدد","Replace invalid value with selected date");onClicked:picker.commit()}
}
