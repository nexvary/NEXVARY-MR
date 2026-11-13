#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
class FixtureServer:public QObject {
public:
 QTcpServer server;QString mode;int requests=0,calls=0,lists=0,paidCalls=0;bool failPaid=false;QList<QByteArray> headers;QJsonObject lastArguments;
 explicit FixtureServer(const QString &m={}):mode(m){
  if(!server.listen(QHostAddress::LocalHost))qFatal("Cannot bind the synthetic loopback MCP fixture");
  connect(&server,&QTcpServer::newConnection,this,[this]{while(server.hasPendingConnections()){
   auto *socket=server.nextPendingConnection();connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
   connect(socket,&QTcpSocket::readyRead,this,[this,socket]{
    auto data=socket->property("request").toByteArray()+socket->readAll();socket->setProperty("request",data);
    const auto split=data.indexOf("\r\n\r\n");if(split<0||socket->property("handled").toBool())return;
    const auto head=data.left(split);qsizetype length=-1;for(auto line:head.split('\n'))if(line.toLower().startsWith("content-length:"))length=line.mid(15).trimmed().toLongLong();
    if(length<0||data.size()<split+4+length)return;
    socket->setProperty("handled",true);headers.append(head);++requests;
    const auto msg=QJsonDocument::fromJson(data.mid(split+4,length)).object();const auto method=msg.value("method").toString();
    if(mode=="timeout")return;
    QByteArray payload,extra;int status=200;QJsonObject result;
    if(method=="initialize"){
     if(mode=="auth"||mode=="payment"||mode=="forbidden"||mode=="service"){status=mode=="auth"?401:mode=="payment"?402:mode=="forbidden"?403:503;payload="secret-reflection-must-not-display";}
     else if(mode=="redirect"){status=302;extra="Location: https://example.invalid/never-follow\r\n";}
     else {result={{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{{"tools",QJsonObject{}}}},{"serverInfo",QJsonObject{{"name","Synthetic fixture"},{"version","1"}}}};if(mode!="metadata")extra="Mcp-Session-Id: synthetic-session\r\n";}
    }else if(method=="notifications/initialized"){status=202;}
    else if(method=="tools/list"){
     ++lists;
     const QJsonObject schema{{"type","object"},{"required",QJsonArray{"selector"}},{"properties",QJsonObject{{"selector",QJsonObject{{"type","string"}}}}}};
     const QJsonObject tool{{"name",msg.value("params").toObject().value("cursor")=="second-page"?"second":"lookup"},{"description","Synthetic local test; not Lampyre data"},{"inputSchema",schema}};
     result={{"tools",QJsonArray{tool}}};
     if(mode=="metadata"){
      QJsonArray tools;for(const auto *name:{"lampyre_balance","lampyre_categories_list","lampyre_endpoints_from_categories"})tools.append(QJsonObject{{"name",name},{"description","Synthetic free metadata. Free."},{"inputSchema",QJsonObject{{"type","object"}}}});
      tools.append(QJsonObject{{"name","lampyre_run_crawlers"},{"description","Synthetic paid operation"},{"inputSchema",QJsonObject{{"type","object"},{"properties",QJsonObject{{"tasks",QJsonObject{{"type","array"}}}}},{"required",QJsonArray{"tasks"}}}}});result={{"tools",tools}};
     }
if(mode=="pages"&&msg.value("params").toObject().value("cursor")!="second-page")result.insert("nextCursor","second-page");
    }else if(method=="tools/call"){
     ++calls;lastArguments=msg.value("params").toObject().value("arguments").toObject();
     result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text","Synthetic server reply; TEST-API-KEY; Lampyre source label"}}}}};
     if(mode=="metadata"){
      const auto name=msg.value("params").toObject().value("name").toString();QJsonObject metadata;
      if(name=="lampyre_balance")metadata={{"photons",37.5}};
      else if(name=="lampyre_categories_list")metadata={{"categories",QJsonArray{QJsonObject{{"name","email"},{"description","Synthetic email category"}},QJsonObject{{"name","phone"}}}}};
      else if(name=="lampyre_endpoints_from_categories")metadata={{"crawlers",QJsonObject{{"email",QJsonArray{QJsonObject{{"tool_name","synthetic_email_lookup"},{"title","Synthetic email lookup"},{"photons",2.0},{"args","email: string (Email address) (all required)"}}}}}}};
      else if(name=="lampyre_run_crawlers"){++paidCalls;metadata={{"fixture",true}};}
      result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text",QString::fromUtf8(QJsonDocument(metadata).toJson(QJsonDocument::Compact))}}}}};
      if(name=="lampyre_run_crawlers"&&failPaid)result.insert("isError",true);
     }

    }
    if(payload.isEmpty()&&status==200)payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",result}}).toJson(QJsonDocument::Compact);
    if(mode=="unsupported"&&method=="initialize")payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",QJsonObject{{"protocolVersion","invalid-version"}}}}).toJson();
    if(mode=="malformed"&&method=="initialize")payload="not JSON";
    if(mode=="oversize"&&method=="initialize")payload=QByteArray(2*1024*1024+1,'x');
    const bool sse=(mode=="sse"||mode=="metadata")&&status==200;
    if(sse)payload="event: message\r\ndata: "+payload+"\r\n\r\n";
    socket->write("HTTP/1.1 "+QByteArray::number(status)+" Fixture\r\nConnection: close\r\nContent-Type: "+(sse?QByteArray("text/event-stream"):QByteArray("application/json"))+"\r\n"+extra+"Content-Length: "+QByteArray::number(payload.size())+"\r\n\r\n"+payload);socket->disconnectFromHost();
   });
  }});
 }
 QString url()const{return "http://127.0.0.1:"+QString::number(server.serverPort())+"/mcp";}
};
