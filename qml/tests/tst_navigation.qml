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
 function test_navigation(){for(let i=0;i<8;i++){let b=app.navigationButton(i);verify(b!==null,"Missing navigation button nav"+i);mouseClick(b);compare(app.currentPage,i)}}
 function test_language(){let b=findChild(app,"languageButton");mouseClick(b);compare(app.ar,false);mouseClick(b);compare(app.ar,true)}
 function test_boardMeasurement(){
  app.currentPage=3;mouseClick(findChild(app,"loadTrainingBoard"));verify(boardBridge.loaded);verify(boardBridge.components.length>0);
  let input=findChild(app,"boardVoltage");input.text="3.30";let before=boardBridge.measurements.length;
  let record=findChild(app,"recordMeasurement");verify(record.enabled);mouseClick(record);compare(boardBridge.measurements.length,before+1);verify(boardBridge.dirty);
 }
 function test_createOrder(){
  app.currentPage=5;let before=workshopBridge.orders.length;mouseClick(findChild(app,"newOrder"));let dialog=findChild(app,"newOrderDialog");tryCompare(dialog,"visible",true);
  findChild(app,"customerField").text="UI test customer";findChild(app,"brandField").text="TestBrand";findChild(app,"modelField").text="Model-01";findChild(app,"symptomField").text="USB not detected";
  mouseClick(dialog.standardButton(Dialog.Save));compare(workshopBridge.orders.length,before+1);tryCompare(dialog,"visible",false);
 }
 function test_diagnosis(){app.currentPage=2;let b=findChild(app,"diagnoseButton");mouseClick(b);let r=findChild(app,"diagnosisResult");verify(r.text.length>40);verify(r.readOnly)}
 function test_exitCancel(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"visible",true);let no=dialog.standardButton(Dialog.No);verify(no);mouseClick(no);tryCompare(dialog,"visible",false);compare(app.visible,true)}
 function test_z_exitAccept(){mouseClick(findChild(app,"exitButton"));let dialog=findChild(app,"exitDialog");tryCompare(dialog,"visible",true);mouseClick(dialog.standardButton(Dialog.Yes));tryCompare(app,"visible",false)}
}
