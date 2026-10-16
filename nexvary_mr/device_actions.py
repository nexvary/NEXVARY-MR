"""Conservative ADB/Fastboot execution with exact serial targeting."""
import re
import subprocess
from .core import tool_path

SERIAL = re.compile(r"^[A-Za-z0-9._:-]{1,128}$")
ALLOWED = {
    "read_info": ("adb", ("shell", "getprop", "ro.product.model")),
    "read_android_version": ("adb", ("shell", "getprop", "ro.build.version.release")),
    "read_manufacturer": ("adb", ("shell", "getprop", "ro.product.manufacturer")),
    "read_security_patch": ("adb", ("shell", "getprop", "ro.build.version.security_patch")),
    "read_sdk": ("adb", ("shell", "getprop", "ro.build.version.sdk")),
    "read_bootloader": ("adb", ("shell", "getprop", "ro.bootloader")),
    "reboot_normal": ("adb", ("reboot",)),
    "reboot_recovery": ("adb", ("reboot", "recovery")),
    "reboot_bootloader": ("adb", ("reboot", "bootloader")),
    "fastboot_check": ("fastboot", ("getvar", "product")),
    "fastboot_reboot": ("fastboot", ("reboot",)),
    "fastboot_bootloader": ("fastboot", ("reboot", "bootloader")),
}

def connected_devices(transport):
    if transport not in ("adb", "fastboot"):
        raise ValueError("Invalid transport")
    executable = tool_path(transport)
    if not executable:
        raise RuntimeError(f"{transport} is not installed")
    command = [executable, "devices"]
    result = subprocess.run(command, capture_output=True, text=True, timeout=12, check=False)
    if result.returncode:
        raise RuntimeError(result.stderr.strip() or "Discovery failed")
    devices = []
    for line in result.stdout.splitlines():
        parts = line.split()
        if len(parts) >= 2 and SERIAL.fullmatch(parts[0]):
            if transport == "adb" and parts[1] != "device":
                continue
            if transport == "fastboot" and parts[1] != "fastboot":
                continue
            devices.append(parts[0])
    return devices

def execute(action, serial, confirmed=False):
    if action not in ALLOWED:
        raise ValueError("Operation not implemented or requires OEM-specific workflow")
    if not SERIAL.fullmatch(serial):
        raise ValueError("Invalid device serial")
    transport, args = ALLOWED[action]
    if serial not in connected_devices(transport):
        raise RuntimeError("Selected device is not connected and authorized")
    if action.startswith("reboot") or action.startswith("fastboot_reboot") or action == "fastboot_bootloader":
        if not confirmed:
            raise PermissionError("Explicit confirmation required")
    executable = tool_path(transport)
    result = subprocess.run([executable, "-s", serial, *args], capture_output=True,
                            text=True, timeout=20, check=False)
    if result.returncode:
        raise RuntimeError(result.stderr.strip() or f"{action} failed")
    return result.stdout.strip() or "Command completed"
