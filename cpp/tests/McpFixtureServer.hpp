#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
class FixtureServer:public QObject {
public:
 QTcpServer server;QString mode;int requests=0,calls=0,lists=0,paidCalls=0,collections=0;bool failPaid=false;QList<QByteArray> headers;QJsonObject lastArguments,lastPaidArguments,lastLookupInput;int lookupSearches=0,lookupReads=0,lookupHistoryReads=0;QList<QByteArray> lookupHeaders;
 explicit FixtureServer(const QString &m={}):mode(m){
  if(!server.listen(QHostAddress::LocalHost))qFatal("Cannot bind the synthetic loopback MCP fixture");
  connect(&server,&QTcpServer::newConnection,this,[this]{while(server.hasPendingConnections()){
   auto *socket=server.nextPendingConnection();connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
   connect(socket,&QTcpSocket::readyRead,this,[this,socket]{
    auto data=socket->property("request").toByteArray()+socket->readAll();socket->setProperty("request",data);
    const auto split=data.indexOf("\r\n\r\n");if(split<0||socket->property("handled").toBool())return;
    const auto head=data.left(split);qsizetype length=head.startsWith("GET ")?0:-1;for(auto line:head.split('\n'))if(line.toLower().startsWith("content-length:"))length=line.mid(15).trimmed().toLongLong();
    if(length<0||data.size()<split+4+length)return;
    socket->setProperty("handled",true);headers.append(head);++requests;
    if(head.startsWith("GET ")){handleLookup(socket,head);return;}
    const auto msg=QJsonDocument::fromJson(data.mid(split+4,length)).object();const auto method=msg.value("method").toString();
    if(mode=="timeout")return;
    QByteArray payload,extra;int status=200;QJsonObject result;
    if(method=="initialize"){
     if(mode=="auth"||mode=="payment"||mode=="forbidden"||mode=="service"){status=mode=="auth"?401:mode=="payment"?402:mode=="forbidden"?403:503;payload="secret-reflection-must-not-display";}
     else if(mode=="redirect"){status=302;extra="Location: https://example.invalid/never-follow\r\n";}
     else {result={{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{{"tools",QJsonObject{}}}},{"serverInfo",QJsonObject{{"name","Synthetic fixture"},{"version","1"}}}};if(!mode.startsWith("metadata"))extra="Mcp-Session-Id: synthetic-session\r\n";}
    }else if(method=="notifications/initialized"){status=202;}
    else if(method=="tools/list"){
     ++lists;
     const QJsonObject schema{{"type","object"},{"required",QJsonArray{"selector"}},{"properties",QJsonObject{{"selector",QJsonObject{{"type","string"}}}}}};
     const QJsonObject tool{{"name",msg.value("params").toObject().value("cursor")=="second-page"?"second":"lookup"},{"description","Synthetic local test; not Lampyre data"},{"inputSchema",schema}};
     result={{"tools",QJsonArray{tool}}};
     if(mode.startsWith("metadata")){
      QJsonArray tools;for(const auto *name:{"lampyre_balance","lampyre_categories_list","lampyre_endpoints_from_categories"})tools.append(QJsonObject{{"name",name},{"description","Synthetic free metadata. Free."},{"inputSchema",QJsonObject{{"type","object"}}}});
      tools.append(QJsonObject{{"name","lampyre_run_crawlers"},{"description","Synthetic paid operation"},{"inputSchema",QJsonObject{{"type","object"},{"properties",QJsonObject{{"tasks",QJsonObject{{"type","array"}}}}},{"required",QJsonArray{"tasks"}}}}});tools.append(QJsonObject{{"name","lampyre_check_result_endpoint"},{"description",mode=="metadata-no-free"?"Unknown cost":"Charges nothing — already paid for."},{"inputSchema",QJsonObject{{"type","object"}}}});result={{"tools",tools}};
     }
if(mode=="pages"&&msg.value("params").toObject().value("cursor")!="second-page")result.insert("nextCursor","second-page");
    }else if(method=="tools/call"){
     ++calls;lastArguments=msg.value("params").toObject().value("arguments").toObject();
     result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text","Synthetic server reply; TEST-API-KEY; Lampyre source label"}}}}};
     if(mode.startsWith("metadata")){
      const auto name=msg.value("params").toObject().value("name").toString();QJsonObject metadata;
      if(name=="lampyre_balance")metadata={{"photons",37.5}};
      else if(name=="lampyre_categories_list")metadata={{"categories",QJsonArray{QJsonObject{{"name","email"},{"description","Synthetic email category"}},QJsonObject{{"name","phone"}}}}};
      else if(name=="lampyre_endpoints_from_categories"){
       const auto category=lastArguments.value("categories").toArray().first().toString();
       metadata={{"crawlers",QJsonObject{{category,QJsonArray{QJsonObject{{"tool_name","synthetic_"+category+"_lookup"},{"title","Synthetic "+category+" lookup"},{"photons",2.0},{"args",category+": string (all required)"}}}}}}};
      }
      else if(name=="lampyre_run_crawlers"||name=="lampyre_check_result_endpoint"){
       const bool collecting=name=="lampyre_check_result_endpoint";if(collecting)++collections;else {++paidCalls;lastPaidArguments=lastArguments;}
       const auto task=lastArguments.value("tasks").toArray().first().toObject();
       QJsonObject row{{"tool_name",task.value("tool_name")},{"task_id","synthetic-task"}};
       if((!collecting&&mode!="metadata")||(mode=="metadata-wait"&&collections==1))row.insert("status","running");
       else if(mode=="metadata-task-error")row.insert("error","The crawler failed and was refunded.");
       else row.insert("result",mode=="metadata-empty"?QJsonArray{}:QJsonArray{QJsonObject{{"fixture",true},{"url","https://example.invalid/synthetic-profile"}}});
       metadata={{"tasks",QJsonArray{row}}};
      }
      result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text",QString::fromUtf8(QJsonDocument(metadata).toJson(QJsonDocument::Compact))}}}}};
      if(name=="lampyre_run_crawlers"&&failPaid)result.insert("isError",true);
     }

    }
    if(payload.isEmpty()&&status==200)payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",result}}).toJson(QJsonDocument::Compact);
    if(mode=="unsupported"&&method=="initialize")payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",QJsonObject{{"protocolVersion","invalid-version"}}}}).toJson();
    if(mode=="malformed"&&method=="initialize")payload="not JSON";
    if(mode=="oversize"&&method=="initialize")payload=QByteArray(2*1024*1024+1,'x');
    const bool sse=(mode=="sse"||mode.startsWith("metadata"))&&status==200;
    if(sse)payload="event: message\r\ndata: "+payload+"\r\n\r\n";
    socket->write("HTTP/1.1 "+QByteArray::number(status)+" Fixture\r\nConnection: close\r\nContent-Type: "+(sse?QByteArray("text/event-stream"):QByteArray("application/json"))+"\r\n"+extra+"Content-Length: "+QByteArray::number(payload.size())+"\r\n\r\n"+payload);socket->disconnectFromHost();
   });
  }});
 }
 void handleLookup(QTcpSocket *socket,const QByteArray &head){
  lookupHeaders.append(head);const auto target=head.split(' ').value(1);const QUrl url=QUrl::fromEncoded(target);const QUrlQuery query(url);
  int status=200;QJsonObject body;QByteArray raw;
  if(mode=="lookup-auth")status=403;
  else if(url.path()=="/api/1.6/find/history/by-token"){++lookupHistoryReads;status=204;}
  else if(url.path()=="/api/1.6/find/by-token"){
   ++lookupSearches;lastLookupInput={};for(const auto &pair:query.queryItems(QUrl::FullyDecoded))if(pair.first!="token")lastLookupInput.insert(pair.first,pair.second);
   if(mode=="lookup-timeout")return;
   if(mode=="lookup-service")status=503;
   else if(mode=="lookup-malformed")raw="not JSON";
   else body={{"request_uuid","00112233-4455-6677-8899-aabbccddeeff"}};
  }else if(url.path()=="/api/1.6/find/00112233-4455-6677-8899-aabbccddeeff/by-token"){
   ++lookupReads;
   if((mode=="lookup-delayed"&&lookupReads==1)||mode=="lookup-pending")body={{"status",1}};
   else if(mode=="lookup-failed")body={{"status",3}};
   else if(mode=="lookup-empty")status=204;
   else {QJsonArray rows{QJsonObject{{"source","Synthetic Network"},{"data",QJsonObject{{"profile",QJsonArray{QJsonObject{{"username","synthetic-user"},{"url","https://example.invalid/unified-profile"}}}}}},{"payment_required",mode=="lookup-locked"}}};body={{"status",2},{"input",lastLookupInput},{"results_count",1},{"result",rows}};}
  }else status=404;
  if(raw.isEmpty()&&status!=204)raw=QJsonDocument(body).toJson(QJsonDocument::Compact);
  socket->write("HTTP/1.1 "+QByteArray::number(status)+" Fixture\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(raw.size())+"\r\n\r\n"+raw);socket->disconnectFromHost();
 }
 QString url()const{return "http://127.0.0.1:"+QString::number(server.serverPort())+"/mcp";}
};
