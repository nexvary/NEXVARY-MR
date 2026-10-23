#pragma once
#include <QStringList>
#include "ServiceOperationPolicy.hpp"
#include <QList>

// Protocol adapters produce bounded command plans; ToolBridge alone owns the
// process, deadlines, cancellation, target pinning and authorization gate.
namespace mr::service {
struct Query {QStringList arguments;QString key;bool optional=false;};
inline QString settingsIntent(const QString &action){
 if(action=="languages")return "android.settings.LOCALE_SETTINGS";
 if(action=="keyboard")return "android.settings.INPUT_METHOD_SETTINGS";
 if(action=="settings")return "android.settings.SETTINGS";
 if(action=="apn")return "android.settings.APN_SETTINGS";
 return {};
}
class ProtocolAdapter {
public:
 virtual ~ProtocolAdapter()=default;
 virtual QString id()const=0;
 virtual QStringList discovery()const=0;
 virtual QList<Query> queries(const QString &action)const=0;
 virtual QStringList command(const QString &action,const QString &package,bool replace)const=0;
 QStringList addressed(const QString &serial,const QStringList &arguments)const{return QStringList{"-s",serial}+arguments;}
};
class AdbAdapter final:public ProtocolAdapter {
public:
 QString id()const override{return "adb";}
 QStringList discovery()const override{return {"devices","-l"};}
 QList<Query> queries(const QString &action)const override{
  if(action=="sideload_zip")return {};
  QList<Query> result{{{"shell","getprop"},"identity"}};
  if(action!="probe")return result;
  result.append({{"shell","cmd","uimode","help"},"night",true});
  for(const auto &a:QStringList{"languages","keyboard","settings","apn"})result.append({{"shell","cmd","package","resolve-activity","--brief","-a",settingsIntent(a)},"intent:"+a,true});
  result.append({{"shell","getconf","PAGE_SIZE"},"observation:page-size",true});
  result.append({{"shell","dumpsys","battery"},"observation:battery",true});
  result.append({{"shell","df","-k","/data"},"observation:storage",true});
  result.append({{"shell","cat","/proc/partitions"},"observation:partitions",true});return result;
 }
 QStringList command(const QString &action,const QString &package,bool replace)const override{
  if(!operationAllowed(action.toStdString()))return {};
  if(QStringList{"reboot_normal","reboot_recovery","reboot_bootloader","reboot_sideload"}.contains(action)){QStringList args{"reboot"};if(action!="reboot_normal")args.append(action.mid(7));return args;}
  if(action=="dark_on"||action=="dark_off")return {"shell","cmd","uimode","night",action=="dark_on"?"yes":"no"};
  const auto intent=settingsIntent(action);if(!intent.isEmpty())return {"shell","am","start","-W","-a",intent};
  if(action=="install_apk"&&!package.isEmpty()){QStringList args{"install"};if(replace)args.append("-r");args.append(package);return args;}
  if(action=="sideload_zip"&&!package.isEmpty())return {"sideload",package};
  return {};
 }
};
class FastbootAdapter final:public ProtocolAdapter {
public:
 QString id()const override{return "fastboot";}
 QStringList discovery()const override{return {"devices"};}
 QList<Query> queries(const QString &action)const override{
  QStringList vars{"product","unlocked","is-userspace"};
  if(action=="probe")vars.append({"version-bootloader","partition-size:cache","partition-size:userdata","partition-type:userdata"});
  else if(action=="erase_cache"||action=="erase_userdata")vars.append(action=="erase_cache"?"partition-size:cache":"partition-size:userdata");
  QList<Query> result;for(const auto &var:vars)result.append({{"getvar",var},"var:"+var,var!="product"});return result;
 }
 QStringList command(const QString &action,const QString &,bool)const override{
  if(!operationAllowed(action.toStdString()))return {};
  if(action=="fastboot_reboot")return {"reboot"};
  if(action=="fastboot_bootloader")return {"reboot-bootloader"};
  if(action=="unlock_ability")return {"flashing","get_unlock_ability"};
  if(action=="erase_cache")return {"erase","cache"};
  if(action=="erase_userdata")return {"erase","userdata"};
  return {};
 }
};
inline const ProtocolAdapter *adapter(const QString &id){static const AdbAdapter adb;static const FastbootAdapter fastboot;return id=="adb"?static_cast<const ProtocolAdapter *>(&adb):id=="fastboot"?static_cast<const ProtocolAdapter *>(&fastboot):nullptr;}
}

