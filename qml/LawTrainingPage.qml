import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 property int selected:0
 property var result:({})
 readonly property var scenario:lawCaseBridge.scenarios[selected]||({})
 function choiceAt(index){return choices.itemAt(index)}
 contentWidth:availableWidth;clip:true
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {title:"NEXVARY Professional Training Lab";iconName:"book_open_lightbulb";stateText:app.trx("محاكاة فقط","SIMULATION ONLY");Layout.fillWidth:true
   Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("سيناريوهات اصطناعية لاتخاذ القرار؛ لا اتصال بهاتف أو تنفيذ أوامر أو قراءة قضية حقيقية. صلاحية التدريب لا تمنح صلاحية إجراءات الأجهزة. النتائج تدريبية وليست اعتمادًا مهنيًا.","Synthetic decision scenarios with no phone connection, command execution or real-case access. Training permission grants no device-operation permission. Scores are training feedback, not professional certification.")}
   ComboBox {id:selector;objectName:"lawTrainingScenario";Layout.fillWidth:true;model:lawCaseBridge.scenarios;textRole:app.ar?"titleAr":"titleEn";currentIndex:page.selected;onActivated:{page.selected=currentIndex;page.result={}}}
   Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;color:app.design.text;font.bold:true;text:page.app.ar?(page.scenario.titleAr||""):(page.scenario.titleEn||"")}
   Label {objectName:"lawTrainingEvidence";Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:app.design.muted;text:page.app.ar?(page.scenario.evidenceAr||JSON.stringify(page.scenario.observations||{},null,2)):(page.scenario.evidenceEn||JSON.stringify(page.scenario.observations||{},null,2))}
   Repeater {id:choices;model:page.scenario.choices||[]
    delegate:ActionButton {id:choice;required property var modelData;objectName:"lawTrainingChoice-"+modelData.id;Layout.fillWidth:true;text:page.app.ar?(modelData.labelAr||modelData.ar):(modelData.labelEn||modelData.en);implicitHeight:Math.max(42,contentItem.implicitHeight+20);enabled:lawCaseBridge.permissions.training;contentItem:Text {text:choice.text;font:choice.font;wrapMode:Text.WordWrap;horizontalAlignment:Text.AlignHCenter;verticalAlignment:Text.AlignVCenter;color:page.app.design.text}onClicked:page.result=lawCaseBridge.evaluateTraining(page.scenario.id,modelData.id)}
   }
   Label {objectName:"lawTrainingResult";visible:page.result.valid===true;Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WordWrap;textFormat:Text.PlainText;color:page.result.correct?app.design.success:app.design.danger;text:page.app.trx("النتيجة: ","Score: ")+(page.result.score||0)+" · "+(page.app.ar?page.result.explanationAr:page.result.explanationEn)}
   Label {Layout.fillWidth:true;color:app.design.muted;text:lawCaseBridge.trainingAttempts.length+app.trx(" محاولة في هذه الجلسة"," attempts in this session")}
  }
 }
}
