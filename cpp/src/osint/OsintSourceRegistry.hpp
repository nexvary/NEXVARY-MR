#pragma once
#include <array>
#include <string_view>

namespace mr::osint {
enum class SourceKind { LocalMetadata, PassiveDomain, PublicUsername, MultiSource, Phone, Commercial };
enum class IntegrationMode { LocalExecutable, LocalService, ExternalApi };
struct SourceDescriptor {
    std::string_view id;
    std::string_view displayName;
    SourceKind kind;
    IntegrationMode mode;
    bool enabledByDefault;
    bool externalDisclosure;
    bool requiresCaseAuthorization;
};
// Registry metadata only: no subprocess launch or network calls.
inline constexpr std::array<SourceDescriptor, 8> kSourceRegistry{{
    {"exiftool", "ExifTool", SourceKind::LocalMetadata, IntegrationMode::LocalExecutable, false, false, true},
    {"subfinder", "Subfinder", SourceKind::PassiveDomain, IntegrationMode::LocalExecutable, false, true, true},
    {"sherlock", "Sherlock", SourceKind::PublicUsername, IntegrationMode::LocalExecutable, false, true, true},
    {"maigret", "Maigret", SourceKind::PublicUsername, IntegrationMode::LocalExecutable, false, true, true},
    {"spiderfoot", "SpiderFoot", SourceKind::MultiSource, IntegrationMode::LocalService, false, true, true},
    {"phoneinfoga", "PhoneInfoga", SourceKind::Phone, IntegrationMode::LocalService, false, true, true},
    {"osint-industries", "OSINT Industries", SourceKind::Commercial, IntegrationMode::ExternalApi, false, true, true},
    {"lampyre", "Lampyre", SourceKind::Commercial, IntegrationMode::ExternalApi, false, true, true}
}};
inline const SourceDescriptor* findSource(std::string_view id) {
    for (const auto& source : kSourceRegistry) {
        if (source.id == id) return &source;
    }
    return nullptr;
}
} // namespace mr::osint
