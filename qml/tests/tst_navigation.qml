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
 function init(){app.allowClose=false;app.ar=true;app.currentPage=0}
 function test_minimumSize(){
  app.width=1180;app.height=760;app.currentPage=7;wait(80);let scroll=findChild(app,"settingsScroll");verify(scroll!==null);verify(scroll.contentHeight>scroll.availableHeight,"Settings must scroll at minimum height");
  for(let i=0;i<9;i++){let b=app.navigationButton(i);verify(b.width>60);let p=b.mapToItem(app.contentItem,0,0);verify(p.x>=0);verify(p.x+b.width<=app.width+1)}
  let deviceScroll=findChild(app,"deviceScroll");verify(deviceScroll!==null);tryVerify(function(){return deviceScroll.contentHeight>deviceScroll.availableHeight});
  deviceScroll.contentItem.contentY=deviceScroll.contentHeight-deviceScroll.availableHeight;verify(waitForRendering(app.contentItem));
  let backup=findChild(app,"quickBackup");let bp=backup.mapToItem(deviceScroll,backup.width/2,backup.height/2);verify(bp.y>=0&&bp.y<deviceScroll.height,"Quick backup action remains reachable");deviceScroll.contentItem.contentY=0;
  app.width=1536;app.height=980;
 }
 function test_navigation(){for(let i=0;i<9;i++){let b=app.navigationButton(i);verify(b!==null,"Missing navigation button nav"+i);mouseClick(b);compare(app.currentPage,i)}}
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
  findChild(app,"customerField").text="UI test customer";findChild(app,"brandField").text="TestBrand";findChild(app,"modelField").text="Model-01";findChild(app,"symptomField").text="USB not detected";
  mouseClick(dialog.standardButton(Dialog.Save));compare(workshopBridge.orders.length,before+1);tryCompare(dialog,"visible",false);
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
  compare(result.text,"not_supported");
  let consent=findChild(app,"recoveryConsent");
  verify(consent!==null);
  consent.checked=true;
  tryCompare(result,"text","need_more_information");
  let source=findChild(app,"recoverySource");
  source.currentIndex=3;
  app.currentPage=0;
  app.currentPage=8;
  let reset=findChild(app,"recoveryReset");
  reset.checked=true;
  compare(result.text,"not_supported");
  reset.checked=false;
  consent.checked=false;
 }
 function test_exitCancel(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);let no=dialog.standardButton(Dialog.No);verify(no);mouseClick(no);tryCompare(dialog,"visible",false);compare(app.visible,true)}
 function test_z_exitAccept(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"opened",true);mouseClick(dialog.standardButton(Dialog.Yes));tryCompare(app,"visible",false)}
}
