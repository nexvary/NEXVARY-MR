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
  property bool lookupConnected:true
  property int lookupRuns:0
  property var lookupInput:({})
  property int pendingResultCount:0
  property bool busy:false
  property double retailPriceEur:14
  property double creditBalance:37.5
  property string selectedCategory:""
  property string resultText:""
  property string errorCode:""
  property int runs:0
  property var categories:Object.keys(Categories.titles).map(name=>({name:name}))
  property var invoices:[]
  property var services:[]
  function displayText(text){return text}
  function refreshCatalog(){}
  function discoverServices(name){selectedCategory=name;services=[{tool_name:"fixture_lookup",title:"Synthetic lookup",args:"email: string (all required)",photons:2}];changed()}
  function runLookup(kind,value,consent){if(consent){lookupRuns++;lookupInput={kind:kind,value:value}}return consent}
  function runService(name,args,price,consent){if(consent)runs++;return consent}
 }
 Component {id:component;MR.IntelligenceSearchCards {width:960;app:style;connection:engine}}
 function init(){engine.connected=true;engine.lookupConnected=true;engine.lookupRuns=0;engine.invoices=[];engine.runs=0;engine.selectedCategory="";engine.services=[];style.ar=false}
 function test_allCardsDisplayRetailPriceInBothLanguages(){
  const pane=createTemporaryObject(component,test);verify(pane);compare(pane.shownCategories.length,54)
  for(const ar of [false,true]){
   style.ar=ar;wait(1)
   for(const row of engine.categories){const card=findChild(pane,"intelligenceCategory-"+row.name);verify(card);compare(findChild(card,"cardRetailPriceText").text,"€14.00")}
  }
  compare(engine.runs,0)
 }
 function test_customerInvoiceShowsEurosWithoutSupplierCredits(){
  const pane=createTemporaryObject(component,test);verify(pane)
  engine.invoices=[{id:"synthetic-invoice",category:"phone",amountDueMinorUnits:1400,status:"due",paymentStatus:"unpaid",internalQuotedPhotons:35}]
  for(const ar of [false,true]){style.ar=ar;const summary=findChild(pane,"intelligenceInvoice-synthetic-invoice");verify(summary);verify(summary.text.includes("€14.00"));verify(!summary.text.includes("35"));verify(!summary.text.toLowerCase().includes("photon"));verify(!summary.text.includes("فوتون"))}
 }
 function test_disconnectedCardStillOpensItsInputForm(){
  engine.connected=false;engine.lookupConnected=false;const pane=createTemporaryObject(component,test);verify(pane)
  findChild(pane,"intelligenceCategory-phone").clicked();const dialog=findChild(pane,"intelligenceServiceDialog");tryCompare(dialog,"opened",true)
  verify(findChild(dialog.contentItem,"intelligenceArgument-phone"));verify(!findChild(dialog.contentItem,"intelligenceExecuteService").enabled);compare(engine.runs,0)
 }
 function test_primaryCardsUseUnifiedSearch(){
  const pane=createTemporaryObject(component,test);verify(pane)
  for(const kind of ["phone","email","username"]){
   findChild(pane,"intelligenceCategory-"+kind).clicked();const dialog=findChild(pane,"intelligenceServiceDialog");tryCompare(dialog,"opened",true);verify(pane.unifiedSearch)
   const input=findChild(dialog.contentItem,"intelligenceArgument-"+kind);input.text=kind==="phone"?"+12025550123":kind==="email"?"synthetic@example.invalid":"synthetic-user";input.textEdited()
   const consent=findChild(dialog.contentItem,"intelligenceServiceConsent");const button=findChild(dialog.contentItem,"intelligenceExecuteService");verify(!button.enabled);consent.checked=true;verify(button.enabled);button.clicked();compare(engine.lookupInput.kind,kind);dialog.close();tryCompare(dialog,"opened",false)
  }
  compare(engine.lookupRuns,3);compare(engine.runs,0)
 }
 function test_openingCardDoesNotExecuteSearch(){
  const pane=createTemporaryObject(component,test);verify(pane)
  findChild(pane,"intelligenceCategory-email").clicked();const specialized=findChild(findChild(pane,"intelligenceServiceDialog").contentItem,"intelligenceSpecializedMode");specialized.checked=true;specialized.toggled();compare(engine.selectedCategory,"email");compare(engine.runs,0);tryCompare(findChild(pane,"intelligenceServiceDialog"),"opened",true)
  const content=findChild(pane,"intelligenceServiceDialog").contentItem
  const primary=findChild(content,"intelligenceArgument-email");verify(primary);primary.text="research@example.invalid";primary.textEdited();
  const selector=findChild(content,"intelligenceServiceSelector");verify(selector);selector.activated(0);compare(pane.values.email,"research@example.invalid");compare(pane.selectedService,"fixture_lookup");compare(pane.fields.length,1)
  const input=findChild(content,"intelligenceArgument-email");verify(input);input.text="research@example.invalid";input.textEdited()
  const execute=findChild(content,"intelligenceExecuteService");verify(!execute.enabled)
  const consent=findChild(content,"intelligenceServiceConsent");consent.checked=true;verify(execute.enabled);execute.clicked();compare(engine.runs,1);verify(!consent.checked)
 }
}
