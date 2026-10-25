#include "ImportAdapters.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringDecoder>
#include <QXmlStreamReader>
#include <functional>
#include <algorithm>

namespace mr::forensic {
namespace {
QString hash(const QByteArray &bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
QString shown(const QString &input){
 QString out;
 for(const auto c:input){const auto u=c.unicode();if(u<32||u==127||(u>=0x202a&&u<=0x202e)||(u>=0x2066&&u<=0x2069))out+=QStringLiteral("\\u%1").arg(u,4,16,QChar('0'));else out+=c;}
 return out;
}
struct Findings {
 QVariantList rows;int count=0;
 void add(const QString &reference,const QString &kind,const QString &value){
  ++count;if(rows.size()>=importFindingLimit)return;
  const bool cut=value.size()>256||reference.size()>512;
  rows.append(QVariantMap{{"reference",shown(reference.left(512))},{"kind",kind},{"value",shown(value.left(256))},{"valueTruncated",cut}});
 }
};
bool textInput(const QByteArray &bytes,QString &text){
 QStringDecoder decoder(QStringDecoder::Utf8);text=decoder(bytes);
 if(decoder.hasError())return false;
 if(text.startsWith(QChar(0xfeff)))text.remove(0,1);
 for(const auto c:text)if((c.unicode()<32&&c!='\r'&&c!='\n'&&c!='\t')||c.unicode()==127)return false;
 return true;
}
bool jsonStructure(const QByteArray &bytes){
 // Validate depth and duplicate decoded object keys before Qt constructs a tree.
 QList<QSet<QString>> keys;int tokens=0;
 for(qsizetype i=0;i<bytes.size();++i){const auto c=bytes[i];
  if(c=='{'||c=='['){if(keys.size()>=32)return false;keys.append(QSet<QString>());if(++tokens>20000)return false;}
  else if(c=='}'||c==']'){if(keys.isEmpty())return false;keys.removeLast();}
  else if(c=='"'){
   const auto begin=i++;bool ended=false;
   for(;i<bytes.size();++i){if(bytes[i]=='\\'){++i;continue;}if(bytes[i]=='"'){ended=true;break;}}
   if(!ended)return false;
   auto next=i+1;while(next<bytes.size()&&(bytes[next]==' '||bytes[next]=='\t'||bytes[next]=='\r'||bytes[next]=='\n'))++next;
   if(next<bytes.size()&&bytes[next]==':'){
    if(keys.isEmpty())return false;
    QJsonParseError e;const auto key=QJsonDocument::fromJson("["+bytes.mid(begin,i-begin+1)+"]",&e);
    if(e.error!=QJsonParseError::NoError||!key.isArray()||key.array().size()!=1||!key.array()[0].isString())return false;
    const auto value=key.array()[0].toString();if(keys.last().contains(value))return false;keys.last().insert(value);
   }
   if(++tokens>20000)return false;
  }else if(c==','&&++tokens>20000)return false;
 }
 return keys.isEmpty();
}
bool jsonImport(const QByteArray &bytes,Findings &findings){
 if(!jsonStructure(bytes))return false;
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
 if(error.error!=QJsonParseError::NoError||doc.isNull())return false;
 std::function<void(const QJsonValue&,const QString&)> walk;
 walk=[&](const QJsonValue &v,const QString &path){
  if(v.isObject()){const auto o=v.toObject();auto keys=o.keys();keys.sort();for(const auto &key:keys){auto part=key;part.replace("~","~0");part.replace("/","~1");walk(o.value(key),path+"/"+part);}}
  else if(v.isArray()){const auto a=v.toArray();for(qsizetype i=0;i<a.size();++i)walk(a[i],path+"/"+QString::number(i));}
  else if(v.isString())findings.add(path,"string",v.toString());
  else if(v.isNull())findings.add(path,"null","null");
  else if(v.isBool())findings.add(path,"boolean",v.toBool()?"true":"false");
  else findings.add(path,"number",QString::fromUtf8(QJsonDocument(QJsonArray{v}).toJson(QJsonDocument::Compact)).mid(1).chopped(1));
 };
 walk(doc.isObject()?QJsonValue(doc.object()):QJsonValue(doc.array()),QString());return true;
}
bool csvImport(const QString &text,Findings &findings){
 QString field;int row=1,column=1;bool quoted=false,closed=false,atStart=true,dirty=false;
 const auto cell=[&]{findings.add(QStringLiteral("row/%1/column/%2").arg(row).arg(column),"csv-cell",field);field.clear();atStart=true;closed=false;};
 for(qsizetype i=0;i<text.size();++i){const auto c=text[i];dirty=true;
  if(quoted){if(c=='"'){if(i+1<text.size()&&text[i+1]=='"'){field+='"';++i;}else{quoted=false;closed=true;}}else field+=c;continue;}
  if(c=='"'){if(!atStart||closed)return false;quoted=true;atStart=false;continue;}
  if(c==','){cell();if(++column>128)return false;continue;}
  if(c=='\n'||c=='\r'){
   if(c=='\r'){if(i+1>=text.size()||text[i+1]!='\n')return false;++i;}
   cell();if(++row>20000)return false;column=1;dirty=false;continue;
  }
  if(closed)return false;
  atStart=false;field+=c;
 }
 if(quoted)return false;
 if(dirty)cell();
 return true;
}
bool xmlImport(const QByteArray &bytes,Findings &findings){
 QXmlStreamReader xml(bytes);int depth=0,nodes=0;QStringList paths;
 while(!xml.atEnd()){
  const auto token=xml.readNext();
  if(token==QXmlStreamReader::StartDocument&&!xml.documentEncoding().isEmpty()&&xml.documentEncoding().compare(QStringLiteral("UTF-8"),Qt::CaseInsensitive)!=0)return false;
  if(token==QXmlStreamReader::DTD||token==QXmlStreamReader::EntityReference)return false;
  if(token==QXmlStreamReader::StartElement){
   if(++depth>32||++nodes>20000)return false;
   paths.append(xml.qualifiedName().toString());const auto path=paths.join('/');
   findings.add(QStringLiteral("line/%1/%2").arg(xml.lineNumber()).arg(path),"xml-element",xml.namespaceUri().toString());
   auto attributes=xml.attributes();std::sort(attributes.begin(),attributes.end(),[](const auto &a,const auto &b){return a.qualifiedName()<b.qualifiedName();});
   for(const auto &a:attributes)findings.add(QStringLiteral("line/%1/%2/@%3").arg(xml.lineNumber()).arg(path,a.qualifiedName().toString()),"xml-attribute",a.value().toString());
  }else if(token==QXmlStreamReader::EndElement){--depth;paths.removeLast();}
  else if(token==QXmlStreamReader::Characters&&!xml.isWhitespace())findings.add(QStringLiteral("line/%1/%2").arg(xml.lineNumber()).arg(paths.join('/')),"xml-text",xml.text().toString());
 }
 return !xml.hasError()&&nodes>0;
}
bool sqliteImport(const QByteArray &bytes,const QString &path,Findings &findings){
 if(bytes.size()<100||!bytes.startsWith(QByteArray("SQLite format 3\0",16))||bytes[18]!=1||bytes[19]!=1)return false;
 bool ok=false;
 {
  auto db=QSqlDatabase::addDatabase("QSQLITE","isolated-import");db.setDatabaseName(path);db.setConnectOptions("QSQLITE_OPEN_READONLY;QSQLITE_BUSY_TIMEOUT=100");
  if(db.open()){
   QSqlQuery q(db);ok=q.exec("PRAGMA query_only=ON")&&q.exec("PRAGMA trusted_schema=OFF")&&q.exec("PRAGMA quick_check(1)")&&q.next()&&q.value(0).toString()=="ok";
   QList<QVariantMap> tables;
   if(ok){ok=q.exec("PRAGMA table_list");while(ok&&q.next()){if(q.value(0).toString()!="main"||q.value(1).toString().startsWith("sqlite_"))continue;
     if(tables.size()>=32){ok=false;break;}tables.append({{"name",q.value(1)},{"type",q.value(2)},{"withoutRowid",q.value(4)}});
   }}
   std::sort(tables.begin(),tables.end(),[](const auto &a,const auto &b){return a.value("name").toString()<b.value("name").toString();});
   for(const auto &table:tables){if(!ok)break;
    const auto name=table.value("name").toString();findings.add("table/"+name,"sqlite-table",table.value("type").toString());
    if(table.value("type")!="table"||table.value("withoutRowid").toInt()!=0){findings.add("table/"+name,"sqlite-skipped","Views, virtual/shadow and WITHOUT ROWID tables are not sampled");continue;}
    auto quoted=name;quoted.replace('"',"\"\"");quoted='"'+quoted+'"';
    QSqlQuery columns(db);if(!columns.exec("PRAGMA table_info("+quoted+")")){ok=false;break;}
    QStringList names;while(columns.next()){if(names.size()>=128){ok=false;break;}names.append(columns.value(1).toString());}
    if(!ok)break;
    QString rowid;for(const auto &candidate:{QString("rowid"),QString("_rowid_"),QString("oid")})if(!names.contains(candidate,Qt::CaseInsensitive)){rowid=candidate;break;}
    if(rowid.isEmpty()){findings.add("table/"+name,"sqlite-skipped","All rowid aliases are shadowed");continue;}
    QStringList selected;for(const auto &column:names.mid(0,8)){auto escaped=column;escaped.replace('"',"\"\"");selected.append('"'+escaped+'"');}
    if(selected.isEmpty())continue;
    QSqlQuery rows(db);if(!rows.exec("SELECT "+rowid+","+selected.join(',')+" FROM "+quoted+" ORDER BY "+rowid+" LIMIT 33")){ok=false;break;}
    int sampled=0;while(rows.next()){
     if(++sampled>32){findings.add("table/"+name,"sqlite-limit","Only first 32 ordered rows sampled");break;}
     for(qsizetype column=0;column<selected.size();++column){const auto value=rows.value(static_cast<int>(column)+1);QString type="sqlite-value",text=value.toString();
      if(value.isNull()){type="null";text="null";}else if(value.metaType().id()==QMetaType::QByteArray){type="sqlite-blob-sha256";text=hash(value.toByteArray());}
      findings.add(QStringLiteral("table/%1/rowid/%2/column/%3").arg(name,rows.value(0).toString(),names[column]),type,text);
     }
    }
   }
   db.close();
  }
 }
 QSqlDatabase::removeDatabase("isolated-import");return ok;
}
}
bool importAdapterSupported(const QString &adapter){return QStringList{"utf8-document","json","xml","csv","sqlite","android-log"}.contains(adapter);}
QVariantMap parseAuthorizedImport(const QByteArray &bytes,const QString &adapter,const QString &workingPath,QString &error){
 error.clear();if(!importAdapterSupported(adapter)||bytes.isEmpty()||bytes.size()>importInputLimit){error="unsupported-adapter-or-input-limit";return {};}
 Findings findings;QString text;bool ok=false;
 if(adapter=="sqlite")ok=sqliteImport(bytes,workingPath,findings);
 else if(!textInput(bytes,text)){error="invalid-utf8-or-binary-document";return {};}
 else if(adapter=="json")ok=jsonImport(text.toUtf8(),findings);
 else if(adapter=="xml")ok=xmlImport(text.toUtf8(),findings);
 else if(adapter=="csv")ok=csvImport(text,findings);
 else{
  const auto lines=text.split('\n');ok=lines.size()<=20000;
  static const QRegularExpression logLine("^\\d{2}-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d{3,9}\\s+\\d+\\s+\\d+\\s+[VDIWEFS]\\s+[^:]+:.*$");
  int line=0;for(auto value:lines){++line;if(value.endsWith('\r'))value.chop(1);
   if(adapter=="android-log"&&!value.isEmpty()&&!value.startsWith("--------- beginning of ")&&!logLine.match(value).hasMatch()){ok=false;break;}
   findings.add("line/"+QString::number(line),adapter=="android-log"?"logcat-threadtime-raw":"text-line",value);
  }
 }
 if(!ok){error="malformed-or-unsupported-source";return {};}
 QVariantMap report{{"schema","nexvary-authorized-import/v1"},{"adapter",adapter},{"parserVersion",1},{"status","imported"},{"sourceSha256",hash(bytes)},{"sourceSize",bytes.size()},
  {"findings",findings.rows},{"observedCount",findings.count},{"findingsTruncated",findings.count>importFindingLimit},
  {"limits",QVariantMap{{"inputBytes",importInputLimit},{"findings",importFindingLimit},{"valueCharacters",256},{"depth",32}}},
  {"scope","Observed file content only; source authenticity, timestamps, timezone and legal authority are not established. SQLite is a bounded standalone row sample; no encrypted backups, archive extraction or device acquisition."}};
 if(QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact).size()>importReportLimit){error="report-limit";return {};}
 return report;
}
}
