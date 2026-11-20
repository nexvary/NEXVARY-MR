#include "RecoveryBridge.hpp"
#include "CaseVault.hpp"
#include "RecoveryImage.hpp"
#include "RecoveryFile.hpp"
#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QUrl>
#include <QUuid>
namespace {
constexpr qint64 maximumFile=8*1024*1024;
QString now(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
 }
QString local(const QString &value){
 if(value.isEmpty()||value.contains(QChar::Null)||value.startsWith("\\\\")||value.startsWith("//"))return {};
 for(const auto c:value)if(c.unicode()<32||c.unicode()==127)return {};
 const QUrl url(value);
 const auto path=url.isLocalFile()?url.toLocalFile():(url.scheme().isEmpty()||QFileInfo(value).isAbsolute()?value:QString());
 if(path.startsWith("\\\\")||path.startsWith("//"))return {};
#ifdef Q_OS_WIN
 if(path.mid(2).contains(QLatin1Char(':')))return {};
 static const QRegularExpression reserved(QStringLiteral("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\..*)?$"),QRegularExpression::CaseInsensitiveOption);
 if(reserved.match(QFileInfo(path).fileName()).hasMatch())return {};
#endif
 return path.isEmpty()?QString():QFileInfo(path).absoluteFilePath();
}
bool text(const QString &s,int limit,bool required=true){return s.size()<=limit&&!s.contains(QChar::Null)&&(!required||!s.trimmed().isEmpty());
 }
bool hexDigest(const QString &s){static const QRegularExpression re(QStringLiteral("^[0-9a-f]{64}$"));
 return re.match(s).hasMatch();
 }
QByteArray readBounded(const QString &path,qint64 maximum,QString &error){
 QFileInfo info(path);
 if(path.isEmpty()||!info.isFile()||info.isSymLink()||info.size()<0||info.size()>maximum){error=QStringLiteral("Choose a bounded regular local file, not a device, link or network path.");
 return {};
 }
 QFile f(path);
 if(!mr::recovery::openRegularInput(f)){error=f.errorString();
 return {};
 }
 const auto bytes=f.read(maximum+1);
 if(f.error()!=QFileDevice::NoError||bytes.size()>maximum||bytes.size()!=info.size()){error=QStringLiteral("Input changed size or could not be read completely.");
 return {};
 }
 QFileInfo after(path);
 if(after.size()!=info.size()||after.lastModified()!=info.lastModified()){error=QStringLiteral("Input changed while reading.");
 return {};
 }
 return bytes;
}
bool writeNew(const QString &path,const QByteArray &bytes,QString &error){
 QFile file(path);
 if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)){error=file.errorString();
 return false;
 }
 file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 if(file.write(bytes)!=bytes.size()||!file.flush()){error=file.errorString();
 file.close();
 file.remove();
 return false;
 }file.close();
 return true;
}
QJsonObject evidenceRecord(const QByteArray &bytes,const QString &name,const QString &kind,const QString &source,const QString &sourceDigest,const QString &offset){
 return {{"name",name},{"kind",kind},{"size",static_cast<double>(bytes.size())},{"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},{"source",source},{"sourceSha256",sourceDigest},{"offset",offset},{"collectedAtUtc",now()},{"payload",QString::fromLatin1(bytes.toBase64())}};
}
}
RecoveryBridge::RecoveryBridge(const QString &directory,bool syntheticTestProfile,QObject *parent):QObject(parent),directory_(local(directory)),synthetic_(syntheticTestProfile){
 status_=QStringLiteral("No recovery case is open. Assess write impact before installing any phone agent.");
 connect(&watcher_,&QFutureWatcher<JobResult>::finished,this,[this]{
  const auto result=watcher_.result();running_=false;
  if(result.error.isEmpty()){document_=result.document;if(!pendingPath_.isEmpty())path_=pendingPath_;error_.clear();progress_=1;status_=QStringLiteral("Completed. Review the evidence and limitations before exporting.");}
  else {if(pendingPath_.isEmpty()&&!result.document.isEmpty())document_=result.document;error_=result.error;status_=result.error;}
  pendingPath_.clear();
  emit changed();
 });
}
RecoveryBridge::~RecoveryBridge(){if(cancelled_)cancelled_->store(true);
 watcher_.waitForFinished();
 }
bool RecoveryBridge::vaultReady()const{return mr::recovery::vaultAvailable()||synthetic_;
 }
QString RecoveryBridge::protection()const{return mr::recovery::vaultAvailable()?QStringLiteral("Windows DPAPI · current account and computer"):(synthetic_?QStringLiteral("SYNTHETIC TEST PROFILE ONLY · not encrypted"):QStringLiteral("Windows encryption required; production cases disabled"));
 }
QVariantList RecoveryBridge::evidence()const{QVariantList result;
 for(const auto &v:document_.value("evidence").toArray()){auto o=v.toObject();
 o.remove("payload");
 result.append(o.toVariantMap());
 }return result;
 }
QVariantList RecoveryBridge::audit()const{return document_.value("audit").toArray().toVariantList();
 }
QVariantMap RecoveryBridge::device()const{return document_.value("device").toObject().toVariantMap();
 }
bool RecoveryBridge::fail(const QString &m){error_=m;
 status_=m;
 emit changed();
 return false;
 }
void RecoveryBridge::addAudit(QJsonObject &document,const QString &operation,const QString &outcome){auto list=document.value("audit").toArray();
 list.append(QJsonObject{{"atUtc",now()},{"operation",operation},{"outcome",outcome.left(2000)}});
 document.insert("audit",list);
 }
bool RecoveryBridge::validate(const QJsonObject &d,QString &error){
 const QRegularExpression uuid(QStringLiteral("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"));
 if(d.value("schema").toString()!="nexvary-recovery-case/v1"||!uuid.match(d.value("id").toString()).hasMatch()||!text(d.value("title").toString(),160)||!text(d.value("ownerReference").toString(),160)||!d.value("consent").toObject().value("confirmed").toBool()||!d.value("evidence").isArray()||!d.value("audit").isArray()||d.value("audit").toArray().size()>1000){error=QStringLiteral("Invalid case schema or consent record.");
 return false;
 }
 const auto entries=d.value("evidence").toArray();
 if(entries.size()>100){error=QStringLiteral("Case evidence limit exceeded.");
 return false;
 }
 qint64 total=0;
 for(const auto &v:entries){const auto o=v.toObject();
 const auto encoded=o.value("payload").toString().toLatin1();
 if(encoded.size()>((maximumFile+2)/3)*4){error=QStringLiteral("Evidence exceeds size limit.");
 return false;
 }
  const auto decoded=QByteArray::fromBase64Encoding(encoded,QByteArray::AbortOnBase64DecodingErrors);
  if(!decoded||decoded.decoded.size()>maximumFile||decoded.decoded.size()!=o.value("size").toDouble(-1)||!text(o.value("name").toString(),240)||!hexDigest(o.value("sha256").toString())||QString::fromLatin1(QCryptographicHash::hash(decoded.decoded,QCryptographicHash::Sha256).toHex())!=o.value("sha256").toString()){error=QStringLiteral("Evidence data/hash validation failed.");
 return false;
 }
  total+=decoded.decoded.size();
 if(total>64*1024*1024){error=QStringLiteral("Case exceeds 64MiB evidence limit.");
 return false;
 }
 }
 return true;
}
bool RecoveryBridge::persist(const QJsonObject &d,const QString &path,bool synthetic,QString &error){
 if(!validate(d,error))return false;
 const auto sealed=mr::recovery::sealCase(QJsonDocument(d).toJson(QJsonDocument::Compact),synthetic,error);
 if(sealed.isEmpty())return false;
 QSaveFile output(path);
 if(!output.open(QIODevice::WriteOnly)){error=output.errorString();
 return false;
 }output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 if(output.write(sealed)!=sealed.size()||!output.commit()){error=output.errorString();
 return false;
 }return true;
}
QString RecoveryBridge::assess(const QString &loss,const QString &encryption,bool external,bool backup,bool arabic)const{
 QStringList lines;
 if(arabic){
  lines<<QStringLiteral("هذا تقييم مشروط بالمعلومات المدخلة، وليس نسبة نجاح أو تشخيصًا مؤكدًا.");
  if(loss=="factory-reset")lines<<(encryption=="fbe"||encryption=="fde"?QStringLiteral("بعد ضبط المصنع، بقاء كتل مشفرة لا يثبت إمكانية استعادة الملفات القديمة. لم نثبت استعادة تخزين داخلي فُقدت مفاتيحه؛ Shizuku لا يعيد المفاتيح."):QStringLiteral("نوع التشفير أو تنفيذ المسح غير محسوم. يلزم الموديل والبناء ومسار الضبط وتجربة مستقلة؛ لا نعد باستعادة الذاكرة الداخلية."));
  else lines<<QStringLiteral("الملفات المقروءة حاليًا تُنسخ منطقيًا. استرجاع المحذوف يحتاج صورة مناسبة وبيانات باقية قابلة للتفسير؛ لا يُستنتج من مجرد اتصال ADB.");
  if(external)lines<<QStringLiteral("افحص بطاقة الذاكرة بنسخة صورة مستقلة؛ التخزين المعتمد كداخلي قد يعتمد على مفاتيح الهاتف.");
  if(backup)lines<<QStringLiteral("ابدأ بالنسخ الموجودة من المصادر التي يصرح بها المالك؛ هذا استرجاع من نسخة وليس إثباتًا لاستعادة المحذوف.");
  lines<<QStringLiteral("قلل الكتابة. قيّم أثر تثبيت Agent/Shizuku أولًا. لا يفتح البرنامج Bootloader ولا يستخدم Root ولا ينفذ ضبط المصنع.")<<QStringLiteral("المتاح: استيراد ملف مصرح به، تقرير Agent محلي، أو تحليل PNG/JPEG متجاورة في صورة خام. لا يوجد تحليل F2FS أو فك تشفير أو جمع أقسام خاصة.");
 }else{
  lines<<QStringLiteral("Conditional assessment from supplied facts, not a success rate or confirmed diagnosis.");
  if(loss=="factory-reset")lines<<(encryption=="fbe"||encryption=="fde"?QStringLiteral("After reset, surviving ciphertext does not prove old files are recoverable. Recovery of internal data with lost keys is not demonstrated; Shizuku does not restore keys."):QStringLiteral("Encryption/reset implementation is unknown. Exact model, build, reset route and an independent experiment are required; no internal-recovery promise."));
  else lines<<QStringLiteral("Currently readable files are logical copies. Undelete requires a suitable image and surviving interpretable data; ADB connectivity alone is insufficient.");
  if(external)lines<<QStringLiteral("Inspect an independent SD-card image; adopted/internal storage may depend on phone keys.");
  if(backup)lines<<QStringLiteral("Start with existing owner-authorized backups. Backup restoration is separate from undelete.");
  lines<<QStringLiteral("Minimize writes. Assess Agent/Shizuku installation impact first. No bootloader unlock, root or reset is performed.")<<QStringLiteral("Available: authorized file import, local Agent report, contiguous PNG/JPEG raw-image carving. No F2FS parser, decryption or protected-partition acquisition.");
 }
 return lines.join(QStringLiteral("\n\n"));
}
bool RecoveryBridge::createCase(const QString &title,const QString &owner,const QString &brand,const QString &model,const QString &androidVersion,const QString &loss,const QString &encryption,bool consent){
 if(busy())return fail(QStringLiteral("Wait for the current operation."));
 if(!vaultReady())return fail(protection());
 if(!consent||!text(title,160)||!text(owner,160)||!text(brand,120)||!text(model,120)||!text(androidVersion,80,false)||!QStringList{"deleted","filesystem-damage","factory-reset","backup"}.contains(loss)||!QStringList{"unknown","fbe","fde","none"}.contains(encryption))return fail(QStringLiteral("Record explicit owner consent, case/device identity and valid assessment values."));
 QJsonObject doc{{"schema","nexvary-recovery-case/v1"},{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"title",title.trimmed()},{"ownerReference",owner.trimmed()},{"createdAtUtc",now()},{"consent",QJsonObject{{"confirmed",true},{"atUtc",now()},{"scope","selected local evidence and image analysis"}}},{"device",QJsonObject{{"manufacturer",brand.trimmed()},{"model",model.trimmed()},{"release",androidVersion},{"identitySource","technician-supplied; unverified"}}},{"loss",loss},{"encryption",encryption},{"evidence",QJsonArray()},{"sources",QJsonArray()},{"audit",QJsonArray()},{"limits","No internal post-reset decryption, protected-partition acquisition, root or bootloader operations. Carved files are candidates, not proof of deletion or original ownership."}};
 addAudit(doc,"case-created","Explicit owner authorization recorded; installation write-impact warning acknowledged.");
 if(!QDir().mkpath(directory_))return fail(QStringLiteral("Could not create local case directory."));
 const auto destination=QDir(directory_).filePath(doc.value("id").toString()+".nmrcase");
 QString e;
 if(QFileInfo::exists(destination)||!persist(doc,destination,synthetic_,e))return fail(e.isEmpty()?QStringLiteral("Case filename already exists."):e);
 document_=doc;
 path_=destination;
 error_.clear();
 status_=QStringLiteral("Case saved locally. Choose an explicitly authorized source.");
 progress_=0;
 emit changed();
 return true;
}
bool RecoveryBridge::start(Job job,const QString &destination){
 if(busy())return fail(QStringLiteral("A recovery operation is already running."));
 const QString target=destination.isEmpty()?path_:destination;
 if(target.isEmpty())return fail(QStringLiteral("Open a case first."));
 const auto snapshot=document_;
 const bool synthetic=synthetic_,opening=!destination.isEmpty();
 pendingPath_=destination;
 cancelled_=std::make_shared<std::atomic_bool>(false);
 const auto cancelled=cancelled_;
 running_=true;
 progress_=0;
 error_.clear();
 status_=QStringLiteral("Reading authorized source; original bytes are not modified.");
 emit changed();
 watcher_.setFuture(QtConcurrent::run([job,snapshot,target,synthetic,cancelled,opening]{
  auto result=job(snapshot,cancelled);
  if(cancelled->load())result.error=QStringLiteral("Operation cancelled; no result committed.");
  if(result.error.isEmpty()&&!persist(result.document,target,synthetic,result.error))result.document={};
  if(!result.error.isEmpty()&&!opening&&!snapshot.isEmpty()){
   result.document=snapshot;addAudit(result.document,"operation-failed",result.error);QString auditError;
   if(!persist(result.document,target,synthetic,auditError)){result.document={};result.error+=QStringLiteral(" Audit update failed: ")+auditError;}
  }
  return result;
 }));
 // Set only a new open destination, never a source image or export path.
 return true;
}
bool RecoveryBridge::openCase(const QString &value){
 if(busy())return fail(QStringLiteral("Wait for the current operation."));
 const auto path=local(value);
 if(path.isEmpty())return fail(QStringLiteral("Choose a local case file."));
 const bool synthetic=synthetic_;
 return start([path,synthetic](const QJsonObject &,const auto &cancelled){
  JobResult r;const auto bytes=readBounded(path,97*1024*1024,r.error);if(!r.error.isEmpty()||cancelled->load())return r;
  const auto plain=mr::recovery::openCaseBytes(bytes,synthetic,r.error);if(!r.error.isEmpty())return r;
  QJsonParseError pe;const auto json=QJsonDocument::fromJson(plain,&pe);if(pe.error!=QJsonParseError::NoError||!json.isObject()){r.error=QStringLiteral("Invalid case JSON.");return r;}r.document=json.object();if(!validate(r.document,r.error))return r;addAudit(r.document,"case-opened","All embedded SHA256 digests validated.");return r;
 },path);
}
bool RecoveryBridge::importVisibleFile(const QString &value,bool consent){
 if(!active()||!consent)return fail(QStringLiteral("Open a case and confirm permission for this selected file."));
 const auto path=local(value);
 if(path.isEmpty()||path==path_)return fail(QStringLiteral("Choose a local evidence file separate from the case vault."));
 return start([path](const QJsonObject &snapshot,const auto &cancelled){
  JobResult r{snapshot,{}};const auto bytes=readBounded(path,maximumFile,r.error);if(!r.error.isEmpty()||cancelled->load())return r;
  QString secondError;const auto second=readBounded(path,maximumFile,secondError);if(!secondError.isEmpty()||QCryptographicHash::hash(bytes,QCryptographicHash::Sha256)!=QCryptographicHash::hash(second,QCryptographicHash::Sha256)){r.error=QStringLiteral("Source changed; file import refused.");return r;}
  const auto digest=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());auto list=r.document.value("evidence").toArray();list.append(evidenceRecord(bytes,QFileInfo(path).fileName(),"logical-copy",path,digest,QStringLiteral("0")));r.document.insert("evidence",list);addAudit(r.document,"logical-file-copy","Explicit selected-file consent; complete bytes hashed twice; not an undelete claim.");return r;
 });
}
bool RecoveryBridge::importAgent(const QString &value,bool consent){
 if(!active()||!consent)return fail(QStringLiteral("Open a case and confirm Agent report import."));
 const auto path=local(value);
 if(path.isEmpty()||path==path_)return fail(QStringLiteral("Choose a separate local Agent report."));
 return start([path](const QJsonObject &snapshot,const auto &cancelled){
  JobResult r{snapshot,{}};const auto bytes=readBounded(path,128*1024,r.error);if(!r.error.isEmpty()||cancelled->load())return r;
  const auto json=QJsonDocument::fromJson(bytes);const auto o=json.object(),dev=o.value("device").toObject(),consentObject=o.value("consent").toObject();const auto sdk=dev.value("sdkInt").toInt(-1);
  if(!json.isObject()||o.value("schema").toString()!="nexvary-recovery-agent/v1"||!consentObject.value("confirmed").toBool()||consentObject.value("scope").toString()!="device-metadata"||!text(dev.value("manufacturer").toString(),128)||!text(dev.value("model").toString(),128)||sdk<26||sdk>100){r.error=QStringLiteral("Invalid Agent protocol, device metadata or consent scope.");return r;}
  const auto shizuku=o.value("shizuku").toObject();const auto state=shizuku.value("state").toString();
  const bool shellIdentity=QStringList{"permission-required","denied","shell"}.contains(state);
  if(!QStringList{"unavailable","permission-required","denied","shell","unsupported-root"}.contains(state)||(shellIdentity?shizuku.value("uid").toDouble(-1)!=2000:!shizuku.value("uid").isNull())){r.error=QStringLiteral("Invalid Shizuku identity/state.");return r;}
  // Metadata is untrusted owner-supplied evidence, not authenticated device identity.
  auto safeDevice=QJsonObject{{"manufacturer",dev.value("manufacturer")},{"model",dev.value("model")},{"sdkInt",sdk},{"release",dev.value("release").toString().left(80)},{"securityPatch",dev.value("securityPatch").toString().left(80)},{"identitySource","imported unsigned Agent report; unverified"}};r.document.insert("device",safeDevice);
  auto entries=r.document.value("evidence").toArray();entries.append(evidenceRecord(bytes,QFileInfo(path).fileName(),"agent-metadata",path,QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()),"0"));r.document.insert("evidence",entries);addAudit(r.document,"agent-report-import","Offline JSON and SHA256 recorded; unsigned report does not prove device origin or historical data recovery.");return r;
 });
}
bool RecoveryBridge::scanImage(const QString &value,bool consent){
 if(!active()||!consent)return fail(QStringLiteral("Open a case and confirm permission for this image."));
 const auto path=local(value);
 if(path.isEmpty()||path==path_)return fail(QStringLiteral("Choose a separate raw image file."));
 return start([this,path](const QJsonObject &snapshot,const auto &cancelled){
  JobResult r{snapshot,{}};double last=-1;
  const auto result=mr::recovery::scanImage(path,{},cancelled,[this,&last](qint64 done,qint64 total){const double p=total>0?static_cast<double>(done)/static_cast<double>(total):0;if(p-last<0.005&&done!=total)return;last=p;QMetaObject::invokeMethod(this,[this,p]{progress_=p;emit changed();},Qt::QueuedConnection);});
  if(!result.error.isEmpty()||result.cancelled||!result.sourceUnchanged){r.error=result.error.isEmpty()?QStringLiteral("Source was not verified unchanged; no candidates committed."):result.error;return r;}
  const auto digest=QString::fromLatin1(result.sourceSha256.toHex());auto entries=r.document.value("evidence").toArray();
  for(const auto &c:result.candidates)entries.append(evidenceRecord(c.bytes,QStringLiteral("candidate_%1.%2").arg(c.offset).arg(c.format),"carved-candidate",path,digest,QString::number(c.offset)));
  r.document.insert("evidence",entries);auto sources=r.document.value("sources").toArray();sources.append(QJsonObject{{"path",path},{"sha256",digest},{"verifiedUnchanged",true},{"candidateCount",static_cast<int>(result.candidates.size())},{"limited",result.limited},{"method","native contiguous PNG/JPEG signature and image decode; no filesystem interpretation"},{"atUtc",now()}});r.document.insert("sources",sources);
  addAudit(r.document,"image-analysis",QStringLiteral("Source SHA256 unchanged; %1 candidates; limited=%2. No inference of deletion, ownership or post-reset recovery.").arg(result.candidates.size()).arg(result.limited));return r;
 });
}
bool RecoveryBridge::allowedOutput(const QString &path)const{
 if(path.isEmpty()||QFileInfo::exists(path)||QFileInfo(path).isSymLink())return false;
 auto equal=[](const QString &a,const QString &b){
#ifdef Q_OS_WIN
 return QString::compare(QDir::cleanPath(a),QDir::cleanPath(b),Qt::CaseInsensitive)==0;
#else
 return QDir::cleanPath(a)==QDir::cleanPath(b);
#endif
 };
 if(equal(path,path_))return false;
 for(const auto &v:document_.value("evidence").toArray())if(equal(path,v.toObject().value("source").toString()))return false;
 for(const auto &v:document_.value("sources").toArray())if(equal(path,v.toObject().value("path").toString()))return false;
 return true;
}
bool RecoveryBridge::exportReport(const QString &value,bool confirmed){
 if(!active()||busy()||!confirmed)return fail(QStringLiteral("Explicit confirmation is required for an unencrypted report export."));
 const auto path=local(value);
 if(!allowedOutput(path))return fail(QStringLiteral("Choose a new output file separate from all originals and the case."));
 auto report=document_;
 auto entries=report.value("evidence").toArray();
 for(qsizetype i=0;i<entries.size();++i){auto o=entries[i].toObject();
 o.remove("payload");
 entries[i]=o;
 }report.insert("evidence",entries);
 report.insert("reportGeneratedAtUtc",now());
 report.insert("protection","PLAINTEXT EXPORT: protect this file before sharing.");
 QString e;
 if(!writeNew(path,QJsonDocument(report).toJson(),e))return fail(e);
 auto doc=document_;
 addAudit(doc,"report-export","Explicit plaintext export authorized.");
 if(!persist(doc,path_,synthetic_,e))return fail(QStringLiteral("Report exported, but audit update failed: ")+e);
 document_=doc;
 error_.clear();
 status_=QStringLiteral("Plaintext report exported. The original case remains encrypted on Windows.");
 emit changed();
 return true;
}
bool RecoveryBridge::exportEvidence(int index,const QString &value,bool confirmed){
 const auto entries=document_.value("evidence").toArray();
 if(!active()||busy()||!confirmed||index<0||index>=entries.size())return fail(QStringLiteral("Select evidence and explicitly confirm an unencrypted export."));
 const auto path=local(value);
 if(!allowedOutput(path))return fail(QStringLiteral("Choose a new export file, separate from originals."));
 const auto record=entries[index].toObject();
 const auto bytes=QByteArray::fromBase64(record.value("payload").toString().toLatin1());
 if(QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())!=record.value("sha256").toString())return fail(QStringLiteral("Evidence SHA256 mismatch; export refused."));
 QString e;
 if(!writeNew(path,bytes,e))return fail(e);
 auto doc=document_;
 addAudit(doc,"evidence-export",QStringLiteral("Explicit plaintext export; SHA256 %1").arg(record.value("sha256").toString()));
 if(!persist(doc,path_,synthetic_,e))return fail(QStringLiteral("Evidence exported, but audit update failed: ")+e);
 document_=doc;
 error_.clear();
 status_=QStringLiteral("Evidence exported with matching SHA256. Protect the plaintext copy.");
 emit changed();
 return true;
}
bool RecoveryBridge::deleteCase(bool confirmed){
 if(!active()||busy()||!confirmed)return fail(QStringLiteral("Confirm deletion of the selected local case container."));
 if(!QFile::remove(path_))return fail(QStringLiteral("Could not delete the case container."));
 document_={};
 path_.clear();
 error_.clear();
 progress_=0;
 status_=QStringLiteral("Case container deleted. Original sources and exported copies remain. Secure media erasure is not claimed.");
 emit changed();
 return true;
}
void RecoveryBridge::cancel(){if(cancelled_)cancelled_->store(true);
 if(busy()){status_=QStringLiteral("Cancelling; incomplete results will not be committed.");
 emit changed();
 }}
bool RecoveryBridge::closeCase(){
 if(busy())return fail(QStringLiteral("Wait for the current operation before closing this case."));
 document_={};
 path_.clear();
 error_.clear();
 progress_=0;
 status_=QStringLiteral("Case closed. Its saved container remains on disk.");
 emit changed();
 return true;
}
