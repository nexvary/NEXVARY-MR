#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace mr::osint {
struct ParsedLead {
    std::string sourceId;
    std::string value;
    std::string classification = "unverified-lead";
};
struct ParseOutcome {
    std::vector<ParsedLead> leads;
    std::string error;
};
inline bool safeDomain(std::string_view domain) {
    if (domain.empty() || domain.size() > 253 || domain.front() == '.' ||
        domain.back() == '.') return false;
    std::size_t labelLength = 0;
    bool lastHyphen = false;
    for (unsigned char ch : domain) {
        if (ch == '.') {
            if (!labelLength || lastHyphen) return false;
            labelLength = 0;
            lastHyphen = false;
        } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                   (ch >= '0' && ch <= '9') || ch == '-') {
            if (!labelLength && ch == '-') return false;
            if (++labelLength > 63) return false;
            lastHyphen = ch == '-';
        } else return false;
    }
    return labelLength > 0 && !lastHyphen;
}
inline ParseOutcome parseSubfinderLines(std::string_view input, std::string_view scopeDomain) {
    ParseOutcome result;
    if (!safeDomain(scopeDomain) || input.size() > 1024 * 1024) {
        result.error = "invalid-scope-or-size";
        return result;
    }
    std::size_t cursor = 0;
    while (cursor < input.size()) {
        const auto end = input.find('\n', cursor);
        const auto lineEnd = end == std::string_view::npos ? input.size() : end;
        auto line = input.substr(cursor, lineEnd - cursor);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        if (!line.empty()) {
            if (line.size() > 253 || !safeDomain(line) ||
                line.size() <= scopeDomain.size() ||
                line[line.size() - scopeDomain.size() - 1] != '.') {
                result.error = "invalid-or-out-of-scope-domain";
                result.leads.clear();
                return result;
            }
            bool matches = true;
            for (std::size_t i = 0; i < scopeDomain.size(); ++i) {
                const auto a = static_cast<unsigned char>(line[line.size()-scopeDomain.size()+i]);
                const auto b = static_cast<unsigned char>(scopeDomain[i]);
                const auto lower = [](unsigned char ch) { return ch >= 'A' && ch <= 'Z' ? ch + 32 : ch; };
                if (lower(a) != lower(b)) { matches = false; break; }
            }
            if (!matches || result.leads.size() >= 10000) {
                result.error = "out-of-scope-or-too-many-results";
                result.leads.clear();
                return result;
            }
            result.leads.push_back({"subfinder", std::string(line)});
        }
        if (end == std::string_view::npos) break;
        cursor = end + 1;
    }
    return result;
}
} // namespace mr::osint
