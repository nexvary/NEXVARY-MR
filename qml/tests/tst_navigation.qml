import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest
import ".." as MR
TestCase {
 id:test
 name:"WorkstationNavigation"
 when:windowShown
 width:1536;height:980
 MR.Main {id:app}
 property bool reviewCapture:false
 Rectangle {parent:app.contentItem;z:100000;visible:test.reviewCapture;anchors.bottom:parent.bottom;anchors.left:parent.left;anchors.right:parent.right;height:24;color:"#4A1722"
  Label {anchors.centerIn:parent;text:"UI REVIEW — SYNTHETIC FIXTURES — NO LIVE SEARCH / NO PAYMENT";color:"white";font.pixelSize:12}
 }
 readonly property var windowUnderTest:app
 SignalSpy {id:avbInspectionFinished;target:firmwareBridge;signalName:"inspectionFinished"}
 function init(){app.governmentMode=false;findChild(app,"deviceTabs").currentIndex=0;findChild(app,"workshopTabs").currentIndex=0;findChild(app,"diagnosticTabs").currentIndex=0;app.allowClose=false;app.toastText="";app.ar=true;app.darkTheme=true;app.showDevicePanel=false;app.width=1536;app.height=980;app.currentPage=0;verify(renderedFrame())}
 function renderedFrame(){return uiEvidence.requestFrame(app)&&waitForRendering(app.contentItem)}
 function reviewedUiException(scroll,panel){
  const reason=findChild(panel,"governmentNetworkReason"),approval=findChild(panel,"governmentNetworkApproval")
  reason.text="Synthetic reviewed UI category exception";approval.checked=true
  artifactClick(scroll,findChild(panel,"governmentNetworkGrant-device-tools"));compare(operationPolicy.offline,true)
  const id=panel.selectedRequest;verify(id.length>0);verify(!operationPolicy.reviewException(id,"Self review must fail"))
  verify(identityBridge.login("ui-admin","Synthetic-only UI password 2026!"));verify(renderedFrame())
  reason.text="Synthetic separate-account review";artifactClick(scroll,findChild(panel,"governmentPolicyReview-"+id))
  reason.text="Synthetic separate-account approval";artifactClick(scroll,findChild(panel,"governmentPolicyApprove-"+id));compare(operationPolicy.offline,true)
  verify(!operationPolicy.activateException(id))
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"));verify(renderedFrame())
  artifactClick(scroll,findChild(panel,"governmentPolicyActivate-"+id));compare(operationPolicy.offline,false)
  return id
 }
 Component {id:professionalIconFixture;MR.SemanticIcon {width:32;height:32}}
 function test_professionalIconAcceptance(){
  const names=["home","phone","gauge","board","arrow_square_down","clipboard_task","database","settings","history","document_lock","toolbox","wrench","question_circle","document_text","search_visual","book_open_lightbulb","shield_checkmark","data_bar_vertical_ascending","puzzle_piece","phone_laptop","arrow_sync","checkmark_circle","error_circle","warning","evidence_box","training_lab","check_mark"];
  for(const name of names){
   const icon=professionalIconFixture.createObject(app.contentItem,{name:name});verify(icon!==null);
   tryCompare(icon,"status",Image.Ready,5000);compare(icon.usingFallback,false);verify(icon.sourceSize.width>=32);icon.destroy();
  }
  const missing=professionalIconFixture.createObject(app.contentItem,{name:"missing-icon-fixture"});
  tryCompare(missing,"usingFallback",true,5000);tryCompare(missing,"status",Image.Ready,5000);missing.destroy();
  app.setWorkspace(true);app.navigatePrimary(3);verify(renderedFrame());
  const nav=findChild(app,"mrPrimaryNavigation");verify(nav!==null);
  for(const size of [[1366,768],[1920,1080],[2560,1440]])for(const ar of [true,false]){
   app.width=size[0];app.height=size[1];app.ar=ar;verify(renderedFrame());
   for(let i=0;i<nav.sectionCount;i++){
    const button=nav.itemAt(i),icon=findChild(button,"primaryNavIcon"+i);
    verify(button!==null);verify(icon!==null);tryCompare(icon,"status",Image.Ready,5000);
    verify(icon.width>0&&icon.height>0);verify(icon.width<=button.width);verify(icon.height<=button.height);
   }
   if(size[0]===1366&&uiScaleTag==="1")verify(uiEvidence.captureWindow(app,uiScreenshotBase+"icon-acceptance-"+(ar?"ar":"en")+".png"));
  }
 }
 function test_phoneOsintBilingualBoundary(){
  identityBridge.logout()
  app.navigatePrimary(3);verify(renderedFrame());
  const ws=findChild(app,"osintWorkspace");
  for(const ar of [true,false]){
   app.ar=ar;
   verify(renderedFrame());
   compare(ws.LayoutMirroring.enabled,ar);
   verify(findChild(ws,"intelligencePhoneCard").visible);
   verify(!findChild(ws,"intelligenceOpenPhoneSearch").enabled);
  }
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"))
  app.goBack()
 }
 function test_fullGovernmentNavigationAndProfessionalIcons(){
  app.setWorkspace(true);app.width=1536;
  const nav=findChild(app,"mrPrimaryNavigation");verify(nav!==null);compare(nav.sectionCount,13);
  for(const ar of [true,false]){
   app.ar=ar;verify(renderedFrame());
   verify(ar?nav.itemAt(0).x>nav.itemAt(11).x:nav.itemAt(0).x<nav.itemAt(11).x);
   const dashboard=nav.itemAt(0), intelligence=nav.itemAt(app.intelligenceSectionIndex)
   compare(dashboard.text,ar?"لوحة التحكم الأمنية":"Security Dashboard")
   compare(intelligence.text,ar?"الاستخبارات":"Intelligence")
   compare(Math.abs(dashboard.x-intelligence.x),dashboard.width+4)
   for(let i=0;i<nav.sectionCount;i++){
    nav.reveal(i);verify(renderedFrame());
    const button=nav.itemAt(i);verify(button!==null);
    compare(button.text,ar?app.governmentSections[i].ar:app.governmentSections[i].en);
    const icon=findChild(button,"primaryNavIcon"+i);verify(icon!==null);tryCompare(icon,"status",Image.Ready,5000);
    verify(icon.source.toString().indexOf("icons-professional/")>=0);
    mouseClick(button);compare(app.activeSection,i);compare(app.currentPage,app.governmentSections[i].page);
   }
   app.navigateSection(6);verify(renderedFrame());
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"all-government-navigation-"+(ar?"ar":"en")+".png"));
  }
  app.width=980;verify(renderedFrame());verify(nav.overflowing);
  nav.reveal(11);verify(renderedFrame());mouseClick(nav.itemAt(11));compare(app.activeSection,11);
  app.setWorkspace(false);compare(nav.sectionCount,14);
 }
 function test_approvedHomeAndPrimaryNavigation(){
  app.navigatePrimary(0);verify(renderedFrame());compare(app.currentPage,0);
  const art=findChild(app,"mrHomeArtwork");verify(art!==null);tryCompare(art,"status",Image.Ready,5000);
  verify(art.source.toString().indexOf("mr-home-background.png")>=0);
  verify(uiEvidence.captureWindow(app,uiScreenshotBase+"approved-home-ar.png"));
  app.navigatePrimary(3);compare(app.currentPage,app.intelligencePage)
  verify(findChild(app,"intelligenceWorkspace").visible)
  identityBridge.logout()
  verify(findChild(app,"lampyreApiKey")===null)
  app.goBack();compare(app.currentPage,0)
  app.navigatePrimary(3);verify(renderedFrame())
  const permissions=findChild(app,"intelligenceAccountPermissions")
  reveal(findChild(app,"intelligenceWorkspace"),permissions);mouseClick(permissions)
  compare(app.currentPage,12);compare(app.activeSection,10)
  app.setWorkspace(true);app.navigateSection(app.intelligenceSectionIndex)
  verify(renderedFrame());
  const intelligence=findChild(app,"intelligenceWorkspace")
  verify(intelligence.visible)
  compare(intelligence.contentItem.contentY,intelligence.contentItem.originY)
  verify(!identityBridge.authenticated);verify(!lawCaseBridge.activeCase.id)
  verify(findChild(app,"lampyreTestConnection")===null)
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"))
  app.goBack();compare(app.currentPage,12);compare(app.activeSection,0)
  app.navigatePrimary(1);compare(app.currentPage,1);compare(app.governmentMode,false);
  app.setWorkspace(true);app.navigateSection(6);verify(renderedFrame());
  verify(!findChild(app,"intelligenceWorkspace").visible);
  verify(findChild(app,"governmentAnalyzeEvidence").visible);
 }
 function test_approvedMrPaletteAndBackground(){
  compare(String(app.design.background),"#040507");
  compare(String(app.design.surface),"#08121b");
  compare(String(app.design.accent),"#23b9e7");
  const background=findChild(app,"workstationBackgroundImage");verify(background!==null);
  tryCompare(background,"status",Image.Ready,5000);verify(background.source.toString().indexOf("mr-network-wave.png")>=0);
  compare(background.width,app.width);compare(background.height,app.height);
 }
 function test_osintWorkspaceBilingualNavigation(){
  app.navigatePrimary(3);verify(renderedFrame());
  const page=findChild(app,"intelligenceWorkspace"),ws=findChild(app,"osintWorkspace");
  ws.demoMode=true;
  for(const ar of [true,false]){
   app.ar=ar;
   for(const tab of [1,2]){
    ws.activeTab=tab;page.contentItem.contentY=0;verify(renderedFrame());
    compare(ws.LayoutMirroring.enabled,ar);
    verify(uiEvidence.captureWindow(app,uiScreenshotBase+"osint-workspace-"+tab+"-"+(ar?"ar":"en")+".png"));
   }
  }
  ws.showInspector=false;compare(findChild(app,"osintAnalysisPane").showInspector,false);
  ws.showRequests=false;compare(findChild(app,"osintAnalysisPane").showRequests,false);
  ws.showInspector=true;ws.showRequests=true;
  ws.activeTab=1;
  lampyreEvaluation.runSynthetic(0);compare(lampyreEvaluation.busy,true);
  lampyreEvaluation.cancel();compare(lampyreEvaluation.state,"synthetic-cancelled");
  lampyreEvaluation.runSynthetic(0);tryCompare(lampyreEvaluation,"state","synthetic-completed",3000);
  compare(lampyreEvaluation.result.networkRequests,0);compare(lampyreEvaluation.result.photonsConsumed,0);
  compare(lampyreEvaluation.result.records.length,3);
  ws.activeTab=2;const analysis=findChild(app,"osintAnalysisPane");
  analysis.pick(0);compare(analysis.selected.value,"research@example.invalid");
  analysis.filterText="example.invalid";compare(analysis.filtered.length,2);analysis.filterText="";
  for(const ar of [true,false]){
   app.ar=ar;
   for(const mode of [0,1]){
    analysis.viewMode=mode;page.contentItem.contentY=0;verify(renderedFrame());
    verify(uiEvidence.captureWindow(app,uiScreenshotBase+"osint-analysis-"+mode+"-"+(ar?"ar":"en")+".png"));
   }
  }
  lampyreEvaluation.runSynthetic(1);tryCompare(lampyreEvaluation,"state","synthetic-failed",3000);
  compare(analysis.records.length,0);compare(analysis.selectedIndex,-1);
  ws.activeTab=0;app.goBack();compare(ws.demoMode,false);
  ws.showLocalTools=true;
 }
 function test_governmentOperationReceipts(){
  operationPolicy.revokeAll();app.setWorkspace(true);app.navigateSection(10);verify(renderedFrame())
  const id=uiEvidence.recordSyntheticOperation("security-scan");verify(id.length>0)
  const scroll=findChild(app,"identityPanel"),panel=findChild(scroll,"governmentNetworkPanel")
  const row=findChild(panel,"governmentOperationReceipt-"+id);verify(row!==null);compare(operationPolicy.operationReceipts[0].status,"completed");verify(row.text.indexOf("aaaaaaaa")>=0)
  for(const ar of [true,false]){app.ar=ar;reveal(scroll,row);verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"operation-receipts-"+(ar?"ar":"en")+".png"))}
  operationPolicy.revokeAll()
  const partitionId=uiEvidence.recordSyntheticOperation("partition-analysis");verify(partitionId.length>0)
  verify(renderedFrame());const partitionRow=findChild(panel,"governmentOperationReceipt-"+partitionId);verify(partitionRow!==null)
  compare(operationPolicy.operationReceipts[0].kind,"partition-analysis")
  for(const ar of [true,false]){app.ar=ar;reveal(scroll,partitionRow);verify(renderedFrame());verify(partitionRow.text.indexOf(ar?"تحليل أقسام الصورة":"Image partition analysis")>=0);verify(uiEvidence.captureWindow(app,uiScreenshotBase+"partition-receipt-"+(ar?"ar":"en")+".png"))}
  identityBridge.logout();verify(renderedFrame());compare(operationPolicy.operationReceipts.length,0)
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"));operationPolicy.revokeAll()
 }
 function test_governmentNetworkPolicyControls(){
  operationPolicy.revokeAll();compare(operationPolicy.offline,true)
  app.setWorkspace(true);app.navigateSection(10);verify(renderedFrame())
  const scroll=findChild(app,"identityPanel"),panel=findChild(scroll,"governmentNetworkPanel")
  const reason=findChild(panel,"governmentNetworkReason"),approval=findChild(panel,"governmentNetworkApproval")
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"));verify(renderedFrame())
  compare(findChild(panel,"governmentNetworkGrant-device-tools").enabled,false)
  verify(!operationPolicy.grant("device-tools","Direct grant disabled",true))
  reviewedUiException(scroll,panel);compare(approval.checked,false);verify(operationPolicy.auditStatus.healthy)
  verify(findChild(app,"governmentPolicyState").text.indexOf("استثناء")>=0)
  for(const arabic of [true,false])for(const size of [[840,480],[1366,768],[1536,980]]){
   app.ar=arabic;app.width=size[0];app.height=size[1];scroll.contentItem.contentY=0;wait(100);verify(renderedFrame())
   const header=uiEvidence.headerFingerprint(app),scope=findChild(panel,"governmentNetworkScope")
   verify(scope.height>=scope.contentHeight);reveal(scroll,scope);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header)
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-network-"+(arabic?"ar":"en")+"-"+size[0]+"-"+uiScaleTag+".png"))
  }
  artifactClick(scroll,findChild(panel,"governmentNetworkRevokeAll"));compare(operationPolicy.offline,true)
  reviewedUiException(scroll,panel)
  identityBridge.logout();compare(operationPolicy.offline,true);compare(reason.text,"");compare(approval.checked,false)
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"));app.setWorkspace(false)
 }
 function test_governmentNavigationAndWorkingCopies(){
  app.setWorkspace(true);compare(app.currentPage,12);compare(app.sections.length,13)
  const law=findChild(app,"lawEnforcementWorkspace")
  for(const arabic of [true,false]){
   app.ar=arabic;app.width=1366;app.height=768
   for(let i=0;i<app.sections.length;i++){
    const button=app.sectionButton(i);reveal(findChild(app,"sidebarScroll"),button);mouseClick(button);compare(app.activeSection,i);compare(app.currentPage,app.sections[i].page)
    if(app.sections[i].law!==undefined)compare(law.section,app.sections[i].law)
    if(app.sections[i].evidence!==undefined)compare(law.evidenceView,app.sections[i].evidence)
    verify(renderedFrame())
   }
   app.navigateSection(0);app.width=1536;app.height=980;reveal(findChild(app,"sidebarScroll"),app.sectionButton(0));verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-"+(arabic?"ar":"en")+".png"))
  }
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-EXPORT-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic fixture",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic evidence only"});verify(id!=="",lawCaseBridge.error)
  const dev=lawCaseBridge.addDevice({model:"Synthetic fixture",custodian:"Synthetic custodian"});verify(dev!=="")
  verify(lawCaseBridge.addEvidence(dev,syntheticEvidencePath,"Synthetic copy UI","Synthetic custodian",true)!=="")
  app.navigateSection(4);verify(renderedFrame());const evidencePage=findChild(app,"lawEvidenceWorkspace");verify(evidencePage!==null)
  findChild(app,"lawCopyPurpose").text="Independent synthetic analysis";findChild(app,"lawCopyDirectory").text=syntheticWorkingCopyPath
  const consent=findChild(app,"lawCopyConsent"),button=findChild(app,"lawExportWorkingCopy");compare(button.enabled,false);consent.checked=true;compare(button.enabled,true)
  reveal(evidencePage,button);reveal(findChild(app,"lawAuditScroll"),button);verify(renderedFrame());mouseClick(button)
  verify(!!evidencePage.lastExport.manifestSha256,lawCaseBridge.error);compare(findChild(app,"lawCopyManifestDigest").text.length,64);compare(consent.checked,false)
  app.navigateSection(8);compare(law.section,9);app.goBack();compare(law.section,6)
  const capture=lawCaseBridge.addEvidence(dev,syntheticCapturePath,"Synthetic PCAP UI","Synthetic custodian",true);verify(capture!=="",lawCaseBridge.error)
  app.navigateSection(6);compare(app.currentPage,14);const selector=findChild(app,"governmentAnalysisEvidence");selector.currentIndex=lawCaseBridge.activeCase.evidence.length-1
  verify(renderedFrame());mouseClick(findChild(app,"governmentAnalyzeEvidence"));verify(lawCaseBridge.activeCase.analyses.length>0,lawCaseBridge.error);verify(findChild(app,"governmentAnalysisResult").text.length>0)
  for(const arabic of [true,false]){app.ar=arabic;verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-analysis-"+(arabic?"ar":"en")+".png"))}
  app.setWorkspace(false);compare(app.currentPage,0)
 }
 function test_governmentAuthorizedImportControls(){
  app.setWorkspace(true)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-IMPORT-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic document",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic document only"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic source",custodian:"Synthetic custodian"});verify(device!=="")
  const source=lawCaseBridge.addEvidence(device,syntheticEvidencePath,"Synthetic text import","Synthetic custodian",true);verify(source!=="")
  app.navigateSection(6);verify(renderedFrame());const page=findChild(app,"governmentAnalysisWorkspace"),panel=findChild(app,"governmentImportPanel"),button=findChild(app,"governmentImportStart"),consent=findChild(app,"governmentImportConsent")
  findChild(app,"governmentImportEvidence").currentIndex=0;findChild(app,"governmentImportAdapter").currentIndex=0
  compare(button.enabled,false);findChild(app,"governmentImportPurpose").text="Synthetic authorized UI import";compare(button.enabled,false);consent.checked=true;compare(button.enabled,true)
  reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"importBusy",false,8000);verify(!!lawCaseBridge.importResult.reportJson,lawCaseBridge.error);compare(JSON.parse(lawCaseBridge.importResult.reportJson).status,"imported");compare(consent.checked,false)
  for(const arabic of [true,false]){app.ar=arabic;app.width=1366;app.height=980;page.contentItem.contentY=0;wait(120);verify(renderedFrame());const header=uiEvidence.headerFingerprint(app);compare(header.length,64);page.contentItem.contentY=panel.y;wait(120);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header,"Scrolling evidence must not paint over the application header");verify(uiEvidence.captureWindow(app,uiScreenshotBase+"authorized-import-"+(arabic?"ar":"en")+"-"+uiScaleTag+".png"))}
  findChild(app,"governmentImportAdapter").currentIndex=1;consent.checked=true;reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"importBusy",false,8000);compare(JSON.parse(lawCaseBridge.importResult.reportJson).status,"rejected");verify(lawCaseBridge.verifyEvidence(source));verify(lawCaseBridge.verifyAudit());lawCaseBridge.closeWorkspace();compare(findChild(app,"governmentImportPurpose").text,"");app.setWorkspace(false)
 }
 function test_governmentSummaryLifetime(){
  app.setWorkspace(true);app.navigateSection(6);verify(renderedFrame())
  const page=findChild(app,"governmentAnalysisWorkspace"),names=["governmentSummaryFirst","governmentSummarySecond","governmentSummaryThird"],cards=names.map(function(name){return findChild(page,name)})
  for(const card of cards)verify(card!==null)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-SUMMARY-LIFETIME",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic summaries",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic summary lifecycle"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic summary",custodian:"Synthetic custodian"});verify(device!=="")
  const capture=lawCaseBridge.addEvidence(device,syntheticCapturePath,"Synthetic PCAP","Synthetic custodian",true),exif=lawCaseBridge.addEvidence(device,syntheticExifPath,"Synthetic JPEG","Synthetic custodian",true);verify(capture!==""&&exif!=="")
  for(let cycle=0;cycle<4;cycle++){
   // Include the empty hidden EXIF branch: its translated authenticity value
   // previously rebuilt all three Repeater delegates on each language change.
   findChild(page,"governmentAnalysisEvidence").currentIndex=cycle%2?0:1;findChild(page,"governmentAnalysisParser").currentIndex=cycle%2?0:1
   page.result=cycle===0?({}):lawCaseBridge.analyzeEvidence(cycle%2?capture:exif,cycle%2?"classic-pcap":"jpeg-exif")
   if(cycle>0)verify(!!page.result.reportJson,lawCaseBridge.error)
   for(const ar of [true,false]){
    app.ar=ar;app.width=cycle%2?840:1536;app.height=cycle%2?480:980;verify(renderedFrame())
    for(let i=0;i<names.length;i++)compare(findChild(page,names[i]),cards[i],"Summary slot survives locale, parser and visibility changes")
    if(page.result.reportJson){reveal(page,findChild(page,"governmentSummaryCards"));verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"summary-lifetime-"+cycle+"-"+(ar?"ar":"en")+"-"+uiScaleTag+".png"))}
   }
  }
  lawCaseBridge.closeWorkspace();verify(renderedFrame());for(let i=0;i<names.length;i++)compare(findChild(page,names[i]),cards[i]);compare(findChild(page,"governmentSummaryCards").visible,false)
 }
 function test_governmentPartitionControls(){
  app.setWorkspace(true)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-PARTITION-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic evidence",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic UI only"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic source",custodian:"Synthetic custodian"});verify(device!=="")
  verify(lawCaseBridge.addEvidence(device,syntheticEvidencePath,"Synthetic UI source","Synthetic custodian",true)!=="")
  app.navigateSection(6);verify(renderedFrame())
  const scroll=findChild(app,"governmentAnalysisWorkspace"),panel=findChild(app,"governmentPartitionPanel"),run=findChild(app,"governmentPartitionRun"),consent=findChild(app,"governmentPartitionConsent"),tool=findChild(app,"governmentPartitionTool")
  compare(run.enabled,false);verify(!lawCaseBridge.partitionsBusy);tool.text="Synthetic tool path only";findChild(app,"governmentPartitionEvidence").currentIndex=0;consent.checked=true;compare(run.enabled,true)
  for(const ar of [true,false]){app.ar=ar;app.width=1366;app.height=768;reveal(scroll,panel);verify(renderedFrame());verify(panel.height>=panel.implicitHeight);verify(uiEvidence.captureWindow(app,uiScreenshotBase+"registered-partition-"+(ar?"ar":"en")+".png"))}
  lawCaseBridge.closeWorkspace();verify(renderedFrame());compare(consent.checked,false);compare(run.enabled,false);compare(findChild(app,"governmentPartitionResult").text,"")
 }
 function test_governmentAdbBackupControls(){
  app.setWorkspace(true)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-ADB-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic ADB fixture",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic inventory only"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic backup",custodian:"Synthetic custodian"});verify(device!=="")
  const source=lawCaseBridge.addEvidence(device,syntheticAdbBackupPath,"Synthetic ADB v5 backup","Synthetic custodian",true);verify(source!=="")
  app.navigateSection(6);verify(renderedFrame())
  const page=findChild(app,"governmentAnalysisWorkspace"),panel=findChild(app,"governmentImportPanel"),button=findChild(app,"governmentImportStart"),selector=findChild(app,"governmentImportAdapter"),consent=findChild(app,"governmentImportConsent")
  findChild(app,"governmentImportEvidence").currentIndex=0;selector.currentIndex=6;compare(selector.model[6].id,"adb-backup-v5")
  findChild(app,"governmentImportPurpose").text="Synthetic authorized backup inventory";compare(button.enabled,false);consent.checked=true
  reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"importBusy",false,8000)
  const report=JSON.parse(lawCaseBridge.importResult.reportJson);compare(report.status,"imported");compare(report.backup.membersScanned,4);compare(report.backup.inventoryOnly,true);compare(report.findings.length,4);compare(report.findings[2].memberSha256.length,64);compare(consent.checked,false)
  for(const arabic of [true,false])for(const size of [[840,480],[1366,768],[1536,980]]){
   app.ar=arabic;app.width=size[0];app.height=size[1];page.contentItem.contentY=0;wait(100);verify(renderedFrame());const header=uiEvidence.headerFingerprint(app)
   const scope=findChild(app,"governmentBackupScope");verify(scope.visible);verify(scope.text.indexOf(arabic?"غير موثّق":"untrusted")>=0);verify(scope.height>=scope.contentHeight)
   reveal(page,scope);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header)
   page.contentItem.contentY=panel.y;wait(100);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header)
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-adb-backup-"+(arabic?"ar":"en")+"-"+size[0]+"-"+uiScaleTag+".png"))
  }
  verify(lawCaseBridge.verifyEvidence(source));verify(lawCaseBridge.verifyAudit());lawCaseBridge.closeWorkspace();compare(button.enabled,false);app.setWorkspace(false)
 }
 function test_governmentImportLayoutLifetime(){
  for(let cycle=0;cycle<4;cycle++){
   app.setWorkspace(true)
   const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-LIFETIME-"+cycle,investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic ADB fixture",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic layout lifetime only"});verify(id!=="",lawCaseBridge.error)
   const device=lawCaseBridge.addDevice({model:"Synthetic backup",custodian:"Synthetic custodian"});verify(device!=="")
   const source=lawCaseBridge.addEvidence(device,syntheticAdbBackupPath,"Synthetic lifetime inventory","Synthetic custodian",true);verify(source!=="")
   app.navigateSection(6);verify(renderedFrame())
   verify(lawCaseBridge.importEvidence(source,"adb-backup-v5","Synthetic layout lifetime",true))
   tryCompare(lawCaseBridge,"importBusy",false,8000);compare(JSON.parse(lawCaseBridge.importResult.reportJson).status,"imported")
   compare(JSON.parse(lawCaseBridge.importResult.reportJson).findings.length,4);compare(lawCaseBridge.activeCase.analyses.length,1);verify(renderedFrame())
   // Reset populated models, change direction and resize before the next frame.
   // Exercise real case/import clearing and rendering, regardless of the
   // implementation of the dynamic result containers.
   app.ar=cycle%2===0;app.width=840;app.height=480
   lawCaseBridge.closeWorkspace();verify(!lawCaseBridge.activeCase.id);verify(!lawCaseBridge.importResult.reportJson)
   app.width=1536;app.height=980;verify(renderedFrame())
   compare(findChild(app,"governmentImportStatus").text,"")
   app.setWorkspace(false);verify(renderedFrame())
  }
 }
 function artifactClick(scroll,target){
  verify(renderedFrame());reveal(scroll,target);verify(renderedFrame());reveal(scroll,target);verify(renderedFrame())
  const point=target.mapToItem(app.contentItem,target.width/2,target.height/2)
  verify(point.x>=0&&point.x<app.width&&point.y>=120&&point.y<app.height-30,"Artifact control is rendered inside the viewport")
  mouseClick(target)
 }
 function test_governmentArtifactControls(){
  app.setWorkspace(true)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-ARTIFACT-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic findings",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic provenance only"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic source",custodian:"Synthetic custodian"});verify(device!=="")
  for(let i=0;i<2;i++){
   const source=lawCaseBridge.addEvidence(device,syntheticArtifactPath,"Synthetic duplicate bytes "+i,"Synthetic custodian",true);verify(source!=="")
   verify(lawCaseBridge.importEvidence(source,"json","Synthetic artifact UI",true));tryCompare(lawCaseBridge,"importBusy",false,8000);verify(!!lawCaseBridge.importResult.reportJson,lawCaseBridge.error)
  }
  app.navigateSection(6);verify(renderedFrame());const page=findChild(app,"governmentAnalysisWorkspace"),panel=findChild(app,"governmentArtifactPanel"),button=findChild(app,"governmentArtifactBuild"),query=findChild(app,"governmentArtifactQuery")
  reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"artifactsBusy",false,8000);compare(lawCaseBridge.artifactsResult.rows.length,30);compare(lawCaseBridge.artifactsResult.digestGroups.length,1);compare(lawCaseBridge.artifactsResult.digestGroups[0].memberCount,2)
  const first=panel.selectAt(0);verify(first!==null);reveal(page,first);verify(renderedFrame());mouseClick(first);compare(panel.selected.provenancePath.length,5);verify(findChild(app,"governmentArtifactPath").visible)
  const next=findChild(app,"governmentArtifactNext"),previous=findChild(app,"governmentArtifactPrevious")
  artifactClick(page,next);tryCompare(panel,"resultPage",1);compare(!!panel.selected.id,false)
  artifactClick(page,next);tryCompare(panel,"resultPage",2);tryCompare(next,"enabled",false)
  artifactClick(page,previous);tryCompare(panel,"resultPage",1)
  artifactClick(page,previous);tryCompare(panel,"resultPage",0);tryCompare(previous,"enabled",false)
  query.text="أحمد";reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"artifactsBusy",false,8000);compare(lawCaseBridge.artifactsResult.rows.length,2);compare(panel.valueAt(0),"أحمد")
  for(const arabic of [true,false])for(const size of [[840,480],[1366,768],[1536,980]]){
   app.ar=arabic;app.width=size[0];app.height=size[1];page.contentItem.contentY=0;wait(100);verify(renderedFrame());const header=uiEvidence.headerFingerprint(app)
   const scope=findChild(app,"governmentArtifactScope");verify(scope.height>=scope.contentHeight);verify(scope.mapToItem(page,0,scope.height).y<=query.mapToItem(page,0,0).y)
   reveal(page,button);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header)
   page.contentItem.contentY=panel.y;wait(100);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header)
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-artifacts-"+(arabic?"ar":"en")+"-"+size[0]+"-"+uiScaleTag+".png"))
  }
  app.width=1536;app.height=980;const select=panel.selectAt(0);reveal(page,select);verify(renderedFrame());mouseClick(select);compare(panel.selected.provenancePath.length,5)
  for(const arabic of [true,false]){app.ar=arabic;const path=findChild(app,"governmentArtifactPath");page.contentItem.contentY+=path.mapToItem(page.contentItem,0,0).y;wait(100);verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-artifact-path-"+(arabic?"ar":"en")+"-"+uiScaleTag+".png"))}
  query.text=".*";reveal(page,button);verify(renderedFrame());mouseClick(button);tryCompare(lawCaseBridge,"artifactsBusy",false,8000);compare(lawCaseBridge.artifactsResult.rows.length,0);compare(lawCaseBridge.artifactsResult.nodes.length,1)
  compare(lawCaseBridge.buildArtifacts("x".repeat(257)),false);const error=findChild(app,"governmentArtifactError");verify(error.visible);verify(error.text.indexOf("256")>=0);verify(!lawCaseBridge.artifactsResult.schema)
  verify(lawCaseBridge.verifyAudit());lawCaseBridge.closeWorkspace();verify(!lawCaseBridge.artifactsResult.schema);compare(!!panel.selected.id,false);compare(query.text,"");compare(button.enabled,false);app.setWorkspace(false)
 }
 function test_governmentTimelineControls(){
  app.setWorkspace(true)
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-TIMELINE-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic timestamp file",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic timeline only"});verify(id!=="",lawCaseBridge.error)
  const device=lawCaseBridge.addDevice({model:"Synthetic source",custodian:"Synthetic custodian"});verify(device!=="")
  const source=lawCaseBridge.addEvidence(device,syntheticTimelinePath,"Synthetic timeline","Synthetic custodian",true);verify(source!=="")
  verify(lawCaseBridge.importEvidence(source,"json","Synthetic timeline UI",true));tryCompare(lawCaseBridge,"importBusy",false,8000);verify(!!lawCaseBridge.importResult.reportJson,lawCaseBridge.error)
  app.navigateSection(6);verify(renderedFrame());const page=findChild(app,"governmentAnalysisWorkspace"),panel=findChild(app,"governmentTimelinePanel"),button=findChild(app,"governmentTimelineBuild")
  reveal(page,button);mouseClick(button);tryCompare(lawCaseBridge,"timelineBusy",false,8000);compare(lawCaseBridge.timelineResult.normalizedCount,15);compare(lawCaseBridge.timelineResult.unresolvedCount,1)
  const next=findChild(app,"governmentTimelineNext"),previous=findChild(app,"governmentTimelinePrevious")
  tryCompare(page,"timelinePage",0);verify(next.enabled);verify(renderedFrame())
  reveal(page,next);verify(renderedFrame());mouseClick(next);tryCompare(page,"timelinePage",1);tryCompare(next,"enabled",false)
  verify(previous.enabled);reveal(page,previous);verify(renderedFrame());mouseClick(previous);tryCompare(page,"timelinePage",0);tryCompare(previous,"enabled",false)
  for(const arabic of [true,false])for(const size of [[840,480],[1366,768],[1536,980]]){
   app.ar=arabic;app.width=size[0];app.height=size[1];page.contentItem.contentY=0;wait(80);verify(renderedFrame());const header=uiEvidence.headerFingerprint(app)
   verify(page.timelineExplanationAt(0).indexOf(arabic?"صحة":"authenticity")>=0,"Timeline uncertainty is explained in the selected language")
   reveal(page,button);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header,"Timeline scrolling must not overpaint the header")
   const top=button.mapToItem(app.contentItem,0,0),bottom=button.mapToItem(app.contentItem,button.width,button.height);verify(top.x>=0&&top.y>=0&&bottom.x<=app.width&&bottom.y<=app.height,"Timeline controls remain reachable")
   page.contentItem.contentY=panel.y;wait(100);verify(renderedFrame());compare(uiEvidence.headerFingerprint(app),header,"Timeline panel capture preserves the header")
   const scope=findChild(app,"governmentTimelineScope"),queryField=findChild(app,"governmentTimelineQuery")
   verify(scope.height>=scope.contentHeight,"Wrapped scope text remains fully allocated")
   verify(scope.mapToItem(page,0,scope.height).y<=queryField.mapToItem(page,0,0).y,"Wrapped scope does not overlap search")
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-timeline-"+(arabic?"ar":"en")+"-"+size[0]+"-"+uiScaleTag+".png"))
  }
  const query=findChild(app,"governmentTimelineQuery");query.text="no-synthetic-match";reveal(page,button);mouseClick(button);tryCompare(lawCaseBridge,"timelineBusy",false,8000);compare(lawCaseBridge.timelineResult.events.length,0)
  verify(lawCaseBridge.verifyEvidence(source));verify(lawCaseBridge.verifyAudit());lawCaseBridge.closeWorkspace();verify(!lawCaseBridge.timelineResult.schema);compare(button.enabled,false);app.setWorkspace(false)
 }
 function test_governmentProtectedBackupControls(){
  app.setWorkspace(true);const law=findChild(app,"lawEnforcementWorkspace")
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-BACKUP-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic device",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic backup review only"});verify(id!=="",lawCaseBridge.error)
  const dev=lawCaseBridge.addDevice({model:"Synthetic fixture",custodian:"Synthetic custodian"});verify(dev!=="");verify(lawCaseBridge.addEvidence(dev,syntheticEvidencePath,"Synthetic backup fixture","Synthetic custodian",true)!=="")
  app.navigateSection(9);compare(law.section,8);verify(renderedFrame())
  const consent=findChild(app,"lawBackupConsent"),save=findChild(app,"lawSaveProtectedBackup"),restore=findChild(app,"lawRestoreProtectedBackup")
  compare(save.enabled,false);compare(restore.enabled,false);consent.checked=true;compare(save.enabled,true);law.saveProtectedBackup(syntheticProtectedBackupPath);verify(!!law.lastBackup.sha256,lawCaseBridge.error);compare(findChild(app,"lawBackupDigest").text.length,64);compare(consent.checked,false)
  findChild(app,"lawRestoreDigest").text=law.lastBackup.sha256;findChild(app,"lawRestoreConsent").checked=true;compare(restore.enabled,true)
  compare(law.restoreProtectedBackup(syntheticProtectedBackupPath),"");verify(lawCaseBridge.error.indexOf("already exists")>=0);compare(lawCaseBridge.activeCase.id,id);verify(lawCaseBridge.verifyAudit())
  for(const arabic of [true,false]){app.ar=arabic;app.width=1536;app.height=980;const panel=findChild(app,"lawProtectedBackupPanel");const scroll=findChild(app,"lawReportsScroll");verify(renderedFrame());reveal(scroll,panel);verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-backup-"+(arabic?"ar":"en")+".png"))}
  app.setWorkspace(false)
 }
 function test_governmentRetentionControls(){
  app.setWorkspace(true);const law=findChild(app,"lawEnforcementWorkspace")
  const id=lawCaseBridge.createCase({caseNumber:"SYNTHETIC-RETENTION-UI",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic device",authorityReference:"Synthetic scope",issuingAuthority:"Lab owner",scope:"Synthetic retention only"});verify(id!=="",lawCaseBridge.error)
  const dev=lawCaseBridge.addDevice({model:"Synthetic fixture",custodian:"Synthetic custodian"});const e=lawCaseBridge.addEvidence(dev,syntheticEvidencePath,"Synthetic retention fixture","Synthetic custodian",true);verify(e!=="")
  law.section=11;verify(renderedFrame());compare(findChild(app,"retentionCreatePolicy").enabled,false)
  const password="Synthetic-only UI password 2026!";verify(identityBridge.login("ui-admin",password));const reviewer=identityBridge.createUser("retention-reviewer","Synthetic retention reviewer","reviewer",password);verify(reviewer!=="",identityBridge.error);verify(identityBridge.setCaseGrant(id,reviewer,true));verify(identityBridge.login("retention-reviewer",password));verify(lawCaseBridge.openCase(id));law.section=11
  findChild(app,"retentionPolicyLabel").text="Synthetic evaluation retention";findChild(app,"retentionAuthority").text="SYNTHETIC-POLICY-001"
  const create=findChild(app,"retentionCreatePolicy"),scroll=findChild(app,"lawRetentionPage");verify(create.enabled);reveal(scroll,create);mouseClick(create);compare(lawCaseBridge.activeCase.retention.policies.length,1)
  const assign=findChild(app,"retentionAssignPolicy");verify(assign.enabled);reveal(scroll,assign);mouseClick(assign);compare(lawCaseBridge.activeCase.retention.assignments.length,1)
  const archive=findChild(app,"retentionArchive");reveal(scroll,archive);mouseClick(archive);compare(lawCaseBridge.activeCase.retention.archives.length,1);verify(lawCaseBridge.verifyArchive(lawCaseBridge.activeCase.retention.archives[0].id).verified)
  findChild(app,"retentionReason").text="Synthetic court preservation exercise";findChild(app,"retentionHoldAuthority").text="SYNTHETIC-HOLD-001";const hold=findChild(app,"retentionPlaceHold");reveal(scroll,hold);mouseClick(hold);compare(lawCaseBridge.activeCase.retention.holds.length,1)
  verify(!lawCaseBridge.retentionAssessment(e).disposalEligible);compare(findChild(app,"retentionRequestDisposal").enabled,false)
  for(const arabic of [true,false]){app.ar=arabic;app.width=1366;app.height=768;reveal(scroll,findChild(app,"retentionAssess"));mouseClick(findChild(app,"retentionAssess"));scroll.contentItem.contentY=0;verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"government-retention-"+(arabic?"ar":"en")+".png"))}
  const back=findChild(app,"retentionBack");reveal(scroll,back);mouseClick(back);compare(law.section,0);verify(lawCaseBridge.verifyAudit());verify(identityBridge.login("ui-investigator",password));app.setWorkspace(false)
 }
 function test_lawCaseAndResearchWorkspaces(){
  verify(lawCaseBridge.ready,lawCaseBridge.error);verify(researchBridge.ready,researchBridge.error)
  app.navigateSection(12);compare(app.currentPage,12);verify(renderedFrame())
  mouseClick(findChild(app,"lawNewCase"));const form=findChild(app,"lawCaseForm");compare(form.step,0)
  form.form={caseNumber:"SYNTHETIC-UI-CASE",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic fixture",authorityReference:"Lab-owned synthetic data",issuingAuthority:"Lab owner",scope:"Synthetic fixture review only"}
  form.step=3;verify(renderedFrame());mouseClick(findChild(app,"lawSaveCase"));verify(!!lawCaseBridge.activeCase.id,lawCaseBridge.error)
  const id=lawCaseBridge.activeCase.id
  const device=lawCaseBridge.addDevice({model:"Synthetic UI phone",custodian:"Synthetic custodian"});verify(device!=="",lawCaseBridge.error)
  verify(lawCaseBridge.addEvidence(device,syntheticEvidencePath,"Synthetic UI artifact","Synthetic custodian",true)!=="",lawCaseBridge.error)
  const law=findChild(app,"lawEnforcementWorkspace")
  for(const arabic of [true,false]){
   app.ar=arabic;app.width=1180;app.height=760;law.section=0;verify(renderedFrame())
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"law-"+(arabic?"ar":"en")+".png"))
   for(let section=0;section<10;section++){law.section=section;verify(renderedFrame())}
  }
  lawCaseBridge.closeWorkspace();verify(lawCaseBridge.openCase(id));verify(lawCaseBridge.verifyAudit())
  app.navigateSection(9);compare(app.currentPage,13)
  const research=findChild(app,"researchCenter")
  for(const arabic of [true,false]){
   app.ar=arabic
   for(let section=0;section<9;section++){research.section=section;verify(renderedFrame())}
   research.section=2;verify(renderedFrame());verify(research.rows.length>=10)
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"research-"+(arabic?"ar":"en")+".png"))
  }
  const query=findChild(app,"researchQuery");query.text="FBE";research.section=1;mouseClick(findChild(app,"researchSearch"));verify(research.rows.length>0);query.text="";research.refresh()
  law.section=0;research.section=0;app.navigateSection(0)
 }
 function test_avbReportExportConsent(){
  app.currentPage=4;findChild(app,"firmwareTabs").currentIndex=1
  const path=findChild(app,"avbPath"),privacy=findChild(app,"avbExportPrivacy"),exportButton=findChild(app,"exportAvb")
  path.text=syntheticAvbPath;compare(privacy.checked,false);compare(exportButton.enabled,false)
  function inspectAndWait(){
   const inspectButton=findChild(app,"inspectAvb");avbInspectionFinished.clear()
   tryCompare(inspectButton,"enabled",true);verify(inspectButton.visible);verify(renderedFrame())
   artifactClick(findChild(app,"avbPanelViewport"),inspectButton);tryCompare(avbInspectionFinished,"count",1,5000)
   tryCompare(firmwareBridge,"busy",false,5000)
   compare(firmwareBridge.avbReport.status,"metadata-only",firmwareBridge.status);compare(firmwareBridge.compatible,false)
  }
  inspectAndWait()
  compare(exportButton.enabled,false);privacy.checked=true;compare(exportButton.enabled,true)
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60);verify(renderedFrame())
   const scroll=findChild(app,"avbScroll");verify(scroll.height>=100)
   for(const control of [privacy,exportButton]){
    reveal(findChild(app,"avbPanelViewport"),control);verify(renderedFrame());
    const top=control.mapToItem(app.contentItem,0,0),bottom=control.mapToItem(app.contentItem,control.width,control.height)
    verify(top.x>=0&&top.y>=0&&bottom.x<=app.width&&bottom.y<=app.height,"AVB export controls remain fully reachable")
   }
   if(dark)verify(uiEvidence.captureWindow(app,avbScreenshotBase+(arabic?"ar":"en")+".png"))
  }
  inspectAndWait();compare(privacy.checked,false)
  privacy.checked=true;firmwareBridge.invalidate();compare(firmwareBridge.avbJson,"");compare(privacy.checked,false);compare(exportButton.enabled,false)
  path.text="";findChild(app,"firmwareTabs").currentIndex=0;mouseClick(app.navigationButton(0));tryCompare(app,"currentPage",0)
 }
 function test_avbScopeAndMinimumSize(){
  findChild(app,"avbPath").text=""
  app.currentPage=4;findChild(app,"firmwareTabs").currentIndex=1
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60)
   const result=findChild(app,"avbResult");verify(result.visible);verify(result.height>=100)
   verify(findChild(app,"avbTrustNotice").visible);verify(findChild(app,"avbScope").text.indexOf("16 MiB")>=0)
   compare(findChild(app,"inspectAvb").enabled,false);compare(findChild(app,"exportAvb").enabled,false)
   const choose=findChild(app,"chooseAvb");const point=choose.mapToItem(app.contentItem,choose.width/2,choose.height/2);verify(point.y>0&&point.y<app.height)
  }
  findChild(app,"firmwareTabs").currentIndex=0
 }
 function test_fontGlyphCoverage(){
  verify(uiEvidence.hasGlyphCoverage("NEXVARYالعربية"),"Rendered English and Arabic must have real font glyphs")
  compare(uiEvidence.hasGlyphCoverage("\uDBFF\uDFFF"),false)
 }
 function test_firmwarePackageStructure(){
  app.currentPage=4;findChild(app,"firmwareTabs").currentIndex=2
  const path=findChild(app,"firmwarePackagePath");path.text=syntheticFirmwarePackagePath
  compare(findChild(app,"exportFirmwarePackage").enabled,false);artifactClick(findChild(app,"packagePanelViewport"),findChild(app,"inspectFirmwarePackage"));tryCompare(firmwareBridge,"busy",false,5000)
  compare(firmwareBridge.packageReport.status,"structure-only");compare(firmwareBridge.compatible,false)
  const output=findChild(app,"firmwarePackageResult");compare(output.textFormat,TextEdit.PlainText);verify(output.text.indexOf("snapshotSha256")>=0)
  const privacy=findChild(app,"firmwarePackagePrivacy");privacy.checked=true;compare(findChild(app,"exportFirmwarePackage").enabled,true)
  const checkImage=findChild(privacy,"neonCheckmark");verify(checkImage!==null);tryCompare(checkImage,"status",Image.Ready);compare(checkImage.visible,true);verify(checkImage.source.toString().indexOf("check_mark.svg")>=0)
  for(const arabic of [true,false])for(const dark of [true,false]){app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60);verify(findChild(app,"packageScope").height>=findChild(app,"packageScope").implicitHeight);verify(findChild(app,"firmwarePackageScroll").height>=100);if(dark)verify(uiEvidence.captureWindow(app,packageScreenshotBase+(arabic?"ar":"en")+".png"))}
  path.text="";compare(firmwareBridge.packageJson,"");compare(privacy.checked,false);compare(findChild(app,"exportFirmwarePackage").enabled,false)
  findChild(app,"firmwareTabs").currentIndex=0;mouseClick(app.navigationButton(0));tryCompare(app,"currentPage",0)
 }
 function test_workshopBarcodeCard(){
  app.currentPage=5;findChild(app,"workshopTabs").currentIndex=0;verify(renderedFrame())
  const page=findChild(app,"workshopPage"),input=findChild(app,"barcodeLookup"),lookup=findChild(app,"findBarcode")
  const id=workshopBridge.createOrder("Synthetic barcode customer","","Synthetic","Card fixture","SYNTHETIC-BC","USB","UI technician","");verify(id>0)
  page.selectedId=id;mouseClick(findChild(app,"issueDeviceCard"));const dialog=findChild(app,"deviceCardDialog");tryCompare(dialog,"opened",true)
  const code=findChild(app,"deviceCardCode").text;verify(/^NMR-[0-9A-F]{16}$/.test(code));verify(findChild(app,"deviceCardBarcode").source.toString().indexOf("data:image/png;base64,")===0)
  compare(findChild(app,"exportDeviceCard").enabled,false);findChild(app,"deviceCardPrivacy").checked=true;compare(findChild(app,"exportDeviceCard").enabled,true)
  verify(findChild(app,"deviceCardHistory").count>=2)
  const scroll=findChild(app,"deviceCardScroll"),exportButton=findChild(app,"exportDeviceCard")
  for(const arabic of [true,false])for(const dark of [true,false]){app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60);verify(dialog.width<=app.width-48);verify(findChild(app,"deviceCardBarcode").width>200);scroll.contentItem.contentY=Math.max(0,scroll.contentHeight-scroll.availableHeight);wait(30);const point=exportButton.mapToItem(scroll,exportButton.width/2,exportButton.height/2);verify(point.y>=0&&point.y<scroll.height,"Card export remains reachable");scroll.contentItem.contentY=0}
  app.darkTheme=true
  for(const arabic of [true,false]){app.ar=arabic;wait(60);verify(renderedFrame());verify(uiEvidence.captureWindow(app,workshopScreenshotBase+(arabic?"ar":"en")+".png"))}
  dialog.close();tryCompare(dialog,"visible",false);input.text=code;mouseClick(lookup);tryCompare(dialog,"opened",true);compare(page.selectedId,id);compare(findChild(app,"deviceCardCode").text,code);compare(findChild(app,"deviceCardPrivacy").checked,false)
  dialog.close();tryCompare(dialog,"visible",false);input.text="INVALID";mouseClick(lookup);verify(findChild(app,"barcodeLookupError").visible);compare(dialog.opened,false);input.clear()
  verify(renderedFrame());mouseClick(app.navigationButton(0));tryCompare(app,"currentPage",0)
 }
 function test_deviceBackgroundRefreshIsQuiet(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=0;verify(uiEvidence.setSyntheticDeviceMode("connected"));tryCompare(toolBridge,"busy",false,5000);toolBridge.selectDevice("SYNTHETIC42");toolBridge.readDeviceInfo();tryCompare(toolBridge,"busy",false,5000);verify(renderedFrame())
  const panel=findChild(app,"deviceIdentitySummary");tryVerify(function(){return panel.field("model")!==null&&panel.field("model").modelData.value==="Synthetic Phone"});const field=panel.field("model");
  const text=field.modelData.value;toolBridge.autoRefresh=true;tryCompare(toolBridge,"backgroundRefreshing",false,5000);verify(!toolBridge.foregroundBusy);verify(toolBridge.screenViewAvailable);compare(field.modelData.value,text);compare(panel.field("model"),field);verify(findChild(app,"reviewMirror").enabled);toolBridge.autoRefresh=false;toolBridge.readDeviceInfo();tryCompare(toolBridge,"busy",false,5000);verify(renderedFrame());compare(panel.field("model"),field)
  for(const arabic of [true,false]){app.ar=arabic;app.width=1366;app.height=768;findChild(app,"deviceManagerScroll").contentItem.contentY=150;verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"screen-view-ready-"+(arabic?"ar":"en")+".png"))}
  toolBridge.autoRefresh=false;verify(uiEvidence.setSyntheticDeviceMode("disconnected"));tryCompare(toolBridge,"busy",false,5000);verify(uiEvidence.clearSyntheticTool())
 }
 function test_mirrorConsentAndNavigation(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=0;verify(renderedFrame())
  verify(findChild(app,"mirrorScope").visible);compare(findChild(app,"reviewMirror").enabled,false);compare(findChild(app,"stopMirror").enabled,false)
  const dialog=findChild(app,"mirrorConfirm")
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;dialog.open();tryCompare(dialog,"opened",true)
   verify(!dialog.standardButton(Dialog.Ok).enabled);verify(findChild(app,"mirrorComponentNotice").visible)
   findChild(app,"mirrorOwner").checked=true;findChild(app,"mirrorComponent").checked=true;findChild(app,"mirrorChallenge").text="OTHER"
   verify(!dialog.standardButton(Dialog.Ok).enabled);verify(dialog.width<=app.width-48);dialog.reject();tryCompare(dialog,"opened",false);tryCompare(dialog,"visible",false)
  }
  verify(renderedFrame());mouseClick(app.navigationButton(0));tryCompare(app,"currentPage",0)
 }
 function test_modernCompatibilityPanel(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1
  const panel=findChild(app,"deviceCompatibilityPanel"),scroll=findChild(app,"deviceServiceDetailsScroll")
  compare(panel.visible,false);panel.profile=syntheticModernCompatibility
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;scroll.contentItem.contentY=0;wait(60)
   verify(panel.visible);verify(panel.width>180);verify(panel.height>=panel.implicitHeight)
   verify(findChild(app,"compatibilityPlatform").text.indexOf("37")>=0)
   verify(findChild(app,"compatibilityPageSize").text.indexOf("16384")>=0)
   const evidence=findChild(app,"compatibilityEvidence");verify(evidence.height>=evidence.implicitHeight)
   const point=evidence.mapToItem(app.contentItem,0,0);verify(point.y>0&&point.y+evidence.height<app.height)
  }
  app.darkTheme=true
  for(const arabic of [true,false]){app.ar=arabic;wait(60);verify(renderedFrame());verify(uiEvidence.captureWindow(app,modernScreenshotBase+(arabic?"ar":"en")+".png"))}
  panel.profile={schemaVersion:1,transport:"fastboot",layout:{fastbootMode:"userspace-fastbootd"}}
  verify(findChild(app,"compatibilityFastbootMode").visible);verify(findChild(app,"compatibilityFastbootMode").text.indexOf("fastbootd")>=0)
  panel.profile=Qt.binding(function(){return toolBridge.serviceProfile.compatibility||({})})
  compare(panel.visible,false)
 }
 function test_compiledBuildProfileIsVisible(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1;verify(renderedFrame())
  const label=findChild(app,"serviceBuildProfile");verify(label.visible)
  app.ar=false;verify(label.text.indexOf(toolBridge.laboratoryBuild?"Laboratory":"read-only")>=0)
  verify(findChild(app,"serviceWorkflow").text.indexOf("Discover")>=0)
 }
 function test_deviceServiceNavigationAndConsent(){
  app.currentPage=1;const tabs=findChild(app,"deviceTabs");tabs.currentIndex=1;verify(renderedFrame());const page=findChild(app,"deviceServicesPage");verify(page!==null)
  compare(findChild(app,"serviceBrand").count,18);compare(findChild(app,"serviceDevice").currentIndex,-1);verify(!findChild(app,"exportDeviceServiceReport").enabled)
  findChild(app,"serviceBrand").currentIndex=1;verify(!findChild(app,"serviceOfficialDrivers").enabled);verify(findChild(app,"serviceDriverUnavailableReason").visible);findChild(app,"serviceBrand").currentIndex=0
  mouseClick(findChild(app,"deviceServiceContextOpen"));const context=findChild(app,"deviceServiceContextDialog");tryCompare(context,"opened",true);
  verify(findChild(app,"deviceServiceContextText").text.indexOf(findChild(app,"serviceWorkflow").text)>=0);context.close();tryCompare(context,"opened",false);
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60)
   const list=findChild(app,"deviceServiceList");verify(list.width>240&&list.height>150,"service list "+list.width+"x"+list.height+" page "+page.width+"x"+page.height+" ar="+arabic)
   page.review({id:"sideload_zip",ar:"اختبار موافقة اصطناعي",en:"Synthetic consent test",risk:"destructive"})
   const dialog=findChild(app,"deviceServiceConfirm");tryCompare(dialog,"opened",true);verify(!dialog.standardButton(Dialog.Ok).enabled)
   findChild(app,"serviceOwnerConsent").checked=true;verify(!dialog.standardButton(Dialog.Ok).enabled)
   findChild(app,"serviceDataLossConsent").checked=true;verify(!dialog.standardButton(Dialog.Ok).enabled)
   dialog.reject();tryCompare(dialog,"opened",false)
  }
  tabs.currentIndex=2;verify(renderedFrame());const rows=findChild(app,"deviceCatalogList");compare(rows.count,48)
  const search=findChild(app,"deviceCatalogSearch");search.text="SM-A057G";tryCompare(rows,"count",1);search.text="nothing matches";tryCompare(rows,"count",0);search.text=""
  mouseClick(findChild(app,"importDeviceCatalog"));const dialog=findChild(app,"deviceCatalogDialog");tryCompare(dialog,"opened",true);verify(!dialog.standardButton(Dialog.Ok).enabled);dialog.reject()
  mouseClick(app.navigationButton(0));compare(app.currentPage,0)
 }
 function cleanup(){
  if(recoveryBridge.active&&!recoveryBridge.busy)recoveryBridge.closeCase()
  for(const name of ["deviceCardDialog","deviceServiceHistoryDetail","deviceServiceConfirm","mirrorConfirm","exitDialog"]){const dialog=findChild(app,name);if(dialog&&dialog.visible)dialog.close()}
  const panel=findChild(app,"deviceCompatibilityPanel");if(panel)panel.profile=Qt.binding(function(){return toolBridge.serviceProfile.compatibility||({})})
  const details=findChild(app,"deviceServiceDetailsScroll");if(details)details.Layout.maximumHeight=Number.POSITIVE_INFINITY
  app.showDevicePanel=false;app.width=1536;app.height=980
 }
 function test_serviceHistoryDetailsArePlainText(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1;verify(renderedFrame())
  const page=findChild(app,"deviceServicesPage")
  page.reviewRecord({serial:"SYNTHETIC-42",transport:"adb",action:"dark_on",atUtc:"2026-10-18T00:00:00Z",result:"<b>Permission denied</b>"})
  const dialog=findChild(app,"deviceServiceHistoryDetail");tryCompare(dialog,"opened",true)
  const output=findChild(app,"deviceServiceHistoryText");verify(output.text.indexOf("SYNTHETIC-42")>=0);verify(output.text.indexOf("<b>Permission denied</b>")>=0);compare(output.textFormat,TextEdit.PlainText)
  dialog.close();tryCompare(dialog,"opened",false)
 }
 function test_serviceMinimumSizeOutputAndHistory(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1;findChild(app,"serviceBrand").currentIndex=1
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(80)
   const result=findChild(app,"deviceServiceResultPanel");verify(result.height>=180,"Operation result must remain readable at minimum size")
   const details=findChild(app,"deviceServiceDetailsScroll");verify(details!==null)
   const originalMaximum=details.Layout.maximumHeight
   for(const maximum of [100,originalMaximum]){
   details.Layout.maximumHeight=maximum;wait(40);if(maximum===100)compare(details.height,100);details.contentItem.contentY=0;wait(30)
   let point=result.mapToItem(details,result.width/2,80);verify(point.y>=0&&point.y<details.height,"Result remains visible at the top")
   details.contentItem.contentY=Math.max(0,details.contentHeight-details.availableHeight);wait(30)
   const report=findChild(app,"exportDeviceServiceReport");point=report.mapToItem(details,report.width/2,report.height/2);verify(point.y>=0&&point.y<details.height,"Report remains reachable by scrolling: "+JSON.stringify({y:point.y,view:details.height,available:details.availableHeight,content:details.contentHeight,offset:details.contentItem.contentY,origin:details.contentItem.originY}))
   verify(point.y+report.height/2<=details.height,"Entire report control remains inside the scrolled viewport")
   if(dark){verify(uiEvidence.captureWindow(app,serviceScreenshotBase+(arabic?"ar":"en")+(maximum===100?"-100px":"")+".png"))}
   details.contentItem.contentY=0
   }
   details.Layout.maximumHeight=originalMaximum
  }
  findChild(app,"serviceBrand").currentIndex=0
 }
 function test_centerProfilesAndTracking(){
  const id=workshopBridge.createOrder("SYNTHETIC UI owner","test-contact","OPPO","TRAINING-01","test-serial","USB","Tech","private");verify(id>0)
  const order=workshopBridge.invoice(id)
  const second=workshopBridge.createOrderForDevice(order.device_id,"Screen","Tech","second visit");verify(second>id)
  compare(workshopBridge.deviceProfile(order.device_id).orders.length,2)
  app.currentPage=5;const tabs=findChild(app,"workshopTabs");tabs.currentIndex=1;verify(renderedFrame())
  verify(findChild(app,"customerProfiles").count>0);verify(findChild(app,"deviceProfiles").count>0)
  tabs.currentIndex=2;verify(renderedFrame());verify(!findChild(app,"startPortal").enabled);verify(!findChild(app,"issueTracking").enabled)
  verify(trackingPortal.start("127.0.0.1",0,true));verify(renderedFrame());verify(findChild(app,"stopPortal").enabled)
  trackingPortal.stop();verify(!trackingPortal.running)
 }
 function test_securityAndFaultTree(){
  const id=workshopBridge.createOrder("Synthetic tree owner","","Samsung","SYNTHETIC-02","","USB","Tech","");verify(id>0)
  app.currentPage=2;const tabs=findChild(app,"diagnosticTabs");tabs.currentIndex=2;verify(renderedFrame())
  verify(!findChild(app,"startSecurityScan").enabled);verify(!findChild(app,"updateSignatures").enabled);verify(!findChild(app,"cancelSignatureUpdate").enabled);compare(findChild(app,"securityVerdict").text,app.trx("لم يُجرَ فحص بعد","No scan performed yet"))
  tabs.currentIndex=3;verify(renderedFrame());compare(serviceCatalog.drivers.length,7);verify(serviceCatalog.drivers.some(function(d){return d.brand==="Google"&&d.url.indexOf("https://developer.android.com/")===0}))
  tabs.currentIndex=1;verify(renderedFrame());const choice=findChild(app,"faultTreeOrder");verify(choice.count>0)
  const selected=choice.currentValue;artifactClick(findChild(app,"faultTreePage"),findChild(app,"startFaultTree"));verify(findChild(app,"faultTreeQuestion").text.length>5)
  artifactClick(findChild(app,"faultTreePage"),findChild(app,"faultTreeYes"));verify(!findChild(app,"faultTreeYes").enabled)
  findChild(app,"treeTechnician").text="Synthetic technician";findChild(app,"treeFinding").text="Synthetic safety observation only";
  const save=findChild(app,"saveFaultTree");verify(save.enabled)
  // The save control is below the first viewport: scroll it into view before clicking.
  const scroll=findChild(app,"faultTreePage");artifactClick(scroll,save)
  verify(workshopBridge.diagnoses(selected).length>0)
 }
 function test_cloudTrackingConsent(){
  app.currentPage=5;findChild(app,"workshopTabs").currentIndex=2;verify(renderedFrame())
  const approval=findChild(app,"cloudUploadConsent"),load=findChild(app,"importCloudCredentials")
  approval.checked=false;compare(load.enabled,false)
  compare(findChild(app,"syncCloudNow").enabled,false);compare(findChild(app,"issueCloudLink").enabled,false);compare(findChild(app,"copyCloudLink").enabled,false)
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;approval.checked=true;tryCompare(load,"enabled",true)
   approval.checked=false;tryCompare(load,"enabled",false)
  }
 }
 function test_newWorkspaceMinimumSize(){
  app.width=1180;app.height=760
  const checks=[{page:2,tab:1,bar:"diagnosticTabs",scroll:"faultTreePage",target:"importModelGuide"},{page:2,tab:2,bar:"diagnosticTabs",scroll:"securityPage",target:"exportSecurityReport"},{page:5,tab:1,bar:"workshopTabs",scroll:"profilesScroll",target:"repeatRepair"},{page:5,tab:2,bar:"workshopTabs",scroll:"trackingScroll",target:"publicTrackingMessage"}]
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark
   for(const c of checks){app.currentPage=c.page;findChild(app,c.bar).currentIndex=c.tab;wait(50);const scroll=findChild(app,c.scroll);const target=findChild(app,c.target);verify(target!==null);verify(scroll.availableHeight>0);let point=target.mapToItem(scroll.contentItem,0,0);scroll.contentItem.contentY=Math.max(0,scroll.contentItem.contentY+point.y-scroll.availableHeight+target.height+20);wait(30);let center=target.mapToItem(scroll,target.width/2,target.height/2);verify(center.y>=0&&center.y<scroll.height,"Bottom action remains reachable: "+c.target);scroll.contentItem.contentY=0}
  }
 }
 function test_minimumSize(){
  app.width=1180;app.height=760;app.currentPage=7;app.showDevicePanel=true;wait(80);let scroll=findChild(app,"settingsScroll");verify(scroll!==null);verify(scroll.contentHeight>scroll.availableHeight,"Settings must scroll at minimum height");
  for(let i=0;i<10;i++){let b=app.navigationButton(i);verify(b.width>=40,"Collapsed navigation keeps a usable hit target");let p=b.mapToItem(app.contentItem,0,0);verify(p.x>=0);verify(p.x+b.width<=app.width+1)}
  let deviceScroll=findChild(app,"deviceScroll");verify(deviceScroll!==null);verify(deviceScroll.availableHeight>0);
  deviceScroll.contentItem.contentY=Math.max(0,deviceScroll.contentHeight-deviceScroll.availableHeight);
  // Content can fit without scrolling, so no new render frame is required.
  let backup=findChild(app,"quickBackup");verify(backup!==null);
  tryVerify(function(){let point=backup.mapToItem(deviceScroll,backup.width/2,backup.height/2);return point.x>=0&&point.x<deviceScroll.width&&point.y>=0&&point.y<deviceScroll.height},5000,"Quick backup action remains reachable");deviceScroll.contentItem.contentY=0;
  app.showDevicePanel=false;app.width=1536;app.height=980;
 }
 function reveal(scroll,target){
  const point=target.mapToItem(scroll.contentItem,0,0)
  scroll.contentItem.contentY=Math.max(0,Math.min(scroll.contentHeight-scroll.availableHeight,scroll.contentItem.contentY+point.y-target.height))
  wait(30)
 }
 function test_deviceIntelligenceProvenance(){
  app.navigateSection(1);app.ar=false
  verify(uiEvidence.setSyntheticDeviceMode("connected"));tryCompare(toolBridge,"busy",false,5000)
  toolBridge.selectDevice("SYNTHETIC42")
  compare(toolBridge.deviceIntelligence.reportedFieldCount,0)
  const scroll=findChild(app,"deviceManagerScroll"),read=findChild(app,"readDeviceInfo")
  reveal(scroll,read);verify(read.enabled);mouseClick(read);tryCompare(toolBridge,"busy",false,5000)
  const panel=findChild(app,"deviceIdentitySummary")
  tryVerify(function(){return panel.field("model")!==null&&panel.field("model").modelData.value==="Synthetic Phone"})
  compare(findChild(panel.field("model"),"intelligenceValue-model").text,"Synthetic Phone")
  compare(panel.field("model").modelData.source,"adb:shell getprop")
  verify(panel.field("model").modelData.observedAtUtc.length>0)
  compare(toolBridge.deviceIntelligence.attestationVerified,false)
  compare(toolBridge.deviceIntelligence.health.battery.state,"unavailable")
  for(const arabic of [true,false]){
   app.ar=arabic;app.width=1366;app.height=768;scroll.contentItem.contentY=panel.y
   verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"device-intelligence-"+(arabic?"ar":"en")+".png"))
  }
  const review=findChild(app,"intelligenceReviewCapabilities");reveal(scroll,review);verify(review.enabled);mouseClick(review)
  compare(app.currentPage,1);compare(findChild(app,"deviceTabs").currentIndex,1)
  verify(uiEvidence.setSyntheticDeviceMode("disconnected"));tryCompare(toolBridge,"busy",false,5000);verify(uiEvidence.clearSyntheticTool())
  compare(toolBridge.deviceIntelligence.reportedFieldCount,0);findChild(app,"deviceTabs").currentIndex=0;scroll.contentItem.contentY=0
 }
 function test_usbDoctorInterpretation(){
  app.navigateSection(4);app.ar=false
  const doctor=findChild(app,"usbDoctorPanel");verify(doctor!==null);verify(doctor.visible)
  verify(findChild(app,"doctorReadUsb").enabled)
  verify(uiEvidence.setSyntheticDeviceMode("unauthorized"));tryCompare(toolBridge,"busy",false,5000)
  toolBridge.selectDevice("SYNTHETIC42")
  tryVerify(function(){return doctor.finding("adb-unauthorized")!==null},5000,JSON.stringify(doctor.report))
  verify(findChild(app,"doctorScope").text.indexOf("not USB descriptor reads")>=0)
  for(const arabic of [true,false]){
   app.ar=arabic;app.width=1366;app.height=768;findChild(app,"driverCatalogPage").contentItem.contentY=0
   verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"usb-doctor-"+(arabic?"ar":"en")+".png"))
  }
  verify(uiEvidence.setSyntheticDeviceMode("disconnected"));tryCompare(toolBridge,"busy",false,5000)
  verify(uiEvidence.clearSyntheticTool());app.ar=false
  mouseClick(findChild(app,"doctorReadUsb"));tryCompare(toolBridge,"busy",false,35000)
  const source=toolBridge.connectionDiagnosis.sources.filter(function(s){return s.source==="windows-pnp"})[0]
  verify(source.state==="ok"||source.state==="windows-only","Read-only inventory: "+toolBridge.status)
  compare(toolBridge.connectionDiagnosis.physicalCauseVerified,false)
  compare(toolBridge.connectionDiagnosis.interfaceBindingVerified,false)
 }
 function test_navigation(){
  for(let i=0;i<app.sections.length;i++){
   const button=app.sectionButton(i);verify(button!==null)
   reveal(findChild(app,"sidebarScroll"),button);mouseClick(button)
   tryCompare(app,"activeSection",i);compare(app.currentPage,app.sections[i].page)
  }
 }
 function test_shellResponsive(){
  for(const size of [[1366,768],[1920,1080],[2560,1440]])for(const arabic of [true,false]){
   app.width=size[0];app.height=size[1];app.ar=arabic
   for(let i=0;i<app.sections.length;i++){
    const button=app.sectionButton(i);reveal(findChild(app,"sidebarScroll"),button);mouseClick(button)
    tryCompare(app,"activeSection",i);compare(app.currentPage,app.sections[i].page);verify(renderedFrame())
    for(const name of ["workstationSidebar","workstationWorkspace","languageButton","topSettings","topHelp"]){
     const item=findChild(app,name),top=item.mapToItem(app.contentItem,0,0)
     verify(item.visible&&item.width>0&&item.height>0,name+" visible")
     verify(top.x>=-1&&top.y>=-1&&top.x+item.width<=app.width+1&&top.y+item.height<=app.height+1,name+" within viewport at "+size)
    }
    verify(uiEvidence.captureWindow(app,uiScreenshotBase+uiScaleTag+"-"+size[0]+"-"+(arabic?"ar":"en")+"-"+i+".png"))
   }
  }
 }
 function test_shellRoutesAndCollapse(){
  app.navigateSection(0);verify(renderedFrame());mouseClick(findChild(app,"dashboardDevices"));compare(app.currentPage,1)
  app.navigateSection(8);verify(renderedFrame());mouseClick(findChild(app,"reportEvidence"));compare(app.currentPage,9);compare(findChild(app,"forensicTabs").currentIndex,1)
  app.goBack();compare(app.activeSection,8)
  const side=findChild(app,"workstationSidebar"),before=side.collapsed
  mouseClick(findChild(app,"collapseSidebar"));compare(side.collapsed,!before)
  app.navigateSection(1);compare(app.currentPage,1);mouseClick(findChild(app,"collapseSidebar"));compare(side.collapsed,before)
 }
 function test_shellDeviceStates(){
  app.navigateSection(1);app.width=1366;app.height=768;app.ar=false
  const page=findChild(app,"deviceManagerPage"),scroll=findChild(app,"deviceManagerScroll"),read=findChild(app,"readDeviceInfo")
  tryCompare(toolBridge,"busy",false);verify(uiEvidence.setSyntheticDeviceMode("connected"));tryCompare(toolBridge,"busy",false,5000);compare(toolBridge.devices.length,1)
  const list=findChild(app,"deviceSelectionList");verify(renderedFrame());mouseClick(list.itemAtIndex(0));compare(toolBridge.selectedSerial,"SYNTHETIC42");compare(page.adbReady,true);compare(findChild(app,"connectionBadge").text,"ADB authorized")
  reveal(scroll,read);verify(read.enabled);mouseClick(read);tryCompare(toolBridge,"busy",false,5000);compare(toolBridge.deviceInfo["ro.product.model"],"Synthetic Phone")
  scroll.contentItem.contentY=0;verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"device-connected-en.png"))
  for(const mode of ["unauthorized","offline"]){
   verify(uiEvidence.setSyntheticDeviceMode(mode));tryCompare(toolBridge,"busy",false,5000);compare(toolBridge.devices[0].state,mode)
   toolBridge.selectDevice("SYNTHETIC42");compare(page.adbReady,false);compare(findChild(app,"connectionBadge").text,"ADB "+mode);compare(read.enabled,false);verify(page.selectionReason.length>0)
   verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"device-"+mode+"-en.png"))
  }
  verify(uiEvidence.setSyntheticDeviceMode("progress"));compare(toolBridge.busy,true);verify(renderedFrame());verify(findChild(app,"cancelActiveOperation").visible)
  verify(uiEvidence.captureWindow(app,uiScreenshotBase+"device-progress-en.png"));mouseClick(findChild(app,"cancelActiveOperation"));tryCompare(toolBridge,"busy",false,5000)
  verify(uiEvidence.setSyntheticDeviceMode("error"));tryCompare(toolBridge,"busy",false,5000);verify(findChild(app,"operationOutcome").text.indexOf("FAILURE")>=0)
  verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"device-error-en.png"))
  verify(uiEvidence.setSyntheticDeviceMode("disconnected"));tryCompare(toolBridge,"busy",false,5000);compare(toolBridge.devices.length,0);compare(toolBridge.selectedSerial,"");verify(uiEvidence.clearSyntheticTool());scroll.contentItem.contentY=0
 }

 function test_forensicLabNavigation(){
  app.currentPage=0;
  let nav=app.navigationButton(9);
  verify(nav!==null,"Forensic Lab navigation missing");
  mouseClick(nav);
  compare(app.currentPage,9);
  let lab=findChild(app,"forensicLabWorkspace");
  verify(lab!==null,"Forensic Lab workspace missing");
  compare(lab.modules.length,14);
  compare(lab.modules.filter(function(m){return m.ready}).length,11);
  verify(renderedFrame());
  app.ar=false;
  verify(renderedFrame());
  compare(app.titles[9],"Forensic Lab");
  app.ar=true;
  verify(renderedFrame());
  compare(app.titles[9],"مختبر الأدلة الرقمية");
  mouseClick(app.navigationButton(0));
  compare(app.currentPage,0);
 }
 function test_forensicCaptureWorkflow(){
  app.currentPage=9;const tabs=findChild(app,"forensicTabs");tabs.currentIndex=3
  const page=findChild(app,"forensicToolsPage"),run=findChild(app,"runPcapAnalysis"),consent=findChild(app,"pcapConsent")
  findChild(app,"forensicToolsScroll").contentItem.contentY=0
  page.capturePath="";consent.checked=false;compare(run.enabled,false)
  verify(recoveryBridge.createCase("Synthetic PCAP UI","Fixture consent","Synthetic","No phone","","backup","none",true))
  page.capturePath=syntheticCapturePath;consent.checked=true;compare(run.enabled,false)
  verify(recoveryBridge.recordOperator("UI operator","Synthetic capture authorization",true));tryCompare(run,"enabled",true)
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(50)
   const heading=findChild(app,"forensicHeading");verify(heading.height>=heading.implicitHeight)
   const scope=findChild(app,"pcapScope");verify(scope.visible);verify(scope.text.indexOf("8 MiB")>=0)
   const point=run.mapToItem(app.contentItem,run.width/2,run.height/2);verify(point.y>0&&point.y<app.height)
  }
  artifactClick(findChild(app,"forensicToolsScroll"),run);tryCompare(recoveryBridge,"busy",false);compare(recoveryBridge.error,"");compare(recoveryBridge.evidence.length,1)
  verify(recoveryBridge.previewEvidence(0).indexOf("nexvary-pcap-summary/v1")>=0)
  const show=findChild(app,"showPcapResult");verify(show.enabled);mouseClick(show);compare(tabs.currentIndex,1)
  verify(findChild(app,"forensicPreview").text.indexOf("nexvary-pcap-summary/v1")>=0);tabs.currentIndex=3
  app.toastText="";app.darkTheme=true
  for(const arabic of [true,false]){
   app.ar=arabic;wait(60);verify(renderedFrame())
   verify(uiEvidence.captureWindow(app,pcapScreenshotBase+(arabic?"ar":"en")+".png"))
  }
  tabs.currentIndex=1;const kind=findChild(app,"forensicKind");kind.currentIndex=kind.count-1;tryCompare(findChild(app,"forensicEvidenceList"),"count",1);kind.currentIndex=0;tabs.currentIndex=3
  page.capturePath="";compare(consent.checked,false);compare(run.enabled,false)
  verify(recoveryBridge.closeCase());tabs.currentIndex=0
 }
 function test_forensicExifWorkflow(){
  app.currentPage=9;const tabs=findChild(app,"forensicTabs");tabs.currentIndex=3
  const page=findChild(app,"forensicToolsPage"),run=findChild(app,"runExifAnalysis"),consent=findChild(app,"exifConsent"),scroll=findChild(app,"forensicToolsScroll")
  page.exifPath="";consent.checked=false;compare(run.enabled,false)
  verify(recoveryBridge.createCase("Synthetic EXIF UI","Fixture consent","Synthetic","No phone","","backup","none",true))
  page.exifPath=syntheticExifPath;consent.checked=true;compare(run.enabled,false)
  verify(recoveryBridge.recordOperator("UI operator","Synthetic photo authorization",true));tryCompare(run,"enabled",true)
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(50)
   scroll.contentItem.contentY=0
   const panel=findChild(app,"exifPanel");scroll.contentItem.contentY=panel.y;wait(50);verify(renderedFrame())
   const scope=findChild(app,"exifScope");verify(scope.visible);verify(scope.height+0.01>=scope.implicitHeight,"EXIF scope height "+scope.height+" implicit "+scope.implicitHeight);verify(scope.text.indexOf("8 MiB")>=0)
   reveal(scroll,run);verify(renderedFrame());
   const top=run.mapToItem(app.contentItem,0,0),bottom=run.mapToItem(app.contentItem,run.width,run.height)
   verify(top.x>=0&&top.y>=0&&bottom.x<=app.width&&bottom.y<app.height,"EXIF action remains reachable")
   if(dark)verify(uiEvidence.captureWindow(app,exifScreenshotBase+(arabic?"ar":"en")+".png"))
  }
  artifactClick(findChild(app,"forensicToolsScroll"),run);tryCompare(recoveryBridge,"busy",false);compare(recoveryBridge.error,"");compare(recoveryBridge.evidence.length,1)
  verify(recoveryBridge.previewEvidence(0).indexOf("nexvary-exif-summary/v1")>=0)
  const show=findChild(app,"showExifResult");verify(show.enabled);mouseClick(show);compare(tabs.currentIndex,1)
  verify(findChild(app,"forensicPreview").text.indexOf("nexvary-exif-summary/v1")>=0)
  tabs.currentIndex=3;page.exifPath="";compare(consent.checked,false);compare(run.enabled,false)
  verify(recoveryBridge.closeCase());scroll.contentItem.contentY=0;tabs.currentIndex=0
 }
 function test_forensicWorkspace(){
  app.currentPage=9;
  let tabs=findChild(app,"forensicTabs");verify(tabs!==null);tabs.currentIndex=1;
  verify(recoveryBridge.createCase("Synthetic lab UI","LAB-ONLY","Synthetic","No phone","15","backup","unknown",true));
  findChild(app,"forensicOperator").text="UI operator";findChild(app,"forensicAuthorization").text="LAB authorization";
  let record=findChild(app,"forensicRecordOperator");compare(record.enabled,false);
  findChild(app,"forensicOperatorConsent").checked=true;tryCompare(record,"enabled",true);
  artifactClick(findChild(app,"forensicDetailScroll"),record);compare(recoveryBridge.caseOperator,"UI operator");
  verify(recoveryBridge.importVisibleFile(syntheticEvidencePath,true));tryCompare(recoveryBridge,"busy",false);
  let files=findChild(app,"forensicEvidenceList");tryCompare(files,"count",1);
  mouseClick(files,files.width/2,40);let preview=findChild(app,"forensicPreview");tryVerify(function(){return preview.text.indexOf("SYNTHETIC UI EVIDENCE")>=0});
  let search=findChild(app,"forensicSearch");search.text="no matching file";tryCompare(files,"count",0);search.text="synthetic";tryCompare(files,"count",1);search.text="";
  let report=findChild(app,"forensicReport");compare(report.enabled,false);findChild(app,"forensicPlaintext").checked=true;tryCompare(report,"enabled",true);findChild(app,"forensicPlaintext").checked=false;
  mouseClick(findChild(app,"forensicVerify"));tryCompare(recoveryBridge,"busy",false);compare(recoveryBridge.error,"");
  let custodyButton=findChild(app,"forensicCustody");let detail=findChild(app,"forensicDetailScroll");
  let point=custodyButton.mapToItem(detail,custodyButton.width/2,custodyButton.height/2);detail.contentItem.contentY+=point.y-detail.height/2;
  verify(renderedFrame());mouseClick(custodyButton);let dialog=findChild(app,"forensicCustodyDialog");tryCompare(dialog,"opened",true);compare(dialog.standardButton(Dialog.Ok).enabled,false);
  findChild(app,"forensicCustodyRecipient").text="Lab storage";findChild(app,"forensicCustodyReason").text="Synthetic review";
  tryCompare(dialog.standardButton(Dialog.Ok),"enabled",true);mouseClick(dialog.standardButton(Dialog.Ok));tryCompare(dialog,"opened",false);compare(recoveryBridge.evidenceCustodian(0),"Lab storage");
  tabs.currentIndex=2;let events=findChild(app,"forensicTimelineList");tryVerify(function(){return events.count>=5});findChild(app,"forensicTimelineSearch").text="custody-transfer";tryCompare(events,"count",1);findChild(app,"forensicTimelineSearch").text="";
  for(let language=0;language<2;language++){app.ar=language===0;for(let i=0;i<tabs.count;i++){tabs.currentIndex=i;verify(renderedFrame())}}
  tabs.currentIndex=3;compare(findChild(app,"runTskAnalysis").enabled,false);compare(toolBridge.forensicTools.length,10);
  verify(recoveryBridge.closeCase());tabs.currentIndex=0;
 }
 function test_backNavigation(){
  app.pageHistory=[];app.previousPage=0;app.currentPage=0;
  mouseClick(app.navigationButton(9));compare(app.currentPage,9);let back=findChild(app,"backButton");compare(back.enabled,true);mouseClick(back);compare(app.currentPage,0);compare(app.visible,true);
 }
 function test_themeAndNarrowForensicTabs(){
  app.width=1180;app.height=760;app.showDevicePanel=false;app.currentPage=9;
  let theme=findChild(app,"themeButton"),tabs=findChild(app,"forensicTabs");
  for(let language=0;language<2;language++){
   app.ar=language===0;
   for(let mode=0;mode<2;mode++){
    mouseClick(theme);compare(app.darkTheme,mode===1);
    for(let tab=0;tab<tabs.count;tab++){tabs.currentIndex=tab;verify(renderedFrame())}
   }
  }
  tabs.currentIndex=0;app.darkTheme=true;app.width=1536;app.height=980;
 }
 function test_language(){let b=findChild(app,"languageButton");mouseClick(b);compare(app.ar,false);mouseClick(b);compare(app.ar,true)}
 function test_boardMeasurement(){
  app.currentPage=3;verify(renderedFrame());mouseClick(findChild(app,"loadTrainingBoard"));tryVerify(function(){return boardBridge.loaded});verify(boardBridge.components.length>0);verify(renderedFrame());
  let input=findChild(app,"boardVoltage");input.text="3.30";let before=boardBridge.measurements.length;
  let record=findChild(app,"recordMeasurement");verify(record.enabled);verify(renderedFrame());
  const inspector=findChild(app,"boardInspector");verify(inspector!==null);
  let center=record.mapToItem(inspector,record.width/2,record.height/2);
  if(center.y<0||center.y>=inspector.height){inspector.contentItem.contentY+=center.y-inspector.height/2;verify(renderedFrame())}
  tryVerify(function(){let p=record.mapToItem(inspector,record.width/2,record.height/2);return p.y>=0&&p.y<inspector.height});
  mouseClick(record);tryVerify(function(){return boardBridge.measurements.length===before+1});verify(boardBridge.dirty);
 }
 function test_createOrder(){
  app.currentPage=5;let before=workshopBridge.orders.length;mouseClick(findChild(app,"newOrder"));let dialog=findChild(app,"newOrderDialog");tryCompare(dialog,"opened",true);
  mouseClick(findChild(app,"saveIntakeButton"));compare(workshopBridge.orders.length,before);compare(dialog.opened,true);verify(findChild(app,"intakeError").visible);
  findChild(app,"customerField").text="UI test customer";findChild(app,"brandField").text="TestBrand";findChild(app,"modelField").text="Model-01";findChild(app,"symptomField").text="USB not detected";
  mouseClick(findChild(app,"saveIntakeButton"));compare(workshopBridge.orders.length,before+1);tryCompare(dialog,"visible",false);
 }
 function test_diagnosis(){app.currentPage=2;let b=findChild(app,"diagnoseButton");mouseClick(b);let r=findChild(app,"diagnosisResult");verify(r.text.length>40);verify(r.readOnly)}
 function test_savedDiagnosis(){
  const id=workshopBridge.createOrder("Synthetic diagnosis UI","","TestBrand","Model-02","","USB not detected","Test technician","");verify(id>0);
  app.currentPage=2;verify(renderedFrame());
  let choice=findChild(app,"diagnosisOrder");tryVerify(function(){return choice.count>1});choice.currentIndex=choice.indexOfValue(id);
  findChild(app,"diagnosisTechnician").text="UI technician";findChild(app,"diagnosisFindings").text="Synthetic observation only";findChild(app,"diagnosisEvidence").text="No phone connected; UI test";
  let analyze=findChild(app,"diagnoseButton");mouseClick(analyze);let save=findChild(app,"saveDiagnosis");tryCompare(save,"enabled",true);
  let scroll=findChild(app,"diagnosisScroll");verify(scroll!==null);let center=save.mapToItem(scroll,save.width/2,save.height/2);scroll.contentItem.contentY+=center.y-scroll.height/2;
  verify(renderedFrame());mouseClick(save);tryVerify(function(){return workshopBridge.diagnoses(id).length===1});
  compare(workshopBridge.diagnoses(id)[0].findings,"Synthetic observation only");
  findChild(app,"symptomChoice").currentIndex=1;compare(save.enabled,false);scroll.contentItem.contentY=0;
 }
 function test_recoveryConsent(){
  app.currentPage=8;
  let result=findChild(app,"recoveryVerdict");
  verify(result!==null);
  compare(result.code,"not_supported");
  let reason=findChild(app,"recoveryReason");
  verify(reason.text.indexOf(app.ar?"موافقة المالك":"owner authorization")>=0);
  verify(result.text!==result.code);
  let consent=findChild(app,"recoveryConsent");
  verify(consent!==null);
  consent.checked=true;
  tryCompare(result,"code","need_more_information");
  let source=findChild(app,"recoverySource");
  source.currentIndex=3;
  app.currentPage=0;
  app.currentPage=8;
  let reset=findChild(app,"recoveryReset");
  reset.checked=true;
  compare(result.code,"not_supported");
  verify(reason.text.indexOf(app.ar?"التخزين الداخلي":"Internal storage")>=0);
  reset.checked=false;
  source.currentIndex=0;findChild(app,"recoveryReadable").currentIndex=2;
  compare(result.code,"not_supported");
  verify(reason.text.indexOf(app.ar?"البطاقة غير قابلة":"card was marked unreadable")>=0);
  findChild(app,"recoveryReadable").currentIndex=0;
  consent.checked=false;
 }
 function test_recoveryWizard(){
  app.currentPage=8;findChild(app,"recoveryTabs").currentIndex=1;let page=findChild(app,"recoveryWorkspace");verify(page!==null);
  for(let i=0;i<8;i++){page.step=i;verify(renderedFrame())}
  page.step=1;findChild(app,"recoveryBrand").text="Synthetic";findChild(app,"recoveryModel").text="UI fixture";
  page.step=4;findChild(app,"recoveryTitle").text="Synthetic UI case";findChild(app,"recoveryOwner").text="Test-only owner reference";
  let create=findChild(app,"createRecoveryCase");compare(create.enabled,false);verify(renderedFrame());mouseClick(findChild(app,"caseOwnerConsent"));tryCompare(create,"enabled",true);mouseClick(create);tryCompare(recoveryBridge,"active",true);compare(page.step,5);
  compare(findChild(app,"runRecovery").enabled,false);page.step=4;verify(renderedFrame());mouseClick(findChild(app,"closeRecoveryCase"));tryCompare(recoveryBridge,"active",false);findChild(app,"recoveryTabs").currentIndex=0;
 }
 function test_exitCancel(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);let no=dialog.standardButton(Dialog.No);verify(no);mouseClick(no);tryCompare(dialog,"visible",false);compare(app.visible,true)}
 function test_osintDefaultShowsServiceCards(){
  verify(identityBridge.login("ui-admin","Synthetic-only UI password 2026!"))
  app.navigatePrimary(3);verify(renderedFrame());
  const ws=findChild(app,"osintWorkspace");
  compare(ws.demoMode,false);
  verify(findChild(ws,"intelligenceDashboard")!==null);
  compare(findChild(ws,"intelligenceRefreshCases").enabled,false);

  for(const ar of [true,false]){
   app.ar=ar;
   verify(renderedFrame());
   verify(findChild(ws,"intelligenceSearchCards").visible);
   verify(findChild(ws,"intelligenceCategory-email")!==null);verify(findChild(ws,"intelligenceCategory-phone")!==null);
   verify(findChild(ws,"intelligenceDefaultPrice").text.includes("€14.00"));
   verify(!findChild(ws,"intelligenceStartPaidSearch").enabled);
   verify(findChild(ws,"lampyreMcpEndpoint")===null);
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"intelligence-cards-"+(ar?"ar":"en")+".png"));
  }
  const settings=findChild(ws,"intelligenceAdvancedSettings");
  const intelligenceScroll=findChild(app,"intelligenceWorkspace");
  reveal(intelligenceScroll,settings);mouseClick(settings);
  tryVerify(function(){return findChild(ws,"lampyreApiKey")!==null});
  verify(findChild(ws,"lampyreMcpEndpoint")===null);
  verify(findChild(ws,"lampyreConnectionPanel").connection.endpoint===undefined);
  for(const ar of [true,false]){
   app.ar=ar;verify(renderedFrame());
   reveal(intelligenceScroll,findChild(ws,"lampyreApiKey"));
   verify(findChild(ws,"lampyreMcpEndpoint")===null);
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"intelligence-setup-"+(ar?"ar":"en")+".png"));
  }
  findChild(ws,"lampyreApiKey").text="Synthetic-credential-draft";
  identityBridge.logout();
  tryVerify(function(){return findChild(ws,"lampyreApiKey")===null});
  verify(!settings.visible);
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"))
  app.navigatePrimary(3);verify(renderedFrame());
  const investigatorSettings=findChild(ws,"intelligenceAdvancedSettings");
  verify(investigatorSettings.visible);
  verify(findChild(ws,"intelligenceStartPaidSearch").enabled);
  verify(findChild(ws,"intelligenceRefreshCases").enabled);
  verify(lawCaseBridge.searchCases(""));
  ws.showDashboardDetails=true;verify(renderedFrame());
  reveal(intelligenceScroll,findChild(ws,"intelligenceActiveCase"));
  verify(uiEvidence.captureWindow(app,uiScreenshotBase+"intelligence-case-dashboard.png"));
  ws.showDashboardDetails=false;verify(renderedFrame());
  if(!ws.showAdvanced){reveal(intelligenceScroll,investigatorSettings);mouseClick(investigatorSettings);}
  tryVerify(function(){return findChild(ws,"lampyreApiKey")!==null});
  verify(findChild(ws,"lampyreMcpEndpoint")===null);
  verify(!findChild(ws,"lampyreExecuteTool").enabled);
  reveal(intelligenceScroll,findChild(ws,"lampyreApiKey"));
  verify(uiEvidence.captureWindow(app,uiScreenshotBase+"intelligence-investigator-setup.png"));
  app.goBack();
 }
 Component {id:firstRunPanelComponent;MR.IdentityPanel {app:test.windowUnderTest;identity:firstRunIdentity;width:600;height:650;x:400;y:160}}
 function test_yFirstAdministratorIsSignedIn(){
  verify(firstRunIdentity.needsSetup);const pane=firstRunPanelComponent.createObject(app.contentItem);verify(pane!==null);verify(renderedFrame());
  findChild(pane,"identityUsername").text="first-admin";findChild(pane,"identityDisplayName").text="Synthetic first administrator";findChild(pane,"identityPassword").text="Synthetic-only UI password 2026!";
  artifactClick(pane,findChild(pane,"identitySignIn"));verify(firstRunIdentity.authenticated,firstRunIdentity.error);compare(firstRunIdentity.session.role,"administrator");compare(findChild(pane,"identityPassword").text,"");pane.destroy();verify(renderedFrame());
 }
 function test_yGuidedInvestigatorAndCaseSave(){
  app.setWorkspace(true);app.navigateSection(10);verify(identityBridge.login("ui-admin","Synthetic-only UI password 2026!"));verify(renderedFrame());
  const law=findChild(app,"lawEnforcementWorkspace"),panel=findChild(app,"identityPanel");law.section=10;
  compare(findChild(app,"lawIdentityRole").text,app.roleLabel("administrator"));verify(findChild(app,"lawAccountGuidance").visible);
  findChild(app,"identityNewUsername").text="guided-investigator";findChild(app,"identityNewDisplayName").text="Synthetic guided operator";findChild(app,"identityNewPassword").text="Synthetic-only UI password 2026!";
  artifactClick(panel,findChild(app,"identityCreateInvestigatorAndSignIn"));compare(identityBridge.session.role,"investigator");compare(findChild(app,"identityNewPassword").text,"");
  law.newCase();verify(renderedFrame());const form=findChild(app,"lawCaseForm");form.step=3;verify(renderedFrame());mouseClick(findChild(app,"lawSaveCase"));verify(findChild(app,"lawFormError").text.length>0);compare(form.missingFields.length,7);
  const m={caseNumber:"SYNTHETIC-GUIDED-SAVE",investigatingAgency:"Synthetic lab",investigatorName:"Synthetic operator",seizedDeviceDescription:"Synthetic item",authorityReference:"TEST-ONLY",issuingAuthority:"Synthetic lab",scope:"Synthetic local fixture only"};form.form=m;verify(renderedFrame());mouseClick(findChild(app,"lawSaveCase"));compare(law.section,0);compare(lawCaseBridge.activeCase.metadata.caseNumber,m.caseNumber);
  law.editCase();form.setField("notes","Edited through guided workflow");form.step=3;verify(renderedFrame());mouseClick(findChild(app,"lawSaveCase"));compare(lawCaseBridge.activeCase.metadata.notes,"Edited through guided workflow");const id=lawCaseBridge.activeCase.id;lawCaseBridge.closeWorkspace();verify(lawCaseBridge.openCase(id));compare(lawCaseBridge.activeCase.metadata.notes,"Edited through guided workflow");
  verify(identityBridge.login("ui-investigator","Synthetic-only UI password 2026!"));app.setWorkspace(false);
 }
 function test_yLocalIdentity(){
  app.navigateSection(12);const law=findChild(app,"lawEnforcementWorkspace");law.section=10;verify(renderedFrame());identityBridge.logout();verify(!identityBridge.needsSetup)
  findChild(app,"identityUsername").text="ui-admin";verify(lawCaseBridge.activeCase.id===undefined);compare(lawCaseBridge.cases.length,0)
  findChild(app,"identityPassword").text="Synthetic-only UI password 2026!";artifactClick(findChild(app,"identityPanel"),findChild(app,"identitySignIn"));verify(identityBridge.authenticated,identityBridge.error);compare(identityBridge.session.role,"administrator");compare(lawCaseBridge.role,"unknown");compare(lawCaseBridge.cases.length,0);verify(renderedFrame())
  verify(uiEvidence.captureWindow(app,uiScreenshotBase+"identity-ar.png"));app.ar=false;verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"identity-en.png"))
  artifactClick(findChild(app,"identityPanel"),findChild(app,"identityLogout"));verify(!identityBridge.authenticated);law.section=0;app.navigateSection(0)
 }
 function test_compactGovernmentSettings(){
  app.setWorkspace(false);app.navigateSection(10);app.ar=true;
  tryVerify(function(){return app.font.family.indexOf("Noto Kufi Arabic")>=0},5000);
  const reason=findChild(app,"governmentNetworkReason"),reference=findChild(app,"governmentNetworkCaseRef"),consent=findChild(app,"governmentNetworkApproval"),grid=findChild(app,"governmentRequestFields");
  for(const size of [[1366,768],[1920,1080]])for(const ar of [true,false]){
   app.width=size[0];app.height=size[1];app.ar=ar;verify(renderedFrame());
   verify(reason.width<=320);verify(reference.width<=320);compare(grid.columns,2);
   compare(Math.round(reason.mapToItem(grid,0,0).y),Math.round(reference.mapToItem(grid,0,0).y));
   consent.checked=true;verify(renderedFrame());compare(String(findChild(consent,"selectionIndicator").color),"#a855f7");
   findChild(app,"settingsScroll").contentItem.contentY=0;
   verify(uiEvidence.captureWindow(app,uiScreenshotBase+"review-compact-settings-"+(ar?"ar":"en")+"-"+size[0]+".png"));
  }
  consent.checked=false;app.ar=true;
 }
 function test_yzNativeCardsDispatchAndDisplay(){
  verify(uiEvidence.prepareReviewContext());app.navigatePrimary(3);verify(renderedFrame());
  const cards=findChild(app,"intelligenceSearchCards");verify(cards!==null);cards.selectedCard="";
  verify(uiEvidence.prepareReviewConnection(true));tryCompare(lampyreConnection,"busy",false,5000);verify(lampyreConnection.connected);
  verify(lampyreConnection.refreshCatalog());tryCompare(lampyreConnection,"busy",false,5000);
  compare(uiEvidence.reviewConnectionStats().paidCalls,0);
  for(const category of ["email","phone"]){
   const before=uiEvidence.reviewConnectionStats().paidCalls;const collectionsBefore=uiEvidence.reviewConnectionStats().collections;
   const card=findChild(cards,"intelligenceCategory-"+category);verify(card!==null);card.clicked();
   const dialog=findChild(cards,"intelligenceServiceDialog");tryCompare(dialog,"opened",true);
   const specialized=findChild(dialog.contentItem,"intelligenceSpecializedMode");specialized.checked=true;specialized.toggled();
   tryCompare(lampyreConnection,"busy",false,5000);compare(lampyreConnection.selectedCategory,category);
   compare(uiEvidence.reviewConnectionStats().paidCalls,before);
   const selector=findChild(dialog.contentItem,"intelligenceServiceSelector");verify(selector!==null);selector.activated(0);
   const input=findChild(dialog.contentItem,"intelligenceArgument-"+category);verify(input!==null);
   const value=category==="email"?"native@example.invalid":"+12025550123";input.text=value;input.textEdited();
   const execute=findChild(dialog.contentItem,"intelligenceExecuteService");verify(!execute.enabled);
   const consent=findChild(dialog.contentItem,"intelligenceServiceConsent");consent.checked=true;verify(execute.enabled);
   const invoicesBefore=lampyreConnection.invoices.length;execute.clicked();
   tryCompare(lampyreConnection,"busy",false,5000);compare(lampyreConnection.state,"completed");compare(uiEvidence.reviewConnectionStats().collections,collectionsBefore+1);
   compare(uiEvidence.reviewConnectionStats().paidCalls,before+1);
   const task=uiEvidence.reviewConnectionStats().lastArguments.tasks[0];compare(task.tool_name,"synthetic_"+category+"_lookup");compare(task.args[category],value);
   compare(lampyreConnection.invoices.length,invoicesBefore+1);compare(lampyreConnection.currentInvoice.amountDueMinorUnits,1400);
   verify(cards.showSearchResult);verify(lampyreConnection.resultText.includes("fixture"));
   tryVerify(()=>findChild(dialog.contentItem,"lampyreConnectionPanel")!==null,3000);
   const finding=findChild(dialog.contentItem,"intelligenceFindingData");verify(finding!==null);verify(finding.text.includes("https://example.invalid/synthetic-profile"));
   const invoice=findChild(cards,"intelligenceCurrentInvoice");verify(invoice.visible);verify(invoice.text.includes("14.00"));verify(!consent.checked);
   compare(uiEvidence.reviewConnectionStats().paidCalls,before+1);dialog.close();tryCompare(dialog,"opened",false);
  }
 }
 function test_yzaUnifiedLookupCards(){
  verify(uiEvidence.prepareReviewContext());app.navigatePrimary(3);verify(renderedFrame());
  const cards=findChild(app,"intelligenceSearchCards");verify(cards!==null);cards.selectedCard="";
  verify(uiEvidence.prepareUnifiedReview());tryCompare(lampyreConnection,"busy",false,5000);verify(lampyreConnection.lookupConnected);
  compare(uiEvidence.reviewConnectionStats().lookupSearches,0);
  reviewCapture=true;
  for(const kind of ["phone","email","username"]){
   const before=uiEvidence.reviewConnectionStats().lookupSearches;findChild(cards,"intelligenceCategory-"+kind).clicked();
   const dialog=findChild(cards,"intelligenceServiceDialog");tryCompare(dialog,"opened",true);verify(cards.unifiedSearch);
   verify(!findChild(dialog.contentItem,"intelligenceServiceSelector").visible);compare(uiEvidence.reviewConnectionStats().lookupSearches,before);
   const value=kind==="phone"?"+12025550123":kind==="email"?"synthetic@example.invalid":"synthetic-user";
   const field=findChild(dialog.contentItem,"intelligenceArgument-"+kind);field.text=value;field.textEdited();
   const submit=findChild(dialog.contentItem,"intelligenceExecuteService");verify(!submit.enabled);
   const consent=findChild(dialog.contentItem,"intelligenceServiceConsent");consent.checked=true;verify(submit.enabled);submit.clicked();submit.clicked();
   tryCompare(lampyreConnection,"busy",false,5000);compare(lampyreConnection.state,"completed");
   compare(uiEvidence.reviewConnectionStats().lookupSearches,before+1);compare(uiEvidence.reviewConnectionStats().lastLookupInput[kind],value);
   const data=findChild(dialog.contentItem,"intelligenceFindingData");verify(data!==null);verify(data.text.includes("https://example.invalid/unified-profile"));
   verify(!consent.checked);compare(lampyreConnection.currentInvoice.amountDueMinorUnits,1400);
   verify(renderedFrame());verify(uiEvidence.captureWindow(app,uiScreenshotBase+"review-unified-"+kind+".png"));dialog.close();tryCompare(dialog,"opened",false);
  }
  reviewCapture=false;
 }
 function test_yzz_ownerInterfaceReview(){
  reviewCapture=true;app.width=1536;app.height=980;
  const screens=[];
  function capture(key,label){
   verify(renderedFrame());wait(30);const name="smoke-professional-ui-review-"+String(screens.length+1).padStart(3,"0")+"-"+key+".png";
   verify(uiEvidence.captureWindow(app,uiScreenshotBase.replace("smoke-professional-ui-","")+name));
   screens.push({file:name,label:label,language:app.ar?"ar":"en",page:app.currentPage,synthetic:true});
  }
  function captureScroll(key,label,name){
   const scroll=findChild(app,name);if(!scroll||!scroll.visible||!scroll.contentItem)return;
   const view=scroll.contentItem;if(view.contentHeight===undefined||view.contentHeight<=view.height+60)return;
   const start=view.contentY;for(let part=1;part<=12;part++){const next=Math.min(view.contentHeight-view.height,part*Math.max(100,view.height*0.85));if(next<=view.contentY+10)break;view.contentY=next;capture(key+"-part-"+(part+1),label);if(next>=view.contentHeight-view.height-2)break}view.contentY=start;
  }
  app.setWorkspace(true);app.navigateSection(10);verify(identityBridge.login("ui-admin","Synthetic-only UI password 2026!"));
  capture("optional-legacy-accounts","إدارة الحسابات القديمة — اختيارية");
  verify(uiEvidence.prepareReviewContext());
  for(const gov of [true,false]){
   app.setWorkspace(gov);
   for(let i=0;i<app.sections.length;i++){
    app.navigateSection(i);const key=(gov?"government-":"professional-")+i;capture(key,app.sections[i].ar);
    const scrollers={7:"settingsScroll",11:"helpWorkspace",14:"governmentAnalysisWorkspace",15:"intelligenceWorkspace"};if(scrollers[app.currentPage])captureScroll(key,app.sections[i].ar,scrollers[app.currentPage]);
   }
  }
  const tabs=[[1,"deviceTabs"],[2,"diagnosticTabs"],[4,"firmwareTabs"],[5,"workshopTabs"],[9,"forensicTabs"],[8,"recoveryTabs"]];
  for(const route of tabs){app.currentPage=route[0];app.activeSection=0;const bar=findChild(app,route[1]);if(bar){for(let i=0;i<bar.count;i++){bar.currentIndex=i;capture(route[1]+"-"+i,route[1]+" / "+i)}}}
  app.currentPage=3;capture("board","فحص اللوحات");app.currentPage=6;capture("parts","مخزون قطع الغيار");
  app.setWorkspace(true);app.navigateSection(7);const research=findChild(app,"researchCenter");
  for(let i=0;i<9;i++){research.section=i;capture("research-"+i,research.titles[i]);captureScroll("research-"+i,research.titles[i],"researchScroll")}
  app.navigateSection(0);const law=findChild(app,"lawEnforcementWorkspace");
  for(let i=0;i<12;i++){law.section=i;capture("law-"+i,law.titles[i])}
  law.newCase();const form=findChild(app,"lawCaseForm");
  for(let i=0;i<4;i++){form.step=i;findChild(app,"lawFormScroll").contentItem.contentY=0;capture("case-form-"+i,"إدخال القضية / "+i);captureScroll("case-form-"+i,"إدخال القضية / "+i,"lawFormScroll")}
  form.form={caseNumber:"SYNTHETIC-VALID-INPUT",investigatingAgency:"Synthetic UI lab",investigatorName:"Synthetic operator"};form.step=0;capture("case-valid-fields","حقول صحيحة — خضراء");
  form.setField("validFromUtc","invalid time");form.step=1;capture("case-invalid-time","قيمة وقت غير صحيحة — صفراء");form.setField("validFromUtc","2026-11-13T12:00:00Z");capture("case-time-controls","عناصر تعديل التاريخ والوقت");
  app.currentPage=8;const recovery=findChild(app,"recoveryWorkspace");
  if(recovery){findChild(app,"recoveryTabs").currentIndex=1;for(let i=0;i<8;i++){recovery.step=i;capture("recovery-step-"+i,"خطوة الاستعادة / "+i)}}
  app.navigatePrimary(3);const ws=findChild(app,"osintWorkspace");const cards=findChild(app,"intelligenceSearchCards");
  verify(cards!==null);
  for(const category of ["email","phone"]){const button=findChild(cards,"intelligenceCategory-"+category);verify(button!==null);button.clicked();capture("search-"+category,"بحث "+category);findChild(cards,"intelligenceServiceDialog").close()}
  if(ws){
   ws.showAdvanced=true;verify(renderedFrame());const scroll=findChild(app,"intelligenceWorkspace");reveal(scroll,findChild(ws,"lampyreConnectionPanel"));capture("intelligence-setup","إعداد اتصال الاستخبارات");ws.showAdvanced=false;scroll.contentItem.contentY=0;
   ws.showDashboardDetails=true;capture("intelligence-history","قضايا وسجل الاستخبارات");ws.showDashboardDetails=false;
   ws.demoMode=true;ws.activeTab=2;verify(renderedFrame());const analysis=findChild(ws,"osintAnalysisPane");verify(analysis!==null);reveal(scroll,analysis);capture("graph-example","واجهة الرسم البياني — مثال محلي فقط");ws.demoMode=false;ws.activeTab=1;scroll.contentItem.contentY=0;
  }
  cards.selectedCard="";verify(uiEvidence.prepareReviewConnection());tryCompare(lampyreConnection,"busy",false,5000);verify(lampyreConnection.connected);verify(lampyreConnection.refreshCatalog());tryCompare(lampyreConnection,"busy",false,5000);
  verify(lampyreConnection.discoverServices("email"));tryCompare(lampyreConnection,"busy",false,5000);
  cards.selectedCard="email";cards.specializedSearch=true;cards.selectedService="synthetic_email_lookup";cards.values={email:"review@example.invalid"};findChild(cards,"intelligenceServiceDialog").open();capture("email-confirmation","تأكيد البحث — بيانات محلية فقط");
  verify(lampyreConnection.runService("synthetic_email_lookup",JSON.stringify(cards.values),2,true));cards.showSearchResult=true;tryCompare(lampyreConnection,"busy",false,5000);capture("invoice-due","فاتورة مستحقة تجريبية 14 يورو");findChild(cards,"intelligenceServiceDialog").close();capture("invoices","سجل الفواتير التجريبي");
  app.ar=false;capture("intelligence-en","Intelligence / English");
  for(const section of [0,1,6,10]){app.setWorkspace(true);app.ar=false;app.navigateSection(section);capture("government-en-"+section,app.sections[section].en)}
  verify(uiEvidence.saveReviewManifest(screens));reviewCapture=false;app.ar=true;
 }
 function test_z_exitAccept(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);mouseClick(dialog.standardButton(Dialog.Yes));tryCompare(app,"visible",false)}
}
