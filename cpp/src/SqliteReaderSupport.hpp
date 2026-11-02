#pragma once
#include "SqliteTableReader.hpp"
#include "SqliteEvidenceFamily.hpp"

namespace mr::forensic {
struct SqliteColumnPolicy {
 QSet<QString> integers,texts,blobs;
 QStringList required;
 bool downloadChainKey=false;
 bool varcharTypes=false;
};
// Bare, ASCII, bounded CREATE grammar only. No SQL execution or path access.
QStringList sqliteDeclaredColumns(const QString &sql,const QString &name,const SqliteColumnPolicy &policy);
bool sqliteTextField(const SqliteTableRow &row,int index);
bool sqliteIntegerField(const SqliteTableRow &row,int index,bool nullable=false);
QVariantList sqliteSourceExtents(const SqliteTableRow &row,const SqliteFamilySnapshot &family);
QVariantMap sqliteChromiumTime(const QVariant &value);
QVariantMap sqliteRecordReference(const QString &rowid,const QVariantList &sourceExtents);
// Call only after the family summary has passed its inventory validator.
bool validateSqliteRecordReference(const QVariantMap &record,const QVariantMap &family,QMap<qint64,QVariantMap> &origins);
}
