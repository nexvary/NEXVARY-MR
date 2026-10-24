#!/usr/bin/env bash
set -euo pipefail
# This script may install and stop a manager only on one disposable Android emulator.
variant=${1:?official or next required}
mkdir -p manager-evidence
mapfile -t devices < <(adb devices | awk 'NR>1 && $2 == "device" {print $1}')
[[ ${#devices[@]} == 1 && ${devices[0]} == emulator-* ]] || { echo 'One disposable emulator required'; exit 1; }
export ANDROID_SERIAL=${devices[0]}
[[ $(adb shell getprop ro.kernel.qemu | tr -d '\r') == 1 ]] || exit 1
[[ $(adb shell id -u | tr -d '\r') == 2000 ]] || { echo 'Shell UID 2000 required; root refused'; exit 1; }
case "$variant" in
 official)
  url='https://github.com/RikkaApps/Shizuku/releases/download/v13.6.0/shizuku-v13.6.0.r1086.2650830c-release.apk'
  digest='6e273ab0e991c4e79bc8b1bbb9b9dd739ccac1a8712a541a214078886b7b790f'
  ;;
 next)
  url='https://github.com/rushiranpise/Shizuku-Next/releases/download/v14.0.29-next/shizuku-v14.0.29-next.apk'
  digest='e46623397e76e7f0292ba5fd435347cda123ead6d7e32869333c64d2dac30079'
  ;;
 *) exit 1 ;;
esac
curl --fail --location --retry 2 "$url" -o manager-evidence/manager.apk
printf '%s  manager-evidence/manager.apk\n' "$digest" | sha256sum --check --strict
"$ANDROID_HOME/build-tools/35.0.0/apksigner" verify --verbose --print-certs manager-evidence/manager.apk > manager-evidence/manager-signature.txt
adb install manager-evidence/manager.apk
adb shell monkey -p moe.shizuku.privileged.api -c android.intent.category.LAUNCHER 1 > manager-evidence/manager-launch.txt
apk=$(adb shell pm path moe.shizuku.privileged.api | tr -d '\r' | sed -n 's/^package://p' | head -1)
[[ "$apk" == /data/app/*/base.apk && "$apk" != *[!a-zA-Z0-9_./=~+-]* ]] || { echo 'Unexpected manager APK path'; exit 1; }
# Both reviewed managers expose this same native starter path as their user ADB command.
starter="${apk%/base.apk}/lib/x86_64/libshizuku.so"
adb shell "$starter" > manager-evidence/starter.txt
for attempt in $(seq 1 30); do
 if adb shell pidof shizuku_server | grep -Eq '^[0-9 ]+$'; then break; fi
 [[ "$attempt" != 30 ]] || { echo 'Manager server did not start'; exit 1; }
 sleep 1
done
adb install -t app/build/outputs/apk/debug/app-debug.apk
adb install -t app/build/outputs/apk/androidTest/debug/app-debug-androidTest.apk
adb shell am instrument -w -e class com.nexvary.mr.agent.ShizukuShellIntegrationTest -e npseManagerIntegration true com.nexvary.mr.agent.test/androidx.test.runner.AndroidJUnitRunner | tee manager-evidence/instrumentation.txt
grep -F 'OK (1 test)' manager-evidence/instrumentation.txt
if grep -E 'FAILURES!!!|INSTRUMENTATION_FAILED|Process crashed|INSTRUMENTATION_ABORTED' manager-evidence/instrumentation.txt; then exit 1; fi
adb pull /data/local/tmp/npse-shizuku-shell.png manager-evidence/npse-shizuku-shell.png
