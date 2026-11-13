import QtQuick
import QtQuick.Controls
import QtTest
import ".." as MR
import "../InputRules.js" as Rules
TestCase {
    id:test
    name:"InputEditing"
    when:windowShown
    width:800;height:650;visible:true
    QtObject {
        id:style
        property bool ar:false
        property var design:({text:"#E5EDF3",muted:"#9EB4C1",surface:"#08121B"})
        function trx(arabic,english){return ar?arabic:english}
    }
    Component {id:fieldComponent;MR.EntryField {width:300;height:42}}
    Component {id:areaComponent;MR.EntryArea {width:300;height:100}}
    Component {id:dateComponent;MR.CaseDateTime {width:480;app:style;onEdited:value=>text=value}}
    function test_validationFeedback(){
        const field=createTemporaryObject(fieldComponent,test,{validationKind:"integer"});verify(field)
        compare(field.inputState,"empty");compare(String(field.inputColor),"#ff3131")
        field.text="اسم";compare(field.inputState,"invalid");compare(String(field.inputColor),"#ffd166")
        field.text="٢٠٢٦";compare(field.inputState,"valid");compare(String(field.inputColor),"#39ff14")
        field.text="";field.requiredInput=false;compare(field.inputState,"neutral")
        field.validationKind="password";field.text="short";compare(field.inputState,"invalid")
        field.text="Synthetic valid password 2026!";compare(field.inputState,"valid")
        field.text="";field.configuredValue=true;compare(field.inputState,"valid")
    }
    function test_clipboardAndContextMenu_data(){return [{tag:"field",component:fieldComponent},{tag:"multiline",component:areaComponent}]}
    function test_clipboardAndContextMenu(data){
        const editor=createTemporaryObject(data.component,test,{text:"Synthetic clipboard"});verify(editor)
        editor.forceActiveFocus();tryCompare(editor,"activeFocus",true)
        keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_C,Qt.ControlModifier)
        editor.text="";keyClick(Qt.Key_V,Qt.ControlModifier);compare(editor.text,"Synthetic clipboard")
        keyClick(Qt.Key_A,Qt.ControlModifier);keyClick(Qt.Key_X,Qt.ControlModifier);compare(editor.text,"")
        keyClick(Qt.Key_V,Qt.ControlModifier);compare(editor.text,"Synthetic clipboard")
        const menu=findChild(editor,"entryEditMenu");verify(menu);menu.popup(0,editor.height);tryCompare(menu,"opened",true);menu.close()
    }
    function test_calendarClampingAndOptionalDates(){
        const picker=createTemporaryObject(dateComponent,test);verify(picker)
        compare(picker.text,"");compare(picker.inputState,"neutral")
        picker.text="2024-02-29T23:59:59Z";compare(picker.inputState,"valid")
        picker.rangeValid=false;compare(picker.inputState,"invalid");picker.rangeValid=true;
        picker.setPart("year",2025);compare(picker.text,"2025-02-28T23:59:59Z")
        picker.setPart("month",4);picker.setPart("day",30);compare(picker.text,"2025-04-30T23:59:59Z")
        picker.text="name";compare(picker.inputState,"invalid")
        verify(!Rules.valid("2025-02-29T12:00:00Z","dateTime"))
        verify(!Rules.valid("2024-04-31T12:00:00Z","dateTime"))
        verify(!Rules.valid("2024-04-30T24:00:00Z","dateTime"))
        verify(Rules.valid("0099-02-28T12:00:00Z","dateTime"))
        style.ar=true;picker.text="2026-11-13T12:00:00Z";compare(picker.inputState,"valid");style.ar=false
    }
}
