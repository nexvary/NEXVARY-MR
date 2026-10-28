#include "SqliteReaderSupport.hpp"
#include <QRegularExpression>

namespace mr::forensic {
bool sqliteTextField(const SqliteTableRow &r,int i){return i>=0&&i<r.serialTypes.size()&&r.serialTypes[i]>=13&&(r.serialTypes[i]&1);}
bool sqliteIntegerField(const SqliteTableRow &r,int i,bool nullable){return i>=0&&i<r.serialTypes.size()&&((r.serialTypes[i]>=1&&r.serialTypes[i]<=6)||r.serialTypes[i]==8||r.serialTypes[i]==9||(nullable&&r.serialTypes[i]==0));}
QStringList sqliteDeclaredColumns(const QString &sql,const QString &name,const SqliteColumnPolicy &policy){
 if(sql.size()>8192)return {};
 for(const auto c:sql){const auto u=c.unicode();if(u>126||(u<32&&u!=9&&u!=10&&u!=12&&u!=13))return {};}
 static const QRegularExpression create("\\ACREATE\\s+TABLE\\s+([a-z_][a-z_0-9]*)\\s*\\((.*)\\)\\s*\\z",QRegularExpression::CaseInsensitiveOption|QRegularExpression::DotMatchesEverythingOption);
 static const QRegularExpression column("\\A([a-z_][a-z_0-9]*)\\s+(INTEGER|BOOLEAN|TEXT|LONGVARCHAR|VARCHAR(?:\\(255\\))?|BLOB)((?:\\s+(?:PRIMARY KEY(?: AUTOINCREMENT)?|NOT NULL|DEFAULT (?:-?[0-9]+|FALSE|TRUE)))*)\\z",QRegularExpression::CaseInsensitiveOption);
 const auto m=create.match(sql.trimmed());if(!m.hasMatch()||m.captured(1).toLower()!=name)return {};
 auto body=m.captured(2);
 if(policy.downloadChainKey){
  // The only supported table constraint is Chromium's exact rowid chain key.
  static const QRegularExpression key(",\\s*PRIMARY\\s+KEY\\s*\\(\\s*id\\s*,\\s*chain_index\\s*\\)\\s*\\z",QRegularExpression::CaseInsensitiveOption);
  const auto k=key.match(body);if(!k.hasMatch())return {};body.truncate(k.capturedStart());
 }
 const auto definitions=body.split(',');if(definitions.size()>32)return {};
 QStringList out;
 for(const auto &def:definitions){
  const auto c=column.match(def.simplified());if(!c.hasMatch())return {};
  const auto field=c.captured(1).toLower(),type=c.captured(2).toUpper(),tail=c.captured(3).toUpper();
  if(out.contains(field))return {};
  if(field=="id"&&!policy.downloadChainKey){
   if(type!="INTEGER"||(tail!=" PRIMARY KEY"&&tail!=" PRIMARY KEY AUTOINCREMENT"))return {};
  }else{
   if(tail.contains("PRIMARY"))return {};
   if(policy.integers.contains(field)){if(type!="INTEGER"&&type!="BOOLEAN")return {};}
   else if(policy.texts.contains(field)){if(type!="TEXT"&&type!="LONGVARCHAR"&&!(policy.varcharTypes&&(type=="VARCHAR"||type=="VARCHAR(255)")))return {};}
   else if(policy.blobs.contains(field)){if(type!="BLOB")return {};}
   else return {};
  }
  out<<field;
 }
 for(const auto &field:policy.required)if(!out.contains(field))return {};
 return out;
}
QVariantList sqliteSourceExtents(const SqliteTableRow &row,const SqliteFamilySnapshot &family){
 QVariantList result;const auto ps=family.analysis.report.value("sqliteFamily").toMap().value("pageSize").toLongLong();
 if(ps<512||ps>65536||(ps&(ps-1)))return {};
 auto extents=row.imageExtents;extents.prepend(row.imageCellExtent);
 for(const auto &value:extents){
  const auto e=value.toMap();const auto off=e.value("imageByteOffset").toLongLong(),len=e.value("lengthBytes").toLongLong();
  const auto page=quint32(off/ps+1);auto origin=family.pageOrigins.value(page);
  if(origin.isEmpty()||len<0||off%ps+len>ps)return {};
  origin.insert("byteOffset",origin.value("byteOffset").toLongLong()+off%ps);origin.insert("lengthBytes",len);origin.insert("imageByteOffset",off);result.append(origin);
 }
 return result;
}
QVariantMap sqliteChromiumTime(const QVariant &value){
 if(value.isNull())return {{"rawMicroseconds1601",QVariant{}},{"status","null"}};
 const auto raw=value.toString();bool ok=false;const auto n=raw.toLongLong(&ok);
 QVariantMap result{{"rawMicroseconds1601",raw},{"status",!ok||n<0?"unsupported-negative":n==0?"unset-zero":"converted"}};
 // Subtract only nonnegative signed values: no overflow or floating point.
 if(ok&&n>0)result.insert("unixMicroseconds",QString::number(n-Q_INT64_C(11644473600000000)));
 return result;
}

QVariantMap sqliteRecordReference(const QString &rowid,const QVariantList &refs){
 QVariantList image,raw;
 for(const auto &v:refs){auto e=v.toMap();image.append(QVariantMap{{"imageByteOffset",e.take("imageByteOffset")},{"lengthBytes",e.value("lengthBytes")}});raw.append(e);}
 return {{"rowid",rowid},{"imageExtents",image},{"rawExtents",raw}};
}
bool validateSqliteRecordReference(const QVariantMap &rec,const QVariantMap &summary,QMap<qint64,QVariantMap> &origins){
 const auto ps=summary.value("pageSize").toLongLong(),imageSize=summary.value("snapshotBytes").toLongLong();
 if(ps<512||ps>65536||(ps&(ps-1))||imageSize<ps||imageSize>8*1024*1024)return false;
 QMap<QString,QVariantMap> members;for(const auto &v:summary.value("members").toList())members.insert(v.toMap().value("role").toString(),v.toMap());
 const auto canonicalInteger=[](const QVariant &v){bool ok=false;const auto n=v.toString().toLongLong(&ok);return v.metaType().id()==QMetaType::QString&&ok&&QString::number(n)==v.toString();};

  if(rec.size()!=3||!canonicalInteger(rec.value("rowid")))return false;
  const auto image=rec.value("imageExtents").toList(),raw=rec.value("rawExtents").toList();
  if(image.isEmpty()||image.size()!=raw.size()||image.size()>141)return false;
  qint64 total=0;
  for(qsizetype i=0;i<image.size();++i){
   const auto e=image[i].toMap(),o=raw[i].toMap();
   const auto exact=[](const QVariant &v){const auto type=v.metaType().id();if(type!=QMetaType::Int&&type!=QMetaType::UInt&&type!=QMetaType::LongLong&&type!=QMetaType::ULongLong&&type!=QMetaType::Double)return false;bool ok=false;const auto n=v.toLongLong(&ok);return ok&&v.toDouble()==double(n);};
   if(e.size()!=2||o.size()!=5||!exact(e.value("imageByteOffset"))||!exact(e.value("lengthBytes"))||!exact(o.value("byteOffset"))||!exact(o.value("lengthBytes")))return false;
   const auto offset=e.value("imageByteOffset").toLongLong(),length=e.value("lengthBytes").toLongLong(),rawOffset=o.value("byteOffset").toLongLong();
   if(offset<0||length<=0||offset>imageSize-length||offset/ps!=(offset+length-1)/ps||o.value("lengthBytes").toLongLong()!=length)return false;
   const auto role=o.value("role").toString();if(role!="main"&&role!="wal")return false;
   const auto m=members.value(role);if(m.isEmpty()||o.value("evidenceId")!=m.value("evidenceId")||o.value("sourceSha256")!=m.value("originalSha256")||rawOffset<0||rawOffset>m.value("bytes").toLongLong()-length)return false;
   const auto base=rawOffset-offset%ps;
   if(role=="main"&&base!=offset/ps*ps)return false;
   if(role=="wal"&&(base<56||(base-56)%(ps+24)||(base-56)/(ps+24)+1>summary.value("lastCommitFrame").toLongLong()))return false;
   QVariantMap origin{{"role",role},{"base",base}};const auto page=offset/ps;
   if(origins.contains(page)&&origins.value(page)!=origin)return false;
   origins.insert(page,origin);total+=length;if(total>64*1024)return false;
  }
  return true;

}
}
