import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ScrollView {
 id:page
 property var app
 property var form:({})
 contentWidth:availableWidth;clip:true
 function setField(key,value){let next=Object.assign({},form);next[key]=value;form=next}
 ColumnLayout {width:page.availableWidth;spacing:12
  Panel {title:app.trx("تسجيل الجهاز دون تغييره","Register a device without modifying it");iconName:"phone";Layout.fillWidth:true
   Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:app.design.muted;text:app.trx("سجل البيانات المعلومة أو استورد المشاهدات الموجودة من Device Intelligence. لا يتم تشغيل ADB أو تغيير حالة الهاتف من هذه الشاشة. الأرقام والهوية تصريحات تحتاج مراجعة.","Enter known details or copy existing Device Intelligence observations. This screen starts no ADB process or phone-state change. Identifiers and declarations require review.")}
   ActionButton {objectName:"lawUseDeviceObservation";text:app.trx("استخدام المشاهدات المقروءة حاليًا","Use currently collected observations");enabled:toolBridge.selectedSerial!==""&&lawCaseBridge.permissions.addDevice;onClicked:page.form=Object.assign({},page.form,lawCaseBridge.currentDeviceObservation())}
   GridLayout {Layout.fillWidth:true;columns:width>800?2:1;columnSpacing:12;rowSpacing:10
    Repeater {model:[{key:"manufacturer",ar:"الشركة",en:"Manufacturer"},{key:"model",ar:"الموديل",en:"Model"},{key:"serial",ar:"الرقم التسلسلي إن عُرف",en:"Serial if known"},{key:"imei",ar:"IMEI — اختياري ومصرح",en:"IMEI — optional, authorized"},{key:"condition",ar:"حالة الجهاز",en:"Device condition"},{key:"powerState",ar:"حالة التشغيل",en:"Power state"},{key:"connectionState",ar:"حالة الاتصال",en:"Connection state"},{key:"lockState",ar:"حالة القفل المرصودة",en:"Observed lock state"},{key:"receivedAtUtc",ar:"وقت الاستلام UTC",en:"Received UTC"},{key:"custodian",ar:"المسؤول عن الجهاز",en:"Device custodian"},{key:"accessories",ar:"الملحقات",en:"Accessories"},{key:"notes",ar:"ملاحظات الفحص",en:"Examination notes"}]
     delegate:ColumnLayout {required property var modelData;Layout.fillWidth:true;spacing:4
      Label {text:page.app.ar?modelData.ar:modelData.en;color:page.app.design.muted;Layout.fillWidth:true;wrapMode:Text.WordWrap}
      TextField {objectName:"lawDevice-"+modelData.key;Layout.fillWidth:true;maximumLength:1024;text:page.form[modelData.key]||"";onTextEdited:page.setField(modelData.key,text)}
     }
    }
   }
   ActionButton {objectName:"lawAddDevice";text:app.trx("ربط الجهاز بالقضية","Link device to case");primary:true;enabled:lawCaseBridge.ready&&!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.addDevice;onClicked:if(lawCaseBridge.addDevice(page.form)!=="")page.form={}}
   Label {visible:!lawCaseBridge.activeCase.id;Layout.fillWidth:true;color:app.design.muted;text:app.trx("افتح قضية أولًا.","Open a case first.")}
  }
  Repeater {model:lawCaseBridge.devices
   delegate:Panel {required property var modelData;Layout.fillWidth:true;title:page.app.trx("جهاز مسجل","Registered device");iconName:"phone"
    Label {Layout.fillWidth:true;Layout.minimumHeight:implicitHeight;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:page.app.design.text;text:JSON.stringify(modelData,null,2)}
   }
  }
 }
}
