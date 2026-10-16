#pragma once
#include <string>
#include <string_view>
#include <vector>
namespace mr {
struct DeviceFamily { std::string brand; std::vector<std::string> families; };
struct Diagnosis { std::string nextCheck; std::string safety; };
std::vector<DeviceFamily> catalog();
Diagnosis diagnose(std::string_view symptom);
bool validCase(std::string_view brand, std::string_view model, std::string_view symptom);
}
