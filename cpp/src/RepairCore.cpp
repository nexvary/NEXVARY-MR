#include "RepairCore.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
namespace mr {
namespace {
struct GuidedRule { std::string_view key; Diagnosis diagnosis; };
// Original, non-destructive first-line guidance, reviewed against official documentation:
// https://support.google.com/android/answer/7664448 (restarts, apps, storage)
// https://support.google.com/pixelphone/answer/7167687 (charge/power)
// https://support.google.com/pixelphone/answer/3333708 (thermal safety)
// https://support.google.com/pixelphone/answer/6010316 (touch)
// https://support.google.com/pixelphone/answer/6306624 (camera)
// https://support.apple.com/en-us/108805 (charging)
// https://support.apple.com/en-us/102643 (liquid detection)
// https://support.apple.com/en-us/108643 (computer recognition/trust)
// https://developer.android.com/tools/adb (device states and authorization)
// These are evidence-collection workflows, never confirmed hardware diagnoses.
const std::vector<GuidedRule> &guidedRules() {
  static const std::vector<GuidedRule> rules{
    {"Bootloop", {"1. Record the exact model, last successful boot and recent changes.\n2. Check available charge and capture the visible boot message.\n3. If Android starts, preserve authorized logs and compare normal/safe-mode behavior using OEM instructions.\n4. Record documented recovery availability for this platform; Fastboot is not universal.", "Preserve user data. No factory reset, unlock, flash or erase during this workflow; recovery changes require separate consent and a verified backup when possible."}},
    {"Not charging", {"1. Stop if there is liquid, swelling or abnormal heat.\n2. Test a known-good compatible charger, data cable and outlet.\n3. Inspect the connector externally and record charging indicators.\n4. Follow the exact model's charging guidance before considering component-level work.", "Do not insert metal into a connector or probe a powered board during initial checks. A charging symptom alone does not identify a failed IC."}},
    {"No network", {"1. Record whether SIM/eSIM is recognized.\n2. Check airplane mode, coverage and carrier outage/account status.\n3. With permission, compare a known-working SIM or location.\n4. Record available baseband and registration status, then consult the carrier/OEM.", "Never modify IMEI, device identity or carrier protection. Redact subscriber identifiers before sharing logs."}},
    {"USB not detected", {"1. Use a known-good data cable and another USB port.\n2. Check Windows Device Manager for the actual USB state.\n3. For Android, distinguish USB enumeration from ADB authorization; for iPhone/iPad, check Apple Devices/Finder and the owner's trust prompt.\n4. Record the official driver/provider and exact error.", "Install drivers only from Windows/OEM sources. USB visibility does not grant access to user data or imply ADB support."}},
    {"No power", {"1. Inspect externally for damage, liquid, swelling or heat.\n2. If safe, try a compatible known-good charger and record any indicator.\n3. Distinguish no power from a dark display by documented indicators.\n4. Use only the OEM's model-specific restart procedure and record the result.", "Stop on physical battery damage or abnormal heat. Do not inject voltage, short test points or assume a universal restart sequence."}},
    {"Overheating", {"1. Disconnect charging and stop heavy tasks.\n2. Let the device cool naturally in a ventilated area away from sunlight.\n3. Record when the heat occurs and any system warning.\n4. If recurring during light use, escalate using the OEM's service guidance.", "Do not bypass thermal protection or use rapid cooling, liquids or external heat. Stop using a swollen or damaged battery."}},
    {"Battery drains quickly", {"1. Record the time interval, charge level and usage conditions.\n2. Review available battery-usage statistics and signal conditions.\n3. Compare background app activity with the owner's normal workload.\n4. Record manufacturer battery-health information only where the device exposes it.", "A single percentage reading does not establish battery failure. Do not run deep-discharge cycles or install unknown battery tools."}},
    {"Touchscreen not responding", {"1. Check for moisture, damage and obstructing accessories.\n2. With permission, test without a removable case/protector.\n3. Determine whether the issue affects one app or the whole screen.\n4. Use model-specific built-in touch diagnostics where officially available.", "Do not bypass the screen lock. Record existing cracks and obtain permission before removing a protector or opening the device."}},
    {"No display", {"1. Record whether the phone vibrates, rings or enumerates over USB.\n2. Check available brightness/accessibility settings only if accessible.\n3. Try the OEM's documented restart procedure.\n4. Separate a no-image observation from a no-power conclusion and escalate with evidence.", "No visible image does not prove a failed display or mainboard. Never press on a damaged/swollen device or probe powered display connectors."}},
    {"Liquid exposure", {"1. Disconnect external power and accessories.\n2. Record the exposure history and any liquid-detection alert.\n3. Stop charging and follow the manufacturer's drying/service instructions.\n4. Escalate suspected internal exposure for qualified inspection.", "Do not apply heat, compressed air or rice, insert objects into the port, or override a liquid warning. A dry exterior does not prove the internal board is safe."}},
    {"Storage full", {"1. Record total/free storage using the device's settings.\n2. Identify large categories with the owner; distinguish cache from personal files.\n3. Verify a backup before proposing any cleanup.\n4. Recheck free space and the original symptom after an owner-approved change.", "Do not delete files, clear application data or format storage without explicit consent. Low storage does not by itself prove storage hardware failure."}},
    {"App crashes", {"1. Record the app/version and reproducible action.\n2. Preserve the crash timestamp and authorized log evidence.\n3. Compare another app and check available storage.\n4. Review official app/system update guidance and escalate persistent reproducible crashes.", "Crash text identifies a symptom, not necessarily the root cause. Do not clear app data or reinstall without discussing local data and backups."}},
    {"Camera not working", {"1. Record the error and affected camera/lens.\n2. Check the camera permission and system privacy toggle where present.\n3. Compare the OEM camera app and another authorized app.\n4. Check obstruction and whether the problem is app-specific before service.", "Do not access or share the owner's photos. A camera-app error does not establish a failed sensor; disassembly requires separate authorization."}},
    {"Microphone not working", {"1. Check mute, microphone permission and privacy controls.\n2. Inspect externally for an obstructing case.\n3. With consent, compare a short local recording and a call.\n4. Check the active Bluetooth/wired audio route and document which path fails.", "Obtain consent before recording; avoid private conversations and delete test media as agreed. Do not insert objects into microphone openings."}},
    {"Speaker not working", {"1. Check volume, mute and do-not-disturb settings.\n2. Check the selected Bluetooth/wired audio output.\n3. Compare a known test sound with call audio at a safe level.\n4. Document whether one output or all sound paths are affected.", "Use a comfortable test volume. Do not insert tools into speaker openings or infer an audio-IC failure from one silent app."}},
    {"Wi-Fi not working", {"1. Record whether Wi-Fi is absent, cannot associate or has no internet.\n2. Check airplane mode and compare another authorized network/device.\n3. Record the router/device error and time.\n4. Check official network/OEM guidance before changing saved settings.", "Connect only to networks you are permitted to test. Network resets erase settings and require separate consent; do not expose passwords in reports."}},
    {"Bluetooth not working", {"1. Check Bluetooth state and accessory power/pairing mode.\n2. Confirm compatibility in the accessory's official guide.\n3. Compare a known-working authorized accessory.\n4. Record whether discovery, pairing or audio routing fails.", "Do not pair with unknown devices. Forgetting pairings or resetting network settings requires the owner's agreement."}},
    {"SIM not detected", {"1. Record the exact SIM/eSIM message.\n2. Check that the line/profile is enabled and provisioned with the carrier.\n3. For a removable SIM, follow the OEM tray procedure and compare a known-working authorized SIM.\n4. Document whether the issue follows the card or device.", "Do not delete eSIM profiles or bypass SIM/carrier locks. Never modify identity data; protect subscriber information."}},
    {"ADB unauthorized", {"1. Confirm this is an Android device with owner-enabled USB debugging.\n2. Unlock the device normally and inspect its RSA authorization prompt.\n3. The owner should approve only the intended computer.\n4. Reconnect and record the resulting ADB state.", "Do not bypass authorization or the screen lock. ADB authorization is platform-specific and is not available on iPhone/iPad."}},
    {"ADB offline", {"1. Record the serial and ADB state.\n2. Check the data cable, port and device responsiveness.\n3. Reconnect the intended device and confirm its existing authorization.\n4. Record official platform-tools version and whether the state changes.", "Offline does not establish firmware corruption. Do not erase data, kill unrelated sessions or force a recovery transition during a connection check."}},
    {"Fastboot not detected", {"1. Confirm that the exact Android model supports Fastboot and is already in its documented mode.\n2. Check USB enumeration, cable and the official driver.\n3. Record the mode displayed on the phone and the tool result.\n4. Consult OEM documentation if the platform uses a different service mode.", "Fastboot absence is not proof of a fault. Do not unlock, flash, erase or switch modes automatically; Apple devices do not use Android Fastboot."}},
    {"Slow performance", {"1. Record the operation, duration and whether it affects one app or the system.\n2. Check storage, recent app changes and thermal warnings.\n3. Compare after a normal owner-approved restart.\n4. Review official app/system guidance and record reproducible evidence.", "Avoid untrusted cleaner apps, deleting user data or disabling security services. Slow behavior alone does not identify defective memory or storage."}}
  };
  return rules;
}
std::string normalizedKey(std::string_view text) {
  while(!text.empty()&&std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
  while(!text.empty()&&std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
  std::string result(text);
  std::transform(result.begin(),result.end(),result.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
  return result;
}
}
std::vector<DeviceFamily> catalog() {
  // Family directory, not a list of certified repair, firmware or boardview support.
  // Historical families remain useful for repairs; availability varies by region.
  // Additional family names verified against official ASUS, Motorola, Sony, HMD,
  // OnePlus, Nothing, TCL and ZTE support/product indexes on 2026-10-16.
  // https://www.asus.com/mobile-handhelds/phones/all-series/
  // https://en-us.support.motorola.com/app/home/
  // https://www.sony.com/electronics/support/mobile-phones-tablets-mobile-phones
  // https://www.hmd.com/en_gb/smartphones
  // https://www.oneplus.com/global/nord-series
  // https://in.nothing.tech/collections/phones
  // https://www.tcl.com/global/en/mobile
  // https://support.ztedevices.com/en-gl/select-product/
  return {
    {"Samsung", {"Galaxy A", "Galaxy S", "Galaxy M", "Galaxy Z", "Galaxy Note", "Galaxy J", "Galaxy F"}},
    {"Xiaomi / Redmi / POCO", {"Xiaomi", "Xiaomi T", "Redmi", "Redmi Note", "POCO C", "POCO M", "POCO X", "POCO F"}},
    {"OPPO", {"A", "Reno", "Find"}},
    {"realme", {"C", "Number", "GT"}},
    {"vivo", {"Y", "V", "X"}},
    {"Infinix", {"Hot", "Note", "Smart"}},
    {"TECNO", {"Spark", "Camon", "Pova"}},
    {"itel", {"A", "S"}},
    {"Huawei", {"Nova", "P", "Mate"}},
    {"HONOR", {"X", "Magic"}},
    {"Apple", {"iPhone", "iPad"}},
    {"Google", {"Pixel"}},
    {"Motorola", {"moto g", "moto e", "edge", "razr", "ThinkPhone"}},
    {"Sony", {"Xperia"}},
    {"ASUS", {"Zenfone", "ROG Phone"}},
    {"HMD", {"Pulse", "Skyline", "Fusion", "XR21"}},
    {"OnePlus", {"OnePlus", "Nord", "Nord CE"}},
    {"Nothing / CMF", {"Nothing Phone", "CMF Phone"}},
    {"TCL", {"TCL 10", "TCL 20", "TCL 30", "TCL 40", "TCL 50", "NXTPAPER"}},
    {"ZTE", {"Axon", "Blade"}}
  };
}
std::vector<std::string> supportedSymptoms() {
  std::vector<std::string> result;result.reserve(guidedRules().size());
  for(const auto &rule:guidedRules()) result.emplace_back(rule.key);
  return result;
}
Diagnosis diagnose(std::string_view symptom) {
  if(symptom.size()<=2000) {
    auto key=normalizedKey(symptom);
    if(key=="water damage") key="liquid exposure";
    if(key=="no sound") key="speaker not working";
    for(const auto &rule:guidedRules()) if(normalizedKey(rule.key)==key) return rule.diagnosis;
  }
  return {"1. Record the exact model and observed symptom.\n2. Check external damage, liquid and heat before connecting power.\n3. Collect authorized, non-destructive evidence and record what was actually tested.\n4. Escalate to model-specific official guidance when evidence is insufficient.", "No confirmed diagnosis is available from this description. Preserve privacy and user data; do not unlock, erase or modify the device during initial checks."};
}
bool validCase(std::string_view brand, std::string_view model, std::string_view symptom) {
  const auto hasText=[](std::string_view s,std::size_t maximum,bool multiline) {
    if(s.empty()||s.size()>maximum) return false;
    bool text=false;
    for(const auto value:s) {
      const auto c=static_cast<unsigned char>(value);
      if(c==0||c==127||(c<32&&(!multiline||(c!='\n'&&c!='\r'&&c!='\t')))) return false;
      text=text||!std::isspace(c);
    }
    return text;
  };
  // Free text deliberately remains allowed: the family directory is not exhaustive.
  return hasText(brand,240,false) && hasText(model,240,false) && hasText(symptom,2000,true);
}
std::vector<LogFinding> analyzeLog(std::string_view log) {
  if(log.size()>512*1024) throw std::invalid_argument("Log exceeds 512 KiB");
  std::vector<LogFinding> rules={{"runtime_crash",0,{}},{"watchdog",0,{}},{"memory_pressure",0,{}},{"usb_transport",0,{}},{"permission_denial",0,{}}};
  while(!log.empty()) {
    const auto end=log.find('\n');const auto line=log.substr(0,end);
    std::string lower(line);std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    const bool matches[]={lower.find("fatal exception")!=std::string::npos||lower.find("fatal signal")!=std::string::npos,
      lower.find("watchdog")!=std::string::npos,
      lower.find("outofmemoryerror")!=std::string::npos||lower.find("lowmemorykiller")!=std::string::npos,
      lower.find("usb disconnect")!=std::string::npos||lower.find("device offline")!=std::string::npos||lower.find("device unauthorized")!=std::string::npos,
      lower.find("permission denial")!=std::string::npos||lower.find("avc: denied")!=std::string::npos};
    for(std::size_t i=0;i<rules.size();++i) if(matches[i]) {
      ++rules[i].count;if(rules[i].firstEvidence.empty()){rules[i].firstEvidence=std::string(line.substr(0,240));for(auto &c:rules[i].firstEvidence) if(static_cast<unsigned char>(c)<32)c=' ';}
    }
    if(end==std::string_view::npos)break;
    log.remove_prefix(end+1);
  }
  std::erase_if(rules,[](const LogFinding &finding){return finding.count==0;});return rules;
}

}
