#include "RepairCore.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
namespace mr {
std::vector<DeviceFamily> catalog() {
  return {
    {"Samsung", {"Galaxy A", "Galaxy S", "Galaxy M", "Galaxy Z"}},
    {"Xiaomi / Redmi / POCO", {"Redmi Note 8", "Redmi Note", "POCO X", "POCO F"}},
    {"OPPO", {"A", "Reno", "Find"}},
    {"realme", {"C", "Number", "GT"}},
    {"vivo", {"Y", "V", "X"}},
    {"Infinix", {"Hot", "Note", "Smart"}},
    {"TECNO", {"Spark", "Camon", "Pova"}},
    {"itel", {"A", "S"}},
    {"Huawei", {"Nova", "P", "Mate"}},
    {"HONOR", {"X", "Magic"}},
    {"Apple", {"iPhone", "iPad"}}
  };
}
Diagnosis diagnose(std::string_view symptom) {
  if (symptom == "Bootloop") return {"Check recovery and fastboot availability; preserve logs before firmware recovery.", "Never erase user data without consent."};
  if (symptom == "Not charging") return {"Check cable, charger, connector and battery response first.", "Only qualified technicians should probe powered circuits."};
  if (symptom == "No network") return {"Verify SIM recognition, baseband and registration status.", "Never modify IMEI or device identity."};
  if (symptom == "USB not detected") return {"Check data cable, port, Device Manager and OEM driver.", "Avoid untrusted driver packages."};
  return {"Collect model, observed symptoms and non-destructive test results.", "Do not perform destructive operations during initial diagnosis."};
}
bool validCase(std::string_view brand, std::string_view model, std::string_view symptom) {
  const auto hasText=[](std::string_view s) {
    return std::any_of(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
  };
  return hasText(brand) && hasText(model) && hasText(symptom);
}
}
