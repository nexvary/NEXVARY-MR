#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageAuthenticationCode>
#include <QTimer>
#include <QFileInfo>
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);auto args=app.arguments();const auto at=args.indexOf("--port");if(at<0||at+1>=args.size())return 2;
 QTcpServer server;if(!server.listen(QHostAddress::LocalHost,args[at+1].toUShort()))return 3;
 QObject::connect(&server,&QTcpServer::newConnection,&app,[&]{while(server.hasPendingConnections()){
  auto *socket=server.nextPendingConnection();socket->setProperty("bytes",QByteArray{});
  QObject::connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
  QObject::connect(socket,&QTcpSocket::readyRead,socket,[&,socket]{
   auto bytes=socket->property("bytes").toByteArray()+socket->readAll();socket->setProperty("bytes",bytes);
   const auto end=bytes.indexOf("\r\n\r\n");if(end<0)return;
   QByteArray challenge,auth;int length=0;for(const auto &line:bytes.left(end).split('\n')){
    const auto row=line.trimmed();if(row.toLower().startsWith("content-length:"))length=row.mid(15).trimmed().toInt();
    if(row.toLower().startsWith("x-nexvary-challenge:"))challenge=row.mid(20).trimmed();
    if(row.toLower().startsWith("authorization:"))auth=row.mid(14).trimmed();
   }
   if(bytes.size()<end+4+length)return;
   const auto token=qgetenv("NEXVARY_PHONEINFOGA_TOKEN");QByteArray body="{\"result\":{\"country\":\"Synthetic fixture\"}}",status="200 OK",headers;
   if(bytes.startsWith("GET /api/ ")){
    auto proof=QMessageAuthenticationCode::hash(challenge,token,QCryptographicHash::Sha256).toHex();
    #ifdef MR_OSINT_WRONG_PROOF
    proof="invalid";
#endif
    headers="X-Nexvary-Proof: "+proof+"\r\n";body="{}";
   }else if(auth!="Bearer "+token){status="403 Forbidden";body="{}";}
   else if(bytes.contains("/ovh/")){return;} // Intentional hanging scanner.
   else if(bytes.contains("/googlesearch/")){status="404 Not Found";body="{\"error\":\"unavailable\"}";}
   else if(bytes.contains("/googlecse/")){body="not JSON";}
   else if(bytes.contains("/numverify/")){body=QByteArray(270000,'x');}
   socket->write("HTTP/1.1 "+status+"\r\nContent-Type: application/json\r\n"+headers+"Content-Length: "+QByteArray::number(body.size())+"\r\nConnection: close\r\n\r\n"+body);socket->disconnectFromHost();
  });
 }});return app.exec();
}
