#include "ImportAdapters.hpp"
#include "RecoveryFile.hpp"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <cstdio>
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);QCommandLineParser cli;
 cli.addOption(QCommandLineOption(QStringLiteral("input"),"Private working copy","path"));
 cli.addOption(QCommandLineOption(QStringLiteral("sha256"),"Expected source SHA-256","hash"));
 cli.addOption(QCommandLineOption(QStringLiteral("adapter"),"Reviewed native adapter","id"));
 if(!cli.parse(app.arguments())||cli.optionNames().size()!=3||!cli.positionalArguments().isEmpty())return 2;
 const auto path=mr::recovery::localFilePath(cli.value("input"));QFile file(path);
 if(path.isEmpty()||QFileInfo(path).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()<=0||file.size()>mr::forensic::importInputLimit)return 2;
 const auto bytes=file.read(mr::forensic::importInputLimit+1);file.close();
 if(bytes.size()>mr::forensic::importInputLimit||QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())!=cli.value("sha256"))return 2;
 QString error;const auto report=mr::forensic::parseAuthorizedImport(bytes,cli.value("adapter"),path,error);
 if(report.isEmpty()||!error.isEmpty())return 3;
 const auto output=QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact);
 return std::fwrite(output.constData(),1,static_cast<size_t>(output.size()),stdout)==static_cast<size_t>(output.size())?0:4;
}
