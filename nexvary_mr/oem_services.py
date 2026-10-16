"""Declarative OEM service catalog. No device-modifying commands are executed here."""
from dataclasses import dataclass
from enum import Enum

class Risk(str, Enum):
    READ_ONLY = "read_only"
    REVERSIBLE = "reversible"
    DATA_LOSS = "data_loss"
    RESTRICTED = "restricted"

@dataclass(frozen=True)
class Service:
    key: str
    title: str
    transport: str
    risk: Risk
    requires_confirmation: bool = False
    note: str = ""

COMMON = (
    Service("read_info", "Read device information", "adb", Risk.READ_ONLY),
    Service("read_android_version", "Android version", "adb", Risk.READ_ONLY),
    Service("read_manufacturer", "Manufacturer", "adb", Risk.READ_ONLY),
    Service("read_security_patch", "Security patch level", "adb", Risk.READ_ONLY),
    Service("read_sdk", "Android API level", "adb", Risk.READ_ONLY),
    Service("read_bootloader", "Bootloader version", "adb", Risk.READ_ONLY),
    Service("reboot_normal", "Reboot to system", "adb", Risk.REVERSIBLE),
    Service("reboot_recovery", "Reboot to recovery", "adb", Risk.REVERSIBLE),
    Service("reboot_bootloader", "Reboot to bootloader", "adb", Risk.REVERSIBLE),
    Service("install_apk", "Install APK from PC", "adb", Risk.REVERSIBLE, True, "Require APK provenance and on-device authorization."),
    Service("sideload", "Official recovery ADB sideload", "adb_sideload", Risk.DATA_LOSS, True, "Verify signature, exact model and recovery support."),
    Service("language", "Configure supported system languages", "android_settings", Risk.REVERSIBLE, False, "Arabic, English, French, Turkish; availability varies by ROM."),
    Service("keyboard", "Install trusted keyboard", "adb", Risk.REVERSIBLE, True),
    Service("factory_reset", "Factory reset via official recovery", "recovery", Risk.DATA_LOSS, True, "Irreversible user data loss; FRP may remain active."),
    Service("fastboot_check", "Check fastboot device", "fastboot", Risk.READ_ONLY),
    Service("fastboot_is_unlocked", "Read bootloader unlock state", "fastboot", Risk.READ_ONLY),
    Service("fastboot_current_slot", "Read active boot slot", "fastboot", Risk.READ_ONLY),
    Service("fastboot_serial", "Read fastboot serial", "fastboot", Risk.READ_ONLY),
    Service("fastboot_reboot", "Fastboot reboot", "fastboot", Risk.REVERSIBLE),
    Service("fastboot_bootloader", "Reboot bootloader", "fastboot", Risk.REVERSIBLE),
    Service("bootloader_unlock", "OEM-authorized bootloader unlock", "fastboot", Risk.DATA_LOSS, True, "Only OEM-supported devices; generally wipes data."),
    Service("bootloader_relock", "OEM-authorized bootloader relock", "fastboot", Risk.DATA_LOSS, True, "Only after verifying signed stock firmware."),
)
OEM = {
    "Samsung": ("download_mode", "odin_firmware", "pit_read", "modem_diagnostics", "carrier_app_review"),
    "LG": ("network_mode_read", "apn_settings", "wifi_diagnostics", "official_firmware"),
    "Huawei": ("official_recovery", "language_settings", "bootloader_status"),
    "Oppo": ("official_recovery", "language_settings", "chipset_detection"),
    "Vivo": ("official_recovery", "language_settings", "chipset_detection"),
    "ZTE": ("official_recovery", "language_settings", "chipset_detection"),
}
CHIPSETS = {"Qualcomm": ("edl_detect", "signed_firehose_check", "diag_read"),
            "MediaTek": ("preloader_detect", "signed_da_check"),
            "Unisoc/SPD": ("download_mode_detect",)}
RESTRICTED = ("frp_bypass", "account_removal", "lockscreen_bypass", "efs_wipe",
              "spc_zero", "sim_unlock_bypass", "security_disable", "rmm_bypass")
SUPPORTED_LANGUAGES = ("ar", "en", "fr", "tr")

def services_for(brand: str):
    if brand not in OEM:
        raise ValueError(f"Unknown OEM: {brand}")
    return COMMON

def capability_status(brand: str, capability: str) -> str:
    if capability in RESTRICTED:
        return "official_recovery_only"
    if brand in OEM and capability in OEM[brand]:
        return "requires_model_specific_adapter"
    if capability in CHIPSETS.get(brand, ()):
        return "requires_signed_vendor_protocol"
    if capability in {service.key for service in COMMON}:
        return "requires_connected_device_and_preflight"
    return "unsupported"
