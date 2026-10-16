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
