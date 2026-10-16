import json
import sys
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QApplication, QComboBox, QFormLayout, QHBoxLayout, QLabel,
    QLineEdit, QListWidget, QMainWindow, QMessageBox, QPushButton, QStackedWidget,
    QTextEdit, QVBoxLayout, QWidget)
from .core import CATALOG, RULES, discover, readiness, save_case
from .oem_services import OEM, COMMON, capability_status
from .device_actions import ALLOWED, connected_devices, execute

STYLE = """
QWidget {background:#0b1220;color:#e7edf8;font-size:14px;font-family:'Segoe UI';}
QPushButton {background:#18283e;border:1px solid #30475d;border-radius:10px;padding:12px;text-align:left;}
QPushButton:hover {background:#24435b;}
QLineEdit,QComboBox,QTextEdit,QListWidget {background:#111e30;border:1px solid #334b60;border-radius:8px;padding:8px;}
QLabel#heading {font-size:23px;font-weight:700;color:#64e2bb;}
"""
class Window(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("NEXVARY MR | Mobile Repair Workspace")
        self.resize(1100, 720)
        root = QWidget()
        self.setCentralWidget(root)
        layout = QHBoxLayout(root)
        nav = QVBoxLayout()
        self.stack = QStackedWidget()
        layout.addLayout(nav, 1)
        layout.addWidget(self.stack, 4)
        sections = [
            ("Dashboard / الرئيسية", self.dashboard),
            ("Readiness / المتطلبات", self.ready_page),
            ("Devices / الأجهزة", self.devices_page),
            ("OEM Service / صيانة الهواتف", self.oem_page),
            ("Diagnosis / التشخيص", self.diagnosis_page),
            ("Repair case / أمر صيانة", self.case_page),
        ]
        for index, (name, factory) in enumerate(sections):
            self.stack.addWidget(factory())
            button = QPushButton(name)
            button.clicked.connect(lambda checked=False, i=index: self.stack.setCurrentIndex(i))
            nav.addWidget(button)
        nav.addStretch()

    def page(self, heading):
        page = QWidget()
        layout = QVBoxLayout(page)
        label = QLabel(heading)
        label.setObjectName("heading")
        layout.addWidget(label)
        return page, layout

    def dashboard(self):
        page, layout = self.page("NEXVARY MR — Mobile Repair")
        layout.addWidget(QLabel("Offline-first • Windows 10/11 • Safe read-only discovery"))
        layout.addWidget(QLabel("Start with readiness, connect a phone, then document diagnostics."))
        layout.addStretch()
        return page

    def ready_page(self):
        page, layout = self.page("Computer readiness / جاهزية الكمبيوتر")
        output = QTextEdit()
        output.setReadOnly(True)
        def refresh():
            output.setPlainText(json.dumps(readiness(), ensure_ascii=False, indent=2))
        button = QPushButton("Scan computer / فحص الكمبيوتر")
        button.clicked.connect(refresh)
        layout.addWidget(button)
        layout.addWidget(output)
        refresh()
        return page

    def devices_page(self):
        page, layout = self.page("Device catalog & USB discovery / الأجهزة")
        search = QLineEdit()
        search.setPlaceholderText("Search brand or model / ابحث عن شركة أو موديل")
        models = QListWidget()
        def filter_models(query=""):
            models.clear()
            for brand, entries in CATALOG.items():
                for model in entries:
                    if query.casefold() in (brand + " " + model).casefold():
                        models.addItem(f"{brand}  ›  {model}")
        search.textChanged.connect(filter_models)
        filter_models()
        output = QTextEdit()
        output.setReadOnly(True)
        scan = QPushButton("Discover ADB / Fastboot (read-only)")
        scan.clicked.connect(lambda: output.setPlainText(json.dumps(discover(), ensure_ascii=False, indent=2)))
        layout.addWidget(search)
        layout.addWidget(models)
        layout.addWidget(scan)
        layout.addWidget(output)
        return page

    def oem_page(self):
        page, layout = self.page("OEM Service / مركز صيانة الهواتف")
        brand = QComboBox()
        brand.addItems(OEM.keys())
        transport = QComboBox()
        transport.addItems(["adb", "fastboot"])
        serial = QComboBox()
        serial.setEditable(False)
        action = QComboBox()
        result = QTextEdit()
        result.setReadOnly(True)
        def refresh_actions():
            action.clear()
            for service in COMMON:
                status = capability_status(brand.currentText(), service.key)
                if service.key in ALLOWED:
                    action.addItem(service.title, service.key)
            result.setPlainText("Only tested transport-level commands are offered. OEM-specific operations require model adapters.")
        def refresh_devices():
            serial.clear()
            try:
                serial.addItems(connected_devices(transport.currentText()))
                if serial.count() == 0:
                    result.setPlainText("No authorized device detected for selected transport.")
            except (RuntimeError, OSError, ValueError) as exc:
                result.setPlainText(str(exc))
        def run():
            selected = action.currentData()
            device = serial.currentText()
            if not selected or not device:
                QMessageBox.warning(self, "Device required", "Connect and select a device first.")
                return
            confirm = QMessageBox.question(self, "Confirm device operation",
                f"Execute {selected} on device {device}?",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if confirm != QMessageBox.StandardButton.Yes:
                return
            try:
                result.setPlainText(execute(selected, device, confirmed=True))
            except (RuntimeError, ValueError, PermissionError, OSError) as exc:
                QMessageBox.warning(self, "Operation failed", str(exc))
                result.setPlainText(str(exc))
        brand.currentTextChanged.connect(lambda: filter_transport_actions())
        transport.currentTextChanged.connect(refresh_devices)
        def filter_transport_actions():
            action.clear()
            for service in COMMON:
                if service.key in ALLOWED and ALLOWED[service.key][0] == transport.currentText():
                    action.addItem(service.title, service.key)
        transport.currentTextChanged.connect(filter_transport_actions)
        form = QFormLayout()
        form.addRow("Manufacturer / الشركة", brand)
        form.addRow("Connection / الاتصال", transport)
        form.addRow("Device serial / الجهاز", serial)
        form.addRow("Available action / الوظيفة", action)
        layout.addLayout(form)
        refresh = QPushButton("Refresh connected devices / تحديث الأجهزة")
        refresh.clicked.connect(refresh_devices)
        layout.addWidget(refresh)
        button = QPushButton("Execute selected action / تنفيذ الأمر")
        button.clicked.connect(run)
        layout.addWidget(button)
        layout.addWidget(result)
        refresh_actions()
        filter_transport_actions()
        return page

    def diagnosis_page(self):
        page, layout = self.page("Guided diagnosis / التشخيص")
        choice = QComboBox()
        choice.addItems(RULES)
        result = QTextEdit()
        result.setReadOnly(True)
        def update():
            steps, warning = RULES[choice.currentText()]
            result.setPlainText(f"Next check:\n{steps}\n\nSafety:\n{warning}")
        choice.currentTextChanged.connect(update)
        layout.addWidget(choice)
        layout.addWidget(result)
        update()
        return page

    def case_page(self):
        page, layout = self.page("New repair case / أمر صيانة")
        brand, model, symptom = QLineEdit(), QLineEdit(), QLineEdit()
        notes = QTextEdit()
        form = QFormLayout()
        for name, widget in (("Brand / الشركة", brand), ("Model / الموديل", model),
                             ("Symptom / العطل", symptom), ("Notes / ملاحظات", notes)):
            form.addRow(name, widget)
        layout.addLayout(form)
        def save():
            try:
                number = save_case(brand.text(), model.text(), symptom.text(), notes.toPlainText())
                QMessageBox.information(self, "Saved", f"Repair case #{number} saved locally.")
            except (ValueError, OSError) as exc:
                QMessageBox.warning(self, "Cannot save", str(exc))
        button = QPushButton("Save case / حفظ أمر الصيانة")
        button.clicked.connect(save)
        layout.addWidget(button)
        layout.addStretch()
        return page

def main():
    app = QApplication(sys.argv)
    app.setStyleSheet(STYLE)
    window = Window()
    window.show()
    sys.exit(app.exec())
