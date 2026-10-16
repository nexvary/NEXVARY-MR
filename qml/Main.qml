import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
    id: window
    width: 1180
    height: 760
    minimumWidth: 860
    minimumHeight: 570
    visible: true
    title: "NEXVARY MR — Mobile Repair"
    color: "#0B101A"
    property int currentPage: 0
    property var names: ["الرئيسية", "جاهزية الكمبيوتر", "مكتبة الهواتف", "تشخيص الأعطال", "أوامر الصيانة"]
    property var details: [
        "مساحة عمل صيانة الهواتف — إصدار تأسيسي",
        "فحص الأدوات الرسمية المتاحة على الكمبيوتر",
        "شجرة الشركات والموديلات — البيانات الأساسية قيد التوسيع",
        "تشخيص إرشادي آمن — لا يتم تنفيذ أي عملية مدمرة",
        "سجل عمليات الصيانة — الربط بقاعدة البيانات قيد التطوير"
    ]
    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.preferredWidth: 275
            Layout.fillHeight: true
            color: "#111C2B"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 14
                Label { text: "NEXVARY MR"; color: "#62E6BA"; font.pixelSize: 27; font.bold: true }
                Label { text: "Mobile Repair Workspace"; color: "#9BAFC5"; font.pixelSize: 12 }
                Repeater {
                    model: window.names
                    delegate: Button {
                        required property int index
                        required property string modelData
                        Layout.fillWidth: true
                        text: modelData
                        onClicked: window.currentPage = index
                        background: Rectangle { radius: 10; color: window.currentPage === index ? "#215D59" : "#1A2B3D" }
                        contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter; rightPadding: 12; font.pixelSize: 16 }
                    }
                }
                Item { Layout.fillHeight: true }
                Label { text: "v0.2.0 • C++20 / Qt 6"; color: "#8098AF" }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0B101A"
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 34
                spacing: 16
                Label { text: window.names[window.currentPage]; color: "#62E6BA"; font.pixelSize: 30; font.bold: true; Layout.alignment: Qt.AlignRight }
                Rectangle { Layout.fillWidth: true; height: 1; color: "#30445A" }
                Label { text: window.details[window.currentPage]; color: "#E7EFFA"; font.pixelSize: 17; Layout.alignment: Qt.AlignRight; wrapMode: Text.WordWrap }
                ColumnLayout {
                    visible: window.currentPage === 0 || window.currentPage === 1
                    Layout.fillWidth: true
                    spacing: 12
                    Label { text: "الأدوات المتصلة"; color: "#62E6BA"; font.pixelSize: 20; Layout.alignment: Qt.AlignRight }
                    Repeater {
                        model: toolBridge.tools
                        delegate: Label {
                            required property var modelData
                            text: modelData.name + ": " + (modelData.available ? "متاح" : "غير مثبت")
                            color: modelData.available ? "#62E6BA" : "#F2BA77"
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignRight
                        Button { text: "تحديث الأدوات"; onClicked: toolBridge.refreshTools() }
                        Button { text: "فحص ADB"; enabled: !toolBridge.busy; onClicked: toolBridge.scanAdb() }
                        Button { text: "فحص Fastboot"; enabled: !toolBridge.busy; onClicked: toolBridge.scanFastboot() }
                        Button { text: "عرض الهاتف (scrcpy)"; enabled: !toolBridge.busy; onClicked: toolBridge.startMirror() }
                    }
                    Label { text: "نتيجة الفحص"; color: "#62E6BA"; font.pixelSize: 17; Layout.alignment: Qt.AlignRight }
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        TextArea {
                            text: toolBridge.status
                            readOnly: true
                            wrapMode: TextEdit.Wrap
                            color: "#E7EFFA"
                            background: Rectangle { color: "#172537"; radius: 8 }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                Label { text: "لا توجد عمليات تفليش أو مسح بيانات في هذا الإصدار"; color: "#9BAFC5"; Layout.alignment: Qt.AlignRight }
            }
        }
    }
}
