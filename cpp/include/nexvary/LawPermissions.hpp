#pragma once
#include <string_view>
namespace nexvary::law {
enum class Role { Investigator, Reviewer, Auditor, Trainer, RepairTechnician, Unknown };
enum class Action { ReadCase, CreateCase, EditCase, ArchiveCase, CloseCase, AddDevice, ReadDevice, AddEvidence, TransferCustody, ExportReport, ExportEvidence, AnalyzeEvidence, BackupCase, RestoreCase, ReadAudit, AccessTraining, ExecuteDevice, ManageRetention, PreserveEvidence, ReleasePreservation, ArchiveEvidence, RequestDisposal, ReviewDisposal };
constexpr std::string_view roleId(Role role) {
 switch(role) {case Role::Investigator:return "investigator";case Role::Reviewer:return "reviewer";case Role::Auditor:return "auditor";case Role::Trainer:return "trainer";case Role::RepairTechnician:return "repair-technician";default:return "unknown";}
}
constexpr bool allows(Role role,Action action) {
 if(action==Action::ExecuteDevice)return false;
 switch(role) {
 case Role::Investigator:
  if(action==Action::RestoreCase||action==Action::PreserveEvidence||action==Action::ArchiveEvidence||action==Action::RequestDisposal)return true;
  return action==Action::ReadCase||action==Action::CreateCase||action==Action::EditCase||action==Action::CloseCase||action==Action::AddDevice||action==Action::ReadDevice||action==Action::AddEvidence||action==Action::TransferCustody||action==Action::ExportReport||action==Action::ExportEvidence||action==Action::AnalyzeEvidence||action==Action::BackupCase||action==Action::ReadAudit||action==Action::AccessTraining;
 case Role::Reviewer:return action==Action::ManageRetention||action==Action::PreserveEvidence||action==Action::ReleasePreservation||action==Action::ArchiveEvidence||action==Action::ReviewDisposal||action==Action::ReadCase||action==Action::ReadDevice||action==Action::ArchiveCase||action==Action::ExportReport||action==Action::ExportEvidence||action==Action::AnalyzeEvidence||action==Action::BackupCase||action==Action::ReadAudit||action==Action::AccessTraining;
 case Role::Auditor:return action==Action::ReadCase||action==Action::ReadDevice||action==Action::ReadAudit;
 case Role::Trainer:return action==Action::AccessTraining;
 case Role::RepairTechnician:return action==Action::AccessTraining;
 default:return false;
 }
}
}
