import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: workspace
    objectName: "boardWorkspace"
    property var app
    property real zoom: 0.6
    property var selected: ({})
    property string activeNet: ""
    property string pendingBoard: ""
    property string pendingAction: ""
    property string status: ""
    property string photoReference: ""
    property string photoPreview: ""
    onVisibleChanged: if (visible) Qt.callLater(fitBoard)
    function tr(ar, en) { return app.trx(ar, en) }
    function containsNet(component, net) {
        if (!net || !component.nets) return false
        for (var i = 0; i < component.nets.length; ++i)
            if (component.nets[i] === net) return true
        return false
    }
    function setZoom(value) {
        var centerX = (viewport.contentX + viewport.width / 2) / Math.max(1, viewport.contentWidth)
        var centerY = (viewport.contentY + viewport.height / 2) / Math.max(1, viewport.contentHeight)
        zoom = Math.max(0.05, Math.min(4, value))
        Qt.callLater(function() {
            viewport.contentX = Math.max(0, Math.min(viewport.contentWidth - viewport.width, centerX * viewport.contentWidth - viewport.width / 2))
            viewport.contentY = Math.max(0, Math.min(viewport.contentHeight - viewport.height, centerY * viewport.contentHeight - viewport.height / 2))
        })
    }
    function fitBoard() {
        if (!boardBridge.loaded || !workspace.visible || viewport.width < 80 || viewport.height < 80) return
        zoom = Math.max(0.05, Math.min(4, Math.min((viewport.width - 36) / boardBridge.boardWidth, (viewport.height - 36) / boardBridge.boardHeight)))
        viewport.contentX = 0
        viewport.contentY = 0
    }
    function requestAction(action) {
        pendingAction = action
        if (boardBridge.dirty) discardDialog.open()
        else performAction()
    }
    function performAction() {
        if (pendingAction === "training") {
            if (boardBridge.importBoard("qrc:/samples/training-board.json", true)) status = tr("تم تحميل لوحة تدريب اصطناعية", "Synthetic training board loaded")
        } else if (pendingAction === "import") {
            boardPicker.open()
        } else if (pendingAction === "session") {
            sessionPicker.open()
        }
        pendingAction = ""
    }
    Connections {
        target: boardBridge
        function onBoardChanged() { workspace.selected = ({}); workspace.activeNet = ""; searchField.text = ""; Qt.callLater(workspace.fitBoard) }
    }
    FileDialog {
        id: photoPicker
        title: workspace.tr("إرفاق صورة للمكون المحدد", "Attach a component photograph")
        fileMode: FileDialog.OpenFile
        nameFilters: ["Photographs (*.png *.jpg *.jpeg)"]
        onAccepted: if (boardBridge.addPhoto(workspace.photoReference, selectedFile.toString())) workspace.status = workspace.tr("أُرفقت الصورة بجلسة اللوحة", "Photograph attached to the board session")
    }
    Dialog {
        id: photoDialog
        title: workspace.tr("صورة المكون", "Component photograph")
        modal: true
        anchors.centerIn: parent
        width: Math.min(800, workspace.width - 24)
        height: Math.min(650, workspace.height - 24)
        standardButtons: Dialog.Close
        contentItem: Image { source: workspace.photoPreview; fillMode: Image.PreserveAspectFit; cache: false; sourceSize.width: 1600; sourceSize.height: 1600 }
    }
    FileDialog {
        id: boardPicker
        title: workspace.tr("اختر ملف لوحة مرخصًا", "Open an authorized board file")
        fileMode: FileDialog.OpenFile
        nameFilters: ["NEXVARY board (*.json)"]
        onAccepted: { workspace.pendingBoard = selectedFile.toString(); rightsCheck.checked = false; rightsDialog.open() }
    }
    FileDialog {
        id: savePicker
        title: workspace.tr("حفظ القياسات والتعليقات والصور", "Save measurements, annotations and photos")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: ["NEXVARY session (*.session.json)", "JSON (*.json)"]
        onAccepted: if (boardBridge.saveSession(selectedFile.toString())) workspace.status = workspace.tr("حُفظت جلسة العمل", "Session saved")
    }
    FileDialog {
        id: sessionPicker
        title: workspace.tr("استعادة جلسة اللوحة الحالية", "Restore a session for this board")
        fileMode: FileDialog.OpenFile
        nameFilters: ["NEXVARY session (*.json)"]
        onAccepted: if (boardBridge.loadSession(selectedFile.toString())) workspace.status = workspace.tr("استُعيدت جلسة العمل", "Session restored")
    }
    Dialog {
        id: rightsDialog
        title: workspace.tr("ترخيص ملف اللوحة", "Board file permission")
        modal: true
        anchors.centerIn: parent
        width: Math.min(500, workspace.width - 24)
        contentItem: ColumnLayout {
            spacing: 14
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: workspace.tr("تأكد من حقك في استخدام الملف. وجود حقل الترخيص داخل الملف لا يثبت ملكية المصدر.", "Confirm your right to use this file. Its license metadata does not independently prove ownership."); color: "#d9e6f5" }
            CheckBox { id: rightsCheck; Layout.fillWidth: true; text: workspace.tr("لدي ترخيص أو تصريح باستخدام هذا المخطط", "I have permission to use this board data") }
        }
        footer: DialogButtonBox {
            ActionButton { text: workspace.tr("إلغاء", "Cancel"); DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            ActionButton { text: workspace.tr("فتح الملف", "Open file"); primary: true; enabled: rightsCheck.checked; DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole }
            onRejected: rightsDialog.close()
            onAccepted: { rightsDialog.close(); if (boardBridge.importBoard(workspace.pendingBoard, true)) workspace.status = workspace.tr("تم التحقق من الملف وتحميله", "Board validated and loaded") }
        }
    }
    Dialog {
        id: discardDialog
        contentHeight: 110
        title: workspace.tr("جلسة غير محفوظة", "Unsaved session")
        modal: true
        anchors.centerIn: parent
        width: Math.min(440, workspace.width - 24)
        standardButtons: Dialog.Yes | Dialog.Cancel
        contentItem: Label { width: discardDialog.availableWidth; wrapMode: Text.WordWrap; text: workspace.tr("ستستبدل العملية القياسات والتعليقات والصور الحالية. هل تريد المتابعة؟ يمكنك الإلغاء وحفظ الجلسة أولًا.", "This will replace the current measurements, annotations and photos. Continue? Cancel to save the session first."); color: "#dbe8f4" }
        onAccepted: workspace.performAction()
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        Flow {
            Layout.fillWidth: true
            spacing: 7
            ActionButton { objectName: "importBoard"; text: workspace.tr("فتح لوحة مرخصة", "Open licensed board"); primary: true; onClicked: workspace.requestAction("import") }
            ActionButton { objectName: "loadTrainingBoard"; text: workspace.tr("لوحة تدريب اصطناعية", "Synthetic training board"); onClicked: workspace.requestAction("training") }
            ActionButton { text: workspace.tr("حفظ الجلسة", "Save session") + (boardBridge.dirty ? " *" : ""); enabled: boardBridge.loaded; onClicked: savePicker.open() }
            ActionButton { text: workspace.tr("استعادة جلسة", "Restore session"); enabled: boardBridge.loaded; onClicked: workspace.requestAction("session") }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: warningLabel.implicitHeight + 18
            color: "#252514"; radius: 6; border.color: "#5e542c"
            Label { id: warningLabel; anchors.fill: parent; anchors.margins: 9; wrapMode: Text.WordWrap; color: "#e9cb7d"; font.pixelSize: 12; text: workspace.tr("عارض مكونات وشبكات أصلي. لوحة التدريب خيالية والقيم توضيحية فقط. القياسات تُدخل يدويًا؛ لا توجد أداة قياس متصلة.", "Native component/net viewer. The training board is fictional; its values are illustrative only. Measurements are entered manually; no measurement hardware is connected.") }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 300
                spacing: 7
                RowLayout {
                    Layout.fillWidth: true
                    TextField { id: searchField; objectName: "boardSearch"; Layout.fillWidth: true; enabled: boardBridge.loaded; placeholderText: workspace.tr("مكون أو شبكة…", "Component or net…"); selectByMouse: true }
                    ActionButton { text: "−"; implicitWidth: 36; enabled: boardBridge.loaded; onClicked: workspace.setZoom(workspace.zoom / 1.2) }
                    Label { text: Math.round(workspace.zoom * 100) + "%"; color: "#91b5d1"; Layout.preferredWidth: 42; horizontalAlignment: Text.AlignHCenter }
                    ActionButton { text: "+"; implicitWidth: 36; enabled: boardBridge.loaded; onClicked: workspace.setZoom(workspace.zoom * 1.2) }
                    ActionButton { text: workspace.tr("ملاءمة", "Fit"); implicitWidth: 54; enabled: boardBridge.loaded; onClicked: workspace.fitBoard() }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#050e17"; radius: 8; border.color: "#203d52"; clip: true
                    Flickable {
                        id: viewport
                        onWidthChanged: if (workspace.visible) Qt.callLater(workspace.fitBoard)
                        onHeightChanged: if (workspace.visible) Qt.callLater(workspace.fitBoard)
                        anchors.fill: parent
                        anchors.margins: 1
                        contentWidth: Math.max(width, circuit.width + 32)
                        contentHeight: Math.max(height, circuit.height + 32)
                        boundsBehavior: Flickable.StopAtBounds
                        clip: true
                        ScrollBar.horizontal: ScrollBar {}
                        ScrollBar.vertical: ScrollBar {}
                        Rectangle {
                            id: circuit
                            visible: boardBridge.loaded
                            x: (viewport.contentWidth - width) / 2
                            y: (viewport.contentHeight - height) / 2
                            width: boardBridge.boardWidth * workspace.zoom
                            height: boardBridge.boardHeight * workspace.zoom
                            radius: 12 * workspace.zoom
                            color: "#0c2a25"
                            border.color: "#356756"
                            border.width: 2
                            Repeater {
                                model: boardBridge.components
                                delegate: Rectangle {
                                    required property var modelData
                                    readonly property bool selectedComponent: workspace.selected.reference === modelData.reference
                                    readonly property bool netMember: workspace.containsNet(modelData, workspace.activeNet)
                                    readonly property bool searchMatch: searchField.text.trim() === "" || workspace.componentMatches(modelData, searchField.text)
                                    x: modelData.x * workspace.zoom; y: modelData.y * workspace.zoom
                                    width: modelData.width * workspace.zoom; height: modelData.height * workspace.zoom
                                    radius: Math.min(6, 4 * workspace.zoom)
                                    opacity: searchMatch ? 1 : 0.22
                                    color: selectedComponent ? "#154b7a" : netMember ? "#125c4f" : "#101d29"
                                    border.width: selectedComponent || netMember ? 2 : 1
                                    border.color: selectedComponent ? "#63afff" : netMember ? "#4de4bd" : "#bd985b"
                                    Column {
                                        anchors.centerIn: parent
                                        width: parent.width - 4
                                        spacing: 3
                                        Label { width: parent.width; text: modelData.reference; textFormat: Text.PlainText; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideRight; color: "#edf5ff"; font.pixelSize: Math.max(8, Math.min(17, 18 * workspace.zoom)); font.bold: true }
                                        Label { width: parent.width; visible: parent.parent.height > 48 && workspace.zoom > 0.45; text: modelData.name; textFormat: Text.PlainText; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideRight; color: "#9ab8cd"; font.pixelSize: 10 }
                                    }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: workspace.selected = modelData }
                                }
                            }
                        }
                    }
                    Column {
                        visible: !boardBridge.loaded
                        anchors.centerIn: parent
                        width: Math.max(100, parent.width - 48)
                        spacing: 12
                        Label { width: parent.width; text: "BOARD WORKSPACE"; color: "#4284ac"; font.pixelSize: 23; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                        Label { width: parent.width; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter; color: "#90abc0"; text: workspace.tr("افتح ملفًا مرخصًا أو جرّب لوحة التدريب. اسحب لتحريك اللوحة واستخدم أزرار التكبير للفحص.", "Open licensed data or load the synthetic training board. Drag to pan; use the zoom controls to inspect components.") }
                    }
                }
                Label { Layout.fillWidth: true; text: boardBridge.loaded ? boardBridge.title + " · " + boardBridge.author + " · " + boardBridge.license : workspace.tr("لم يُفتح ملف بعد", "No board loaded"); textFormat: Text.PlainText; elide: Text.ElideRight; color: "#87a5ba"; font.pixelSize: 11 }
                ListView {
                    Layout.fillWidth: true; Layout.preferredHeight: searchField.text.trim() ? 92 : 0
                    visible: searchField.text.trim() !== ""
                    model: boardBridge.search(searchField.text); spacing: 3; clip: true
                    delegate: ItemDelegate {
                        required property var modelData
                        width: ListView.view.width; height: 29
                        text: modelData.reference + " · " + modelData.name
                        contentItem: Text { text: parent.text; textFormat: Text.PlainText; color: "#bbd7ea"; elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter }
                        onClicked: { workspace.selected = modelData; workspace.focusComponent(modelData) }
                    }
                }
            }
            Rectangle {
                Layout.preferredWidth: workspace.width < 850 ? 250 : 285
                Layout.fillHeight: true
                color: "#091b29"; radius: 8; border.color: "#203d52"
                ScrollView {
                    objectName: "boardInspector"
                    anchors.fill: parent; anchors.margins: 12; clip: true
                    contentWidth: availableWidth
                    ColumnLayout {
                        width: parent.width
                        spacing: 10
                        Label { text: workspace.tr("فحص المكونات", "Component inspector"); color: "#eef5ff"; font.bold: true; font.pixelSize: 16 }
                        Label { Layout.fillWidth: true; text: workspace.selected.reference || workspace.tr("اختر مكونًا من اللوحة", "Select a board component"); textFormat: Text.PlainText; color: "#6eb6ff"; font.pixelSize: 20; font.bold: true; wrapMode: Text.WordWrap }
                        Label { Layout.fillWidth: true; text: workspace.selected.name || ""; textFormat: Text.PlainText; color: "#a5bfd3"; wrapMode: Text.WordWrap }
                        Flow {
                            Layout.fillWidth: true; spacing: 5
                            Repeater {
                                model: workspace.selected.nets || []
                                delegate: ActionButton {
                                    required property string modelData
                                    text: modelData; primary: workspace.activeNet === modelData
                                    onClicked: workspace.activeNet = workspace.activeNet === modelData ? "" : modelData
                                }
                            }
                        }
                        Label { Layout.fillWidth: true; text: workspace.tr("إضاءة الشبكة تُظهر عضوية المكونات فقط؛ لا تتوفر مسارات الأسلاك.", "Net highlighting indicates component membership; trace geometry is not available."); color: "#7696ad"; font.pixelSize: 11; wrapMode: Text.WordWrap }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#233b4c" }
                        Label { text: workspace.tr("قياس يدوي · فولت", "Manual measurement · volts"); color: "#d9e8f2"; font.bold: true }
                        ComboBox { id: netBox; Layout.fillWidth: true; enabled: boardBridge.loaded; model: boardBridge.nets; textRole: "name"; valueRole: "name"; onActivated: workspace.activeNet = currentText }
                        TextField { id: voltageField; objectName: "boardVoltage"; Layout.fillWidth: true; enabled: boardBridge.loaded; placeholderText: "3.30 V"; inputMethodHints: Qt.ImhFormattedNumbersOnly; validator: DoubleValidator { bottom: -1000; top: 1000; locale: "C"; notation: DoubleValidator.StandardNotation } }
                        TextField { id: measurementNote; Layout.fillWidth: true; enabled: boardBridge.loaded; maximumLength: 2000; placeholderText: workspace.tr("ظروف القياس أو ملاحظة", "Measurement conditions or note") }
                        ActionButton { Layout.fillWidth: true; objectName: "recordMeasurement"; text: workspace.tr("تسجيل القياس", "Record measurement"); enabled: boardBridge.loaded && voltageField.acceptableInput && voltageField.text.trim() !== "" && netBox.currentIndex >= 0; onClicked: if (boardBridge.addMeasurement(netBox.currentText, Number(voltageField.text), measurementNote.text)) { voltageField.clear(); measurementNote.clear(); workspace.status = workspace.tr("سُجل القياس اليدوي", "Manual measurement recorded") } }
                        Repeater {
                            model: boardBridge.measurements.slice(-5).reverse()
                            delegate: Label {
                                required property var modelData
                                Layout.fillWidth: true; wrapMode: Text.WordWrap; textFormat: Text.PlainText; font.pixelSize: 11
                                text: modelData.net + ": " + Number(modelData.value).toFixed(3) + " V · " + workspace.comparisonText(modelData.comparison)
                                color: modelData.comparison === "outside_reference" ? "#edb266" : "#8bd6c0"
                            }
                        }
                        Label { Layout.fillWidth: true; text: workspace.tr("المقارنة حسابية مع مرجع الملف وليست حكمًا بسلامة القطعة.", "Comparison is arithmetic against the file reference; it does not establish component health."); wrapMode: Text.WordWrap; color: "#7797ae"; font.pixelSize: 11 }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#233b4c" }
                        Label { text: workspace.tr("تعليق على المكون", "Component annotation"); color: "#d9e8f2"; font.bold: true }
                        TextArea { id: annotationText; Layout.fillWidth: true; Layout.preferredHeight: 80; enabled: !!workspace.selected.reference; wrapMode: TextEdit.Wrap; placeholderText: workspace.tr("ملاحظات الفني…", "Technician notes…"); color: "#d8e8f7"; background: Rectangle { color: "#071623"; border.color: "#24445a"; radius: 5 } onTextChanged: if (text.length > 4000) text = text.slice(0, 4000) }
                        ActionButton { Layout.fillWidth: true; text: workspace.tr("إضافة تعليق", "Add annotation"); enabled: !!workspace.selected.reference && annotationText.text.trim().length > 0; onClicked: if (boardBridge.addAnnotation(workspace.selected.reference, annotationText.text, workspace.selected.x + workspace.selected.width / 2, workspace.selected.y + workspace.selected.height / 2)) { annotationText.clear(); workspace.status = workspace.tr("أُضيف التعليق", "Annotation added") } }
                        Repeater {
                            model: boardBridge.annotations
                            delegate: Label { required property var modelData; Layout.fillWidth: true; visible: modelData.reference === workspace.selected.reference; text: "• " + modelData.text; textFormat: Text.PlainText; color: "#9fbbcd"; wrapMode: Text.WordWrap; font.pixelSize: 12 }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#233b4c" }
                        Label { text: workspace.tr("صور المكون", "Component photographs"); color: "#d9e8f2"; font.bold: true }
                        Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: "#7797ae"; font.pixelSize: 11; text: workspace.tr("حتى 4 صور لكل جلسة. PNG/JPEG محلية حتى 4 MiB و4096×4096؛ الصورة المحفوظة حتى 2 MiB. تبقى الصور داخل ملف الجلسة.", "Up to 4 photos per session. Local PNG/JPEG up to 4 MiB and 4096×4096; stored PNG up to 2 MiB. Photos remain inside the session file.") }
                        ActionButton { objectName: "attachBoardPhoto"; Layout.fillWidth: true; text: workspace.tr("إرفاق صورة", "Attach photograph"); enabled: !!workspace.selected.reference && boardBridge.photos.length < 4; onClicked: { workspace.photoReference = workspace.selected.reference; photoPicker.open() } }
                        Repeater {
                            model: boardBridge.photos
                            delegate: ColumnLayout {
                                required property var modelData
                                required property int index
                                visible: modelData.reference === workspace.selected.reference
                                Layout.fillWidth: true
                                Image { Layout.fillWidth: true; Layout.preferredHeight: 130; fillMode: Image.PreserveAspectFit; source: modelData.image; sourceSize.width: 520; sourceSize.height: 260; cache: false; MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { workspace.photoPreview = modelData.image; photoDialog.open() } } }
                                Label { Layout.fillWidth: true; text: modelData.name; textFormat: Text.PlainText; color: "#9fbbcd"; elide: Text.ElideRight; font.pixelSize: 11 }
                                ActionButton { text: workspace.tr("حذف المرفق", "Remove attachment"); implicitHeight: 28; onClicked: boardBridge.removePhoto(index) }
                            }
                        }
                        Label { Layout.fillWidth: true; text: boardBridge.description; textFormat: Text.PlainText; wrapMode: Text.WordWrap; color: "#708ca2"; font.pixelSize: 10 }
                    }
                }
            }
        }
        Label { Layout.fillWidth: true; visible: boardBridge.error !== "" || workspace.status !== ""; text: boardBridge.error || workspace.status; textFormat: Text.PlainText; wrapMode: Text.WordWrap; color: boardBridge.error ? "#ffae8c" : "#6fd9b5"; font.pixelSize: 12 }
    }
    function comparisonText(value) {
        if (value === "within_reference") return tr("ضمن المرجع", "within reference")
        if (value === "outside_reference") return tr("خارج المرجع", "outside reference")
        return tr("بلا مرجع", "no reference")
    }
    function componentMatches(component, query) {
        var needle = query.trim().toLowerCase()
        if (component.reference.toLowerCase().indexOf(needle) >= 0 || component.name.toLowerCase().indexOf(needle) >= 0) return true
        for (var i = 0; i < component.nets.length; ++i)
            if (component.nets[i].toLowerCase().indexOf(needle) >= 0) return true
        return false
    }
    function focusComponent(component) {
        viewport.contentX = Math.max(0, Math.min(viewport.contentWidth - viewport.width, circuit.x + (component.x + component.width / 2) * zoom - viewport.width / 2))
        viewport.contentY = Math.max(0, Math.min(viewport.contentHeight - viewport.height, circuit.y + (component.y + component.height / 2) * zoom - viewport.height / 2))
    }
}
