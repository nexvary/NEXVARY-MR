import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
OsintDesktopPane {
 id:panel
 objectName:"phoneOsintPanel"
 property var app
 readonly property var report:lawCaseBridge.osintResult.report||({})
 title:app.trx("استخبارات المصادر المفتوحة — أرقام الهاتف","OSINT Intelligence — Phone numbers")
 iconName:"shield_checkmark"
 LayoutMirroring.enabled:app.ar
 LayoutMirroring.childrenInherit:true
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("يلزم فتح قضية وتسجيل جهاز، ثم اعتماد استثناء external-analysis يشير إلى UUID القضية ويتضمن الرقم الدولي كاملًا في سبب الطلب. كل اعتماد يسمح ببحث واحد فقط.","Open a case and register a device, then approve an external-analysis exception referencing the case UUID and the exact international number in its reason. Each approval permits one search.")}
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.danger;text:panel.app.trx("النتائج غير متحققة: لا تثبت هوية المشترك أو الموقع الحي أو سجلات الاتصالات. روابط Google المقترحة ليست نتائج بحث. PhoneInfoga غير مُصان.","Unverified results: no confirmed subscriber identity, live location or telecom records. Generated Google links are not search hits. PhoneInfoga is unmaintained.")}
 ComboBox {palette.base:"#08121B";palette.text:"#E5EDF3";palette.button:"#0D2A3D";palette.buttonText:"#E5EDF3";id:device;objectName:"osintDevice";Layout.fillWidth:true;model:lawCaseBridge.devices;textRole:"model";enabled:!lawCaseBridge.osintBusy}
 TextField {color:"#E5EDF3";palette.base:"#08121B";palette.placeholderText:"#9EB4C1";id:number;objectName:"osintNumber";Layout.fillWidth:true;maximumLength:64;selectByMouse:true;LayoutMirroring.enabled:false;horizontalAlignment:TextInput.AlignLeft;enabled:!lawCaseBridge.osintBusy;placeholderText:panel.app.trx("رقم دولي واحد مثل ‎+201xxxxxxxxx","One international number, e.g. +201xxxxxxxxx")}
 Flow {Layout.fillWidth:true;spacing:12
  CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:local;checked:true;text:panel.app.trx("بيانات خطة الترقيم المحلية","Offline numbering plan");enabled:!lawCaseBridge.osintBusy}
  CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:google;text:panel.app.trx("روابط بحث مقترحة","Generated search links");enabled:!lawCaseBridge.osintBusy}
  CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:ovh;text:"OVH";enabled:!lawCaseBridge.osintBusy}
  CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:numverify;text:"Numverify";enabled:!lawCaseBridge.osintBusy}
  CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:googlecse;text:"Google CSE";enabled:!lawCaseBridge.osintBusy}
 }
 Label {Layout.fillWidth:true;wrapMode:Text.WordWrap;color:panel.app.design.muted;text:panel.app.trx("تُعرض أخطاء الخدمات أو المفاتيح غير المتاحة صراحةً. لا يستورد المحرك مفاتيح من بيئة النظام. OSINT Industries معطل لحين اعتماد ترخيصه.","Unavailable services or missing keys are reported explicitly. The engine does not inherit service keys. OSINT Industries remains disabled pending an approved license.")}
 TextField {color:"#E5EDF3";palette.base:"#08121B";palette.placeholderText:"#9EB4C1";id:engine;objectName:"osintEngine";Layout.fillWidth:true;maximumLength:4096;selectByMouse:true;enabled:!lawCaseBridge.osintBusy;placeholderText:panel.app.trx("المسار الكامل لمحرك PhoneInfoga المعدل والمراجع","Absolute path to the reviewed, hardened PhoneInfoga engine")}
 TextField {color:"#E5EDF3";palette.base:"#08121B";palette.placeholderText:"#9EB4C1";id:engineHash;objectName:"osintEngineHash";Layout.fillWidth:true;maximumLength:64;selectByMouse:true;enabled:!lawCaseBridge.osintBusy;placeholderText:panel.app.trx("SHA-256 من إيصال بناء موثوق","SHA-256 from a trusted build receipt")}
 CheckBox {palette.windowText:"#E5EDF3";palette.buttonText:"#E5EDF3";palette.text:"#E5EDF3";id:consent;objectName:"osintConsent";enabled:!lawCaseBridge.osintBusy;text:panel.app.trx("أؤكد تفويض البحث وحفظ تقرير المصادر ضمن هذه القضية","Confirm search authority and retention of source observations in this case");contentItem:Label {text:consent.text;wrapMode:Text.WordWrap;color:panel.app.design.text;leftPadding:consent.mirrored?0:consent.indicator.width+consent.spacing;rightPadding:consent.mirrored?consent.indicator.width+consent.spacing:0}}
 Flow {Layout.fillWidth:true;spacing:8
  OsintDesktopCommand {objectName:"osintRun";primary:true;text:panel.app.trx("بحث وتوثيق","Investigate and retain");enabled:!!lawCaseBridge.activeCase.id&&lawCaseBridge.permissions.analyze&&lawCaseBridge.permissions.addEvidence&&consent.checked&&!lawCaseBridge.osintBusy;onClicked:{let scanners=[];if(local.checked)scanners.push("local");if(google.checked)scanners.push("googlesearch");if(ovh.checked)scanners.push("ovh");if(numverify.checked)scanners.push("numverify");if(googlecse.checked)scanners.push("googlecse");lawCaseBridge.runPhoneOsint(number.text,scanners,engine.text,engineHash.text,device.currentIndex>=0?device.model[device.currentIndex].id:"",consent.checked);consent.checked=false}}
  OsintDesktopCommand {objectName:"osintCancel";text:panel.app.trx("إلغاء وإيقاف المحرك","Cancel and stop engine");enabled:lawCaseBridge.osintBusy;onClicked:lawCaseBridge.cancelOsint()}
  BusyIndicator {visible:lawCaseBridge.osintBusy;running:visible}
 }
 Column {id:rows;Layout.fillWidth:true;spacing:8
  Repeater {model:panel.report.scanners||[];delegate:OsintDesktopPane {required property var modelData;width:rows.width;title:modelData.scanner;iconName:"document_text"
   Label {Layout.fillWidth:true;textFormat:Text.PlainText;color:panel.app.design.text;wrapMode:Text.WrapAnywhere;text:modelData.status+" · "+modelData.classification+"\n"+modelData.retrievedUtc+"\n"+modelData.source+(modelData.error?"\n"+modelData.error:"")+(modelData.providerError?"\n"+modelData.providerError:"")}
   TextArea {color:"#E5EDF3";palette.base:"#08121B";Layout.fillWidth:true;readOnly:true;selectByMouse:true;textFormat:TextEdit.PlainText;wrapMode:TextEdit.WrapAnywhere;text:JSON.stringify(modelData.result,null,2)||""}
  }}
 }
 Label {visible:!!lawCaseBridge.osintResult.evidenceId;Layout.fillWidth:true;wrapMode:Text.WrapAnywhere;textFormat:Text.PlainText;color:panel.app.design.success;text:panel.app.trx("حُفظ التقرير كدليل محمي. صدّره من شاشة الأدلة بالتفويض المعتاد. SHA-256: ","Report retained as protected evidence. Export through the evidence workspace with its normal authorization. SHA-256: ")+(lawCaseBridge.osintResult.reportSha256||"")}
 Connections {target:lawCaseBridge;function onChanged(){consent.checked=false;if(!lawCaseBridge.activeCase.id)number.text=""}}
}
