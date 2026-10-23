#include <QCoreApplication>
#include <QFile>
#include <QThread>
#include <cstdio>

// A deterministic child process for native UI acceptance. This executable never
// invokes ADB, enumerates USB, opens a network connection or addresses hardware.
int main(int argc,char **argv) {
 QCoreApplication app(argc,argv);
 QFile modeFile(qEnvironmentVariable("MR_UI_DEVICE_MODE"));
 if(!modeFile.open(QIODevice::ReadOnly)) {
  std::fputs("Synthetic UI fixture mode is unavailable\n",stderr);return 90;
 }
 const auto mode=modeFile.read(32).trimmed();
 const auto args=app.arguments().mid(1);
 if(args==QStringList{"devices","-l"}) {
  if(mode=="error") {
   std::fputs("Synthetic discovery failure for UI acceptance\n",stderr);return 1;
  }
  if(mode=="progress")QThread::msleep(2000);
  std::fputs("List of devices attached\n",stdout);
  if(mode=="disconnected"||mode=="none")return 0;
  if(mode=="connected"||mode=="device"||mode=="progress") {
   std::fputs("SYNTHETIC42\tdevice product:ui_fixture model:Synthetic_Phone device:fixture transport_id:1\n",stdout);return 0;
  }
  if(mode=="unauthorized") {
   std::fputs("SYNTHETIC42\tunauthorized\n",stdout);return 0;
  }
  if(mode=="offline") {
   std::fputs("SYNTHETIC42\toffline\n",stdout);return 0;
  }
  std::fputs("Unknown synthetic UI fixture mode\n",stderr);return 91;
 }
 if((mode=="connected"||mode=="device"||mode=="progress")&&
    args==QStringList{"-s","SYNTHETIC42","shell","getprop"}) {
  std::fputs(
   "[ro.product.manufacturer]: [Synthetic Lab]\n"
   "[ro.product.brand]: [Synthetic]\n"
   "[ro.product.model]: [Synthetic Phone]\n"
   "[ro.product.device]: [ui_fixture]\n"
   "[ro.build.version.release]: [15]\n"
   "[ro.build.version.sdk]: [35]\n"
   "[ro.build.version.codename]: [REL]\n"
   "[ro.build.version.preview_sdk]: [0]\n"
   "[ro.build.version.security_patch]: [2026-09-01]\n"
   "[ro.build.fingerprint]: [synthetic/ui_fixture/fixture:15/TEST/1:user/release-keys]\n"
   "[ro.product.first_api_level]: [33]\n"
   "[ro.hardware]: [synthetic]\n"
   "[ro.soc.manufacturer]: [Synthetic]\n"
   "[ro.soc.model]: [UI fixture]\n"
   "[ro.boot.verifiedbootstate]: [green]\n"
   "[ro.boot.vbmeta.device_state]: [locked]\n"
   "[ro.boot.flash.locked]: [1]\n"
   "[ro.boot.dynamic_partitions]: [true]\n"
   "[ro.virtual_ab.enabled]: [true]\n"
   "[ro.build.ab_update]: [true]\n"
   "[ro.boot.slot_suffix]: [_a]\n"
   "[ro.crypto.state]: [encrypted]\n"
   "[ro.crypto.type]: [file]\n"
   "[sys.boot_completed]: [1]\n",stdout);
  return 0;
 }
 if((mode=="connected"||mode=="device")&&
    args==QStringList{"-s","SYNTHETIC42","logcat","-d","-t","200","-v","threadtime"}) {
  std::fputs("10-23 12:00:00.000 100 100 I Synthetic: UI acceptance fixture only\n",stdout);return 0;
 }
 std::fputs("Synthetic UI fixture rejected an unexpected command\n",stderr);return 99;
}
