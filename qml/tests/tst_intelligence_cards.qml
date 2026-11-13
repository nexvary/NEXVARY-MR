import QtQuick
import QtQuick.Controls
import QtTest
import ".." as MR
import "../IntelligenceCategories.js" as Categories
TestCase {
 id:test
 name:"IntelligenceCards"
 when:windowShown;width:1000;height:800;visible:true
 QtObject {
  id:style
  property bool ar:false
  property var design:({text:"#E5EDF3",muted:"#9EB4C1",surface:"#0A0A0A",raised:"#0A2231",border:"#8396A3",accent:"#00BFFF",success:"#39FF14",danger:"#FF3131"})
  function trx(a,e){return ar?a:e}
 }
 QtObject {
  id:engine
  signal changed()
  property bool connected:true
  property bool busy:false
  property double retailPriceEur:14
  property double creditBalance:37.5
  property string selectedCategory:""
  property string resultText:""
  property string errorCode:""
  property int runs:0
  property var categories:Object.keys(Categories.titles).map(name=>({name:name}))
  property var services:[]
  function displayText(text){return text}
  function refreshCatalog(){}
  function discoverServices(name){selectedCategory=name;services=[{tool_name:"fixture_lookup",title:"Synthetic lookup",args:"email: string (all required)",photons:2}];changed()}
  function runService(name,args,price,consent){if(consent)runs++;return consent}
 }
 Component {id:component;MR.IntelligenceSearchCards {width:960;app:style;connection:engine}}
 function init(){engine.runs=0;engine.selectedCategory="";engine.services=[];style.ar=false}
 function test_allCardsDisplayRetailPriceInBothLanguages(){
  const pane=createTemporaryObject(component,test);verify(pane);compare(pane.shownCategories.length,54)
  for(const ar of [false,true]){
   style.ar=ar;wait(1)
   for(const row of engine.categories){const card=findChild(pane,"intelligenceCategory-"+row.name);verify(card);compare(findChild(card,"cardRetailPriceText").text,"€14.00")}
  }
  compare(engine.runs,0)
 }
 function test_openingCardDoesNotExecuteSearch(){
  const pane=createTemporaryObject(component,test);verify(pane)
  findChild(pane,"intelligenceCategory-email").clicked();compare(engine.selectedCategory,"email");compare(engine.runs,0);tryCompare(findChild(pane,"intelligenceServiceDialog"),"opened",true)
  const content=findChild(pane,"intelligenceServiceDialog").contentItem
  const selector=findChild(content,"intelligenceServiceSelector");verify(selector);selector.activated(0);compare(pane.selectedService,"fixture_lookup");compare(pane.fields.length,1)
  const input=findChild(content,"intelligenceArgument-email");verify(input);input.text="research@example.invalid";input.textEdited()
  const execute=findChild(content,"intelligenceExecuteService");verify(!execute.enabled)
  const consent=findChild(content,"intelligenceServiceConsent");consent.checked=true;verify(execute.enabled);execute.clicked();compare(engine.runs,1);verify(!consent.checked)
 }
}
