import QtQuick
import QtQuick.Controls
import QtTest
import ".." as MR
TestCase {
 id:test
 name:"WorkstationNavigation"
 when:windowShown
 width:1536;height:980
 MR.Main {id:app}
 function init(){findChild(app,"deviceTabs").currentIndex=0;findChild(app,"workshopTabs").currentIndex=0;findChild(app,"diagnosticTabs").currentIndex=0;app.allowClose=false;app.toastText="";app.ar=true;app.darkTheme=true;app.showDevicePanel=false;app.width=1536;app.height=980;app.currentPage=0;verify(waitForRendering(app.contentItem))}
 function test_avbScopeAndMinimumSize(){
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
 function test_compiledBuildProfileIsVisible(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1;verify(waitForRendering(app.contentItem))
  const label=findChild(app,"serviceBuildProfile");verify(label.visible)
  app.ar=false;verify(label.text.indexOf(toolBridge.laboratoryBuild?"Laboratory":"read-only")>=0)
  verify(findChild(app,"serviceWorkflow").text.indexOf("Discover")>=0)
 }
 function test_deviceServiceNavigationAndConsent(){
  app.currentPage=1;const tabs=findChild(app,"deviceTabs");tabs.currentIndex=1;verify(waitForRendering(app.contentItem));const page=findChild(app,"deviceServicesPage");verify(page!==null)
  compare(findChild(app,"serviceBrand").count,14);compare(findChild(app,"serviceDevice").currentIndex,-1);verify(!findChild(app,"exportDeviceServiceReport").enabled)
  findChild(app,"serviceBrand").currentIndex=1;verify(!findChild(app,"serviceOfficialDrivers").enabled);verify(findChild(app,"serviceDriverUnavailableReason").visible);findChild(app,"serviceBrand").currentIndex=0
  for(const arabic of [true,false])for(const dark of [true,false]){
   app.ar=arabic;app.darkTheme=dark;app.width=1180;app.height=760;wait(60)
   const list=findChild(app,"deviceServiceList");verify(list.width>240&&list.height>150)
   page.review({id:"sideload_zip",ar:"اختبار موافقة اصطناعي",en:"Synthetic consent test",risk:"destructive"})
   const dialog=findChild(app,"deviceServiceConfirm");tryCompare(dialog,"opened",true);verify(!dialog.standardButton(Dialog.Ok).enabled)
   findChild(app,"serviceOwnerConsent").checked=true;verify(!dialog.standardButton(Dialog.Ok).enabled)
   findChild(app,"serviceDataLossConsent").checked=true;verify(!dialog.standardButton(Dialog.Ok).enabled)
   dialog.reject();tryCompare(dialog,"opened",false)
  }
  tabs.currentIndex=2;verify(waitForRendering(app.contentItem));const rows=findChild(app,"deviceCatalogList");compare(rows.count,44)
  const search=findChild(app,"deviceCatalogSearch");search.text="SM-A057G";tryCompare(rows,"count",1);search.text="nothing matches";tryCompare(rows,"count",0);search.text=""
  mouseClick(findChild(app,"importDeviceCatalog"));const dialog=findChild(app,"deviceCatalogDialog");tryCompare(dialog,"opened",true);verify(!dialog.standardButton(Dialog.Ok).enabled);dialog.reject()
  mouseClick(app.navigationButton(0));compare(app.currentPage,0)
 }
 function cleanup(){if(recoveryBridge.active&&!recoveryBridge.busy)recoveryBridge.closeCase();app.showDevicePanel=false;app.width=1536;app.height=980}
 function test_serviceHistoryDetailsArePlainText(){
  app.currentPage=1;findChild(app,"deviceTabs").currentIndex=1;verify(waitForRendering(app.contentItem))
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
   const details=findChild(app,"deviceServiceDetailsScroll");verify(details!==null);details.contentItem.contentY=0;wait(30)
   let point=result.mapToItem(details,result.width/2,80);verify(point.y>=0&&point.y<details.height,"Result remains visible at the top")
   details.contentItem.contentY=Math.max(0,details.contentHeight-details.availableHeight);wait(30)
   const report=findChild(app,"exportDeviceServiceReport");point=report.mapToItem(details,report.width/2,report.height/2);verify(point.y>=0&&point.y<details.height,"Report remains reachable by scrolling")
   details.contentItem.contentY=0
  }
  findChild(app,"serviceBrand").currentIndex=0
 }
 function test_centerProfilesAndTracking(){
  const id=workshopBridge.createOrder("SYNTHETIC UI owner","test-contact","OPPO","TRAINING-01","test-serial","USB","Tech","private");verify(id>0)
  const order=workshopBridge.invoice(id)
  const second=workshopBridge.createOrderForDevice(order.device_id,"Screen","Tech","second visit");verify(second>id)
  compare(workshopBridge.deviceProfile(order.device_id).orders.length,2)
  app.currentPage=5;const tabs=findChild(app,"workshopTabs");tabs.currentIndex=1;verify(waitForRendering(app.contentItem))
  verify(findChild(app,"customerProfiles").count>0);verify(findChild(app,"deviceProfiles").count>0)
  tabs.currentIndex=2;verify(waitForRendering(app.contentItem));verify(!findChild(app,"startPortal").enabled);verify(!findChild(app,"issueTracking").enabled)
  verify(trackingPortal.start("127.0.0.1",0,true));verify(waitForRendering(app.contentItem));verify(findChild(app,"stopPortal").enabled)
  trackingPortal.stop();verify(!trackingPortal.running)
 }
 function test_securityAndFaultTree(){
  const id=workshopBridge.createOrder("Synthetic tree owner","","Samsung","SYNTHETIC-02","","USB","Tech","");verify(id>0)
  app.currentPage=2;const tabs=findChild(app,"diagnosticTabs");tabs.currentIndex=2;verify(waitForRendering(app.contentItem))
  verify(!findChild(app,"startSecurityScan").enabled);verify(!findChild(app,"updateSignatures").enabled);verify(!findChild(app,"cancelSignatureUpdate").enabled);compare(findChild(app,"securityVerdict").text,app.trx("لم يُجرَ فحص بعد","No scan performed yet"))
  tabs.currentIndex=3;verify(waitForRendering(app.contentItem));compare(serviceCatalog.drivers.length,6)
  tabs.currentIndex=1;verify(waitForRendering(app.contentItem));const choice=findChild(app,"faultTreeOrder");verify(choice.count>0)
  const selected=choice.currentValue;mouseClick(findChild(app,"startFaultTree"));verify(findChild(app,"faultTreeQuestion").text.length>5)
  mouseClick(findChild(app,"faultTreeYes"));verify(!findChild(app,"faultTreeYes").enabled)
  findChild(app,"treeTechnician").text="Synthetic technician";findChild(app,"treeFinding").text="Synthetic safety observation only";
  const save=findChild(app,"saveFaultTree");verify(save.enabled)
  // The save control is below the first viewport: scroll it into view before clicking.
  const scroll=findChild(app,"faultTreePage");const position=save.mapToItem(scroll.contentItem,0,0);scroll.contentItem.contentY=Math.max(0,position.y-scroll.availableHeight+save.height+20);wait(60);mouseClick(save)
  verify(workshopBridge.diagnoses(selected).length>0)
 }
 function test_cloudTrackingConsent(){
  app.currentPage=5;findChild(app,"workshopTabs").currentIndex=2;verify(waitForRendering(app.contentItem))
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
  for(let i=0;i<10;i++){let b=app.navigationButton(i);verify(b.width>60);let p=b.mapToItem(app.contentItem,0,0);verify(p.x>=0);verify(p.x+b.width<=app.width+1)}
  let deviceScroll=findChild(app,"deviceScroll");verify(deviceScroll!==null);verify(deviceScroll.availableHeight>0);
  deviceScroll.contentItem.contentY=Math.max(0,deviceScroll.contentHeight-deviceScroll.availableHeight);
  // Content can fit without scrolling, so no new render frame is required.
  let backup=findChild(app,"quickBackup");verify(backup!==null);
  tryVerify(function(){let point=backup.mapToItem(deviceScroll,backup.width/2,backup.height/2);return point.x>=0&&point.x<deviceScroll.width&&point.y>=0&&point.y<deviceScroll.height},5000,"Quick backup action remains reachable");deviceScroll.contentItem.contentY=0;
  app.showDevicePanel=false;app.width=1536;app.height=980;
 }
 function test_navigation(){for(let i=0;i<10;i++){let b=app.navigationButton(i);verify(b!==null,"Missing navigation button nav"+i);wait(50);let point=b.mapToItem(app.contentItem,b.width/2,b.height/2);verify(point.y>=0&&point.y<app.height,"Navigation remains reachable: "+i);mouseClick(b);tryCompare(app,"currentPage",i)}}
 function test_forensicLabNavigation(){
  app.currentPage=0;
  let nav=app.navigationButton(9);
  verify(nav!==null,"Forensic Lab navigation missing");
  mouseClick(nav);
  compare(app.currentPage,9);
  let lab=findChild(app,"forensicLabWorkspace");
  verify(lab!==null,"Forensic Lab workspace missing");
  compare(lab.modules.length,13);
  compare(lab.modules.filter(function(m){return m.ready}).length,10);
  verify(waitForRendering(app.contentItem));
  app.ar=false;
  verify(waitForRendering(app.contentItem));
  compare(app.titles[9],"Forensic Lab");
  app.ar=true;
  verify(waitForRendering(app.contentItem));
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
  mouseClick(run);tryCompare(recoveryBridge,"busy",false);compare(recoveryBridge.error,"");compare(recoveryBridge.evidence.length,1)
  verify(recoveryBridge.previewEvidence(0).indexOf("nexvary-pcap-summary/v1")>=0)
  const show=findChild(app,"showPcapResult");verify(show.enabled);mouseClick(show);compare(tabs.currentIndex,1)
  verify(findChild(app,"forensicPreview").text.indexOf("nexvary-pcap-summary/v1")>=0);tabs.currentIndex=3
  app.toastText="";app.darkTheme=true
  for(const arabic of [true,false]){
   app.ar=arabic;wait(60);verify(waitForRendering(app.contentItem))
   verify(uiEvidence.captureWindow(app,pcapScreenshotBase+(arabic?"ar":"en")+".png"))
  }
  tabs.currentIndex=1;const kind=findChild(app,"forensicKind");kind.currentIndex=kind.count-1;tryCompare(findChild(app,"forensicEvidenceList"),"count",1);kind.currentIndex=0;tabs.currentIndex=3
  page.capturePath="";compare(consent.checked,false);compare(run.enabled,false)
  verify(recoveryBridge.closeCase());tabs.currentIndex=0
 }
 function test_forensicWorkspace(){
  app.currentPage=9;
  let tabs=findChild(app,"forensicTabs");verify(tabs!==null);tabs.currentIndex=1;
  verify(recoveryBridge.createCase("Synthetic lab UI","LAB-ONLY","Synthetic","No phone","15","backup","unknown",true));
  findChild(app,"forensicOperator").text="UI operator";findChild(app,"forensicAuthorization").text="LAB authorization";
  let record=findChild(app,"forensicRecordOperator");compare(record.enabled,false);
  findChild(app,"forensicOperatorConsent").checked=true;tryCompare(record,"enabled",true);
  verify(waitForRendering(app.contentItem));mouseClick(record);compare(recoveryBridge.caseOperator,"UI operator");
  verify(recoveryBridge.importVisibleFile(syntheticEvidencePath,true));tryCompare(recoveryBridge,"busy",false);
  let files=findChild(app,"forensicEvidenceList");tryCompare(files,"count",1);
  mouseClick(files,files.width/2,40);let preview=findChild(app,"forensicPreview");tryVerify(function(){return preview.text.indexOf("SYNTHETIC UI EVIDENCE")>=0});
  let search=findChild(app,"forensicSearch");search.text="no matching file";tryCompare(files,"count",0);search.text="synthetic";tryCompare(files,"count",1);search.text="";
  let report=findChild(app,"forensicReport");compare(report.enabled,false);findChild(app,"forensicPlaintext").checked=true;tryCompare(report,"enabled",true);findChild(app,"forensicPlaintext").checked=false;
  mouseClick(findChild(app,"forensicVerify"));tryCompare(recoveryBridge,"busy",false);compare(recoveryBridge.error,"");
  let custodyButton=findChild(app,"forensicCustody");let detail=findChild(app,"forensicDetailScroll");
  let point=custodyButton.mapToItem(detail,custodyButton.width/2,custodyButton.height/2);detail.contentItem.contentY+=point.y-detail.height/2;
  verify(waitForRendering(app.contentItem));mouseClick(custodyButton);let dialog=findChild(app,"forensicCustodyDialog");tryCompare(dialog,"opened",true);compare(dialog.standardButton(Dialog.Ok).enabled,false);
  findChild(app,"forensicCustodyRecipient").text="Lab storage";findChild(app,"forensicCustodyReason").text="Synthetic review";
  tryCompare(dialog.standardButton(Dialog.Ok),"enabled",true);mouseClick(dialog.standardButton(Dialog.Ok));tryCompare(dialog,"opened",false);compare(recoveryBridge.evidenceCustodian(0),"Lab storage");
  tabs.currentIndex=2;let events=findChild(app,"forensicTimelineList");tryVerify(function(){return events.count>=5});findChild(app,"forensicTimelineSearch").text="custody-transfer";tryCompare(events,"count",1);findChild(app,"forensicTimelineSearch").text="";
  for(let language=0;language<2;language++){app.ar=language===0;for(let i=0;i<tabs.count;i++){tabs.currentIndex=i;verify(waitForRendering(app.contentItem))}}
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
    for(let tab=0;tab<tabs.count;tab++){tabs.currentIndex=tab;verify(waitForRendering(app.contentItem))}
   }
  }
  tabs.currentIndex=0;app.darkTheme=true;app.width=1536;app.height=980;
 }
 function test_language(){let b=findChild(app,"languageButton");mouseClick(b);compare(app.ar,false);mouseClick(b);compare(app.ar,true)}
 function test_boardMeasurement(){
  app.currentPage=3;verify(waitForRendering(app.contentItem));mouseClick(findChild(app,"loadTrainingBoard"));tryVerify(function(){return boardBridge.loaded});verify(boardBridge.components.length>0);verify(waitForRendering(app.contentItem));
  let input=findChild(app,"boardVoltage");input.text="3.30";let before=boardBridge.measurements.length;
  let record=findChild(app,"recordMeasurement");verify(record.enabled);verify(waitForRendering(app.contentItem));
  const inspector=findChild(app,"boardInspector");verify(inspector!==null);
  let center=record.mapToItem(inspector,record.width/2,record.height/2);
  if(center.y<0||center.y>=inspector.height){inspector.contentItem.contentY+=center.y-inspector.height/2;verify(waitForRendering(app.contentItem))}
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
  app.currentPage=2;verify(waitForRendering(app.contentItem));
  let choice=findChild(app,"diagnosisOrder");tryVerify(function(){return choice.count>1});choice.currentIndex=choice.indexOfValue(id);
  findChild(app,"diagnosisTechnician").text="UI technician";findChild(app,"diagnosisFindings").text="Synthetic observation only";findChild(app,"diagnosisEvidence").text="No phone connected; UI test";
  let analyze=findChild(app,"diagnoseButton");mouseClick(analyze);let save=findChild(app,"saveDiagnosis");tryCompare(save,"enabled",true);
  let scroll=findChild(app,"diagnosisScroll");verify(scroll!==null);let center=save.mapToItem(scroll,save.width/2,save.height/2);scroll.contentItem.contentY+=center.y-scroll.height/2;
  verify(waitForRendering(app.contentItem));mouseClick(save);tryVerify(function(){return workshopBridge.diagnoses(id).length===1});
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
  for(let i=0;i<8;i++){page.step=i;verify(waitForRendering(app.contentItem))}
  page.step=1;findChild(app,"recoveryBrand").text="Synthetic";findChild(app,"recoveryModel").text="UI fixture";
  page.step=4;findChild(app,"recoveryTitle").text="Synthetic UI case";findChild(app,"recoveryOwner").text="Test-only owner reference";
  let create=findChild(app,"createRecoveryCase");compare(create.enabled,false);verify(waitForRendering(app.contentItem));mouseClick(findChild(app,"caseOwnerConsent"));tryCompare(create,"enabled",true);mouseClick(create);tryCompare(recoveryBridge,"active",true);compare(page.step,5);
  compare(findChild(app,"runRecovery").enabled,false);page.step=4;verify(waitForRendering(app.contentItem));mouseClick(findChild(app,"closeRecoveryCase"));tryCompare(recoveryBridge,"active",false);findChild(app,"recoveryTabs").currentIndex=0;
 }
 function test_exitCancel(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);let no=dialog.standardButton(Dialog.No);verify(no);mouseClick(no);tryCompare(dialog,"visible",false);compare(app.visible,true)}
 function test_z_exitAccept(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);mouseClick(dialog.standardButton(Dialog.Yes));tryCompare(app,"visible",false)}
}
