#include "RepairCore.hpp"
#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition,const char *message) { if(!condition) throw std::runtime_error(message); }
bool contains(const std::string &value,const std::string &part) { return value.find(part)!=std::string::npos; }
void catalogCoverage() {
  const auto brands=mr::catalog();
  require(brands.size()>=20,"manufacturer directory coverage regressed");
  require(brands.front().brand=="Samsung","existing catalog ordering changed");
  std::set<std::string> names;
  std::size_t families=0;
  for(const auto &entry:brands) {
    require(!entry.brand.empty()&&names.insert(entry.brand).second,"empty/duplicate brand");
    require(!entry.families.empty(),"brand has no families");
    std::set<std::string> unique;
    for(const auto &family:entry.families) {
      require(!family.empty()&&unique.insert(family).second,"empty/duplicate family");
      ++families;
    }
  }
  for(const auto *name:{"Apple","Google","Motorola","Sony","ASUS","HMD","OnePlus","Nothing / CMF","TCL","ZTE"})
    require(names.contains(name),"expected manufacturer absent");
  std::cout<<"Catalog: "<<brands.size()<<" brand groups, "<<families<<" family entries (directory only)\n";
}
void guidedWorkflows() {
  const auto keys=mr::supportedSymptoms();
  require(keys.size()>=22,"guided symptom coverage regressed");
  require(keys[0]=="Bootloop"&&keys[1]=="Not charging"&&keys[2]=="No network"&&keys[3]=="USB not detected","existing symptom keys changed");
  std::set<std::string> uniqueKeys,uniqueGuidance;
  for(const auto &key:keys) {
    require(uniqueKeys.insert(key).second,"duplicate symptom key");
    const auto rule=mr::diagnose(key);
    require(contains(rule.nextCheck,"1.")&&contains(rule.nextCheck,"2.")&&contains(rule.nextCheck,"3.")&&contains(rule.nextCheck,"4."),"workflow lacks sequential evidence checks");
    require(!rule.safety.empty(),"workflow lacks safety guidance");
    require(uniqueGuidance.insert(rule.nextCheck).second,"different symptoms share generic guidance");
  }
  require(mr::diagnose("  bOoTlOoP\t").nextCheck==mr::diagnose("Bootloop").nextCheck,"case/whitespace normalization failed");
  require(mr::diagnose("Water damage").nextCheck==mr::diagnose("Liquid exposure").nextCheck,"liquid alias failed");
  require(mr::diagnose("No sound").nextCheck==mr::diagnose("Speaker not working").nextCheck,"audio alias failed");
  require(contains(mr::diagnose("Bootloop").nextCheck,"not universal"),"bootloop implies universal Fastboot support");
  require(contains(mr::diagnose("Not charging").nextCheck,"liquid, swelling or abnormal heat"),"charging lacks physical safety triage");
  require(contains(mr::diagnose("ADB unauthorized").safety,"Do not bypass"),"authorization safety missing");
  require(contains(mr::diagnose("Fastboot not detected").safety,"Apple devices"),"platform-specific Fastboot limitation missing");
  const auto unknown=mr::diagnose("Unclassified symptom");
  require(contains(unknown.safety,"No confirmed diagnosis"),"unknown symptom is presented as confirmed");
  require(mr::diagnose(std::string(2001,'x')).nextCheck==unknown.nextCheck,"oversized symptom should use safe fallback");
  std::cout<<"Guided workflows: "<<keys.size()<<" distinct symptoms, sequential read-only triage\n";
}
void caseInputs() {
  require(mr::validCase("Samsung","A52","Bootloop"),"valid input rejected");
  require(mr::validCase("Unlisted brand","Unlisted model","Intermittent symptom"),"directory incorrectly treated as compatibility whitelist");
  require(mr::validCase("Example","Model","First observation\nSecond observation"),"multiline symptom rejected");
  require(mr::validCase("شركة","جهاز","عطل الشحن"),"UTF-8 case text rejected");
  require(!mr::validCase(" \t\n","A52","Bootloop"),"blank brand accepted");
  require(!mr::validCase("Samsung","","Bootloop"),"empty model accepted");
  require(!mr::validCase("Samsung","A52"," \n\t"),"blank symptom accepted");
  require(!mr::validCase(std::string("Sam\0sung",8),"Model","USB"),"NUL-containing brand accepted");
  require(!mr::validCase("Brand","Model\nInjected","USB"),"control-containing model accepted");
  require(!mr::validCase(std::string(241,'b'),"Model","USB"),"oversized brand accepted");
  require(!mr::validCase("Brand",std::string(241,'m'),"USB"),"oversized model accepted");
  require(!mr::validCase("Brand","Model",std::string(2001,'s')),"oversized symptom accepted");
}
void logEvidence() {
  const auto logs=mr::analyzeLog("I normal\nF FATAL EXCEPTION: main\nF Fatal signal 11\nE USB disconnect\n");
  require(logs.size()==2,"log indicators missing");
  require(logs[0].count==2,"crash lines not counted");
  require(logs[0].firstEvidence=="F FATAL EXCEPTION: main","first evidence not preserved");
  require(mr::analyzeLog("I normal startup").empty(),"normal log falsely diagnosed");
  require(mr::analyzeLog("").empty(),"empty log returned findings");
  const auto findings=mr::analyzeLog("WATCHDOG\nOutOfMemoryError\ndevice unauthorized\navc: denied\n");
  require(findings.size()==4,"supported log categories not recognized");
  bool rejected=false;
  try { mr::analyzeLog(std::string(512*1024+1,'x')); }
  catch(const std::invalid_argument &) { rejected=true; }
  require(rejected,"oversized log accepted");
}
}
int main() {
  try {
    catalogCoverage();guidedWorkflows();caseInputs();logEvidence();
    std::cout<<"Core rules and input validation passed\n";return 0;
  } catch(const std::exception &error) { std::cerr<<error.what()<<'\n';return 1; }
}
