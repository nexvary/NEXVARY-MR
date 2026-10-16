import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 property var app
 ColumnLayout {
  anchors.fill:parent;spacing:14
  Panel {
   title:app.trx("مسار التشخيص الإرشادي","Guided diagnostic workflow");Layout.fillWidth:true;Layout.fillHeight:true
   Label {text:app.trx("1  حدد العَرَض   ←   2  اجمع الأدلة   ←   3  وثّق النتيجة في أمر الصيانة","1  Select symptom   →   2  Collect evidence   →   3  Document repair order");color:"#69b7ff";Layout.fillWidth:true;wrapMode:Text.WordWrap}
   ComboBox {id:symptom;objectName:"symptomChoice";Layout.fillWidth:true;model:["Bootloop","Not charging","No network","USB not detected"]}
   ActionButton {objectName:"diagnoseButton";text:app.trx("تحليل العَرَض","Analyze symptom");primary:true;onClicked:result.text=repairBridge.diagnoseLocalized(symptom.currentText,app.ar)}
   TextArea {id:result;objectName:"diagnosisResult";Layout.fillWidth:true;Layout.fillHeight:true;readOnly:true;selectByMouse:true;wrapMode:TextEdit.Wrap;text:repairBridge.diagnoseLocalized(symptom.currentText,app.ar);color:"#d0e0ef";font.pixelSize:17;background:Rectangle{color:"#0c2232";radius:6}}
   Label {text:app.trx("إرشادات قائمة على قواعد، وليست تشخيصًا مؤكدًا أو مساعد ذكاء اصطناعي متصلًا.","Rule-based guidance, not a confirmed diagnosis or connected AI service.");color:"#eab775";wrapMode:Text.WordWrap;Layout.fillWidth:true}
  }
  Panel {
   title:app.trx("مكتبة الشركات والعائلات","Brands and device families");Layout.fillWidth:true;Layout.preferredHeight:205
   TextField {id:catalogSearch;placeholderText:app.trx("البحث عن شركة أو عائلة أجهزة","Search brand or device family");Layout.fillWidth:true}
   ListView {
    Layout.fillWidth:true;Layout.fillHeight:true;clip:true;model:repairBridge.brands()
    delegate:Label {required property var modelData;width:ListView.view.width;visible:(modelData.brand+modelData.families.join(" ")).toLowerCase().indexOf(catalogSearch.text.toLowerCase())>=0;height:visible?32:0;text:modelData.brand+"   /   "+modelData.families.join(" · ");color:"#a8bed3";textFormat:Text.PlainText;elide:Text.ElideRight}
   }
  }
 }
}
