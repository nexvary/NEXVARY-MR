import QtQuick
import QtQuick.Controls
import QtTest
import "../../qml" as MR
TestCase {
 id:test
 name:"RetentionViewport"
 when:windowShown
 MR.Main {id:app}
 function test_minimumViewport(){
  app.governmentMode=true;app.showDevicePanel=false;app.navigateSection(0)
  const password="Synthetic-only UI password 2026!"
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-VIEWPORT",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic investigator",seizedDeviceDescription:"Synthetic fixture",authorityReference:"Synthetic authority",issuingAuthority:"Lab",scope:"Synthetic layout QA only"});verify(id!=="",lawCaseBridge.error)
  const dev=lawCaseBridge.addDevice({model:"Synthetic fixture",custodian:"Synthetic custodian"}),e=lawCaseBridge.addEvidence(dev,syntheticEvidencePath,"Synthetic viewport fixture","Synthetic custodian",true);verify(e!=="")
  verify(identityBridge.login("ui-admin",password));const reviewer=identityBridge.createUser("viewport-reviewer","Synthetic reviewer","reviewer",password);verify(reviewer!=="");verify(identityBridge.setCaseGrant(id,reviewer,true));verify(identityBridge.login("viewport-reviewer",password));verify(lawCaseBridge.openCase(id))
  verify(lawCaseBridge.retentionOperation("policy",{label:"Synthetic viewport policy",days:365,archiveAfterDays:0,authorityReference:"SYNTHETIC-POLICY"}));const pid=lawCaseBridge.activeCase.retention.policies[0].id;verify(lawCaseBridge.retentionOperation("assign",{evidenceId:e,policyId:pid}));verify(lawCaseBridge.retentionOperation("hold",{evidenceId:e,kind:"legal-hold",reason:"Synthetic preservation",authorityReference:"SYNTHETIC-HOLD"}))
  const law=findChild(app,"lawEnforcementWorkspace");law.section=11
  for(const width of [1180,1366])for(const ar of [true,false]){
   app.width=width;app.height=760;app.ar=ar;wait(80);verify(uiEvidence.requestFrame(app)&&waitForRendering(app.contentItem))
   const scroll=findChild(app,"lawRetentionPage");scroll.assessment=lawCaseBridge.retentionAssessment(e);scroll.contentItem.contentY=0
   verify(uiEvidence.requestFrame(app)&&waitForRendering(app.contentItem));verify(scroll.width>0)
   for(const name of ["retentionAssess","retentionArchive","retentionCreatePolicy","retentionAssignPolicy","retentionPlaceHold","retentionRequestDisposal","retentionBack"]){const button=findChild(app,name);verify(button!==null);const point=button.mapToItem(scroll.contentItem,0,0);verify(point.x>=-1,name+" starts outside content");verify(point.x+button.width<=scroll.availableWidth+1,name+" exceeds content width")}
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"retention-minimum-"+width+"-"+(ar?"ar":"en")+".png"))
  }
 }
}
