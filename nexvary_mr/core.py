import json
import os
import platform
import shutil
import sqlite3
import subprocess
from pathlib import Path

CATALOG = {
    "Samsung": ["Galaxy A", "Galaxy S", "Galaxy M", "Galaxy Z"],
    "Xiaomi / Redmi / POCO": ["Redmi Note 8", "Redmi Note", "POCO X", "POCO F"],
    "OPPO": ["OPPO A", "Reno", "Find"],
    "realme": ["C Series", "Number Series", "GT"],
    "vivo": ["Y Series", "V Series", "X Series"],
    "Infinix": ["Hot", "Note", "Smart"],
    "TECNO": ["Spark", "Camon", "Pova"],
    "itel": ["A Series", "S Series"],
    "Huawei": ["Nova", "P Series", "Mate"],
    "HONOR": ["X Series", "Magic"],
    "Apple": ["iPhone", "iPad"],
}
RULES = {
    "Bootloop": ("Check whether recovery/fastboot is available; capture logs before considering firmware restoration.", "Do not wipe data without explicit owner consent."),
    "Not charging": ("Verify charger, cable, USB port and battery response before board-level inspection.", "Do not probe powered circuits without appropriate training."),
    "No network": ("Check SIM detection, baseband information and network registration before hardware work.", "Never alter device identity or IMEI."),
    "USB not detected": ("Try a known data cable, another port, Device Manager and the appropriate OEM driver.", "Avoid installing untrusted driver bundles."),
}

def tool_path(name):
    return shutil.which(name)

def readiness():
    return {
        "os": platform.platform(),
        "windows": platform.system() == "Windows",
        "adb": tool_path("adb"),
        "fastboot": tool_path("fastboot"),
        "python": platform.python_version(),
    }

def run_readonly(args, timeout=8):
    try:
        p = subprocess.run(args, capture_output=True, text=True, timeout=timeout, check=False,
                           creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        return (p.stdout + "\n" + p.stderr).strip()[:10000]
    except (OSError, subprocess.TimeoutExpired) as exc:
        return f"Unavailable: {exc}"

def discover():
    data = {}
    for name, args in (("ADB", ["adb", "devices", "-l"]), ("Fastboot", ["fastboot", "devices"])):
        data[name] = run_readonly(args) if tool_path(args[0]) else "Tool not installed"
    return data

def db_path():
    base = Path(os.environ.get("LOCALAPPDATA", str(Path.home() / ".local" / "share")))
    folder = base / "NEXVARY-MR"
    folder.mkdir(parents=True, exist_ok=True)
    return folder / "repairs.db"

def init_db(path=None):
    connection = sqlite3.connect(str(path or db_path()))
    connection.execute("""CREATE TABLE IF NOT EXISTS cases (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        brand TEXT NOT NULL, model TEXT NOT NULL,
        symptom TEXT NOT NULL, notes TEXT NOT NULL DEFAULT ''
    )""")
    connection.commit()
    return connection

def save_case(brand, model, symptom, notes="", path=None):
    if not all([brand.strip(), model.strip(), symptom.strip()]):
        raise ValueError("Brand, model and symptom are required")
    with init_db(path) as connection:
        cursor = connection.execute(
            "INSERT INTO cases (brand, model, symptom, notes) VALUES (?, ?, ?, ?)",
            (brand.strip(), model.strip(), symptom.strip(), notes.strip()))
        number = cursor.lastrowid
    connection.close()
    return number
