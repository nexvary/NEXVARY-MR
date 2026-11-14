import QtQuick
import QtQuick.Controls
Menu {
    id:menu
    property var editor:parent
    readonly property bool arabic:editor && editor.LayoutMirroring.enabled === true
    readonly property bool copyAllowed:!!editor && editor.selectedText.length > 0
        && (editor.echoMode === undefined || editor.echoMode === TextInput.Normal)
    MenuItem {objectName:"editCut";text:menu.arabic?"قص":"Cut";enabled:menu.copyAllowed && !menu.editor.readOnly;onTriggered:menu.editor.cut()}
    MenuItem {objectName:"editCopy";text:menu.arabic?"نسخ":"Copy";enabled:menu.copyAllowed;onTriggered:menu.editor.copy()}
    MenuItem {objectName:"editPaste";text:menu.arabic?"لصق":"Paste";enabled:!!menu.editor && !menu.editor.readOnly && menu.editor.canPaste;onTriggered:menu.editor.paste()}
    MenuSeparator {}
    MenuItem {objectName:"editSelectAll";text:menu.arabic?"تحديد الكل":"Select all";enabled:!!menu.editor && menu.editor.length > 0;onTriggered:menu.editor.selectAll()}
    Shortcut {sequence:"Shift+F10";enabled:!!menu.editor && menu.editor.activeFocus;onActivated:menu.popup(0,menu.editor.height)}
    Shortcut {sequence:"Menu";enabled:!!menu.editor && menu.editor.activeFocus;onActivated:menu.popup(0,menu.editor.height)}
}
