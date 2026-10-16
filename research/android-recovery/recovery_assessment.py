"""Evidence-based Android recovery feasibility triage.

No phone access, privileged commands, decryption or promises of recovery.
The assessment is a decision aid for an authorized technician.
"""
from dataclasses import dataclass
from enum import Enum


class Source(str, Enum):
    EXTERNAL_CARD = "external_card"
    ACCESSIBLE_BACKUP = "accessible_backup"
    CLOUD_EXPORT = "cloud_export"
    INTERNAL_STORAGE = "internal_storage"


class Verdict(str, Enum):
    ACQUIRE_COPY = "acquire_copy"
    VERIFY_BACKUP = "verify_backup"
    REQUEST_EXPORT = "request_export"
    NOT_SUPPORTED = "not_supported"
    NEED_MORE_INFORMATION = "need_more_information"


@dataclass(frozen=True)
class Assessment:
    verdict: Verdict
    next_action: str
    limitations: tuple[str, ...]


def assess(source: Source, *, factory_reset: bool, authorized: bool,
           readable: bool | None = None, encryption_keys_available: bool = False) -> Assessment:
    if not authorized:
        return Assessment(Verdict.NOT_SUPPORTED, "Obtain documented owner authorization.", (
            "No acquisition or recovery without authorization.",))
    if source is Source.INTERNAL_STORAGE and factory_reset:
        return Assessment(Verdict.NOT_SUPPORTED,
                          "Check independently existing backups or exports.",
                          ("Modern Android factory reset typically destroys file-based encryption keys.",
                           "Shizuku does not provide root access or restore erased encryption keys.",
                           "No recovery claim is made for encrypted internal data."))
    if source is Source.EXTERNAL_CARD:
        if readable is None:
            return Assessment(Verdict.NEED_MORE_INFORMATION,
                              "Determine whether the card is removable and independently readable.",
                              ("Adoptable/encrypted SD cards may depend on device keys.",))
        if not readable:
            return Assessment(Verdict.NOT_SUPPORTED,
                              "Do not write to the original; refer for authorized hardware evaluation.",
                              ("A missing or unreadable card cannot be acquired with this tool.",))
        return Assessment(Verdict.ACQUIRE_COPY,
                          "Acquire a read-only image and hash it before examining a copy.",
                          ("No recovery success is guaranteed.",))
    if source is Source.ACCESSIBLE_BACKUP:
        return Assessment(Verdict.VERIFY_BACKUP,
                          "Inventory available backups and verify integrity before restoration.",
                          ("A backup may omit application-private data.",))
    if source is Source.CLOUD_EXPORT:
        return Assessment(Verdict.REQUEST_EXPORT,
                          "Request an authorized export through the provider's official interface.",
                          ("Provider retention and export coverage vary.",))
    if source is Source.INTERNAL_STORAGE:
        return Assessment(Verdict.NEED_MORE_INFORMATION,
                          "Document OS version, lock state, and accessible user-approved exports.",
                          ("Shizuku is not root.", "Do not bypass device locks.",
                           "Encryption keys being available does not itself grant filesystem access."))
    raise ValueError("Unknown source")
