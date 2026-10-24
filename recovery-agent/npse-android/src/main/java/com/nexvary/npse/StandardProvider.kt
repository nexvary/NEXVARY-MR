package com.nexvary.npse

import android.app.ActivityManager
import android.content.Context
import android.content.Intent
import android.hardware.usb.UsbManager
import android.net.Uri
import android.os.BatteryManager
import android.os.Build
import android.provider.Settings
import org.json.JSONObject

/** No serial, fingerprint, IP/MAC address, account, installed package list or private app data in reports. */
class StandardProvider(private val context: Context) {
    fun diagnostics(): JSONObject {
        val memory = ActivityManager.MemoryInfo()
        context.getSystemService(ActivityManager::class.java).getMemoryInfo(memory)
        val battery = context.getSystemService(BatteryManager::class.java)
        val usb = context.getSystemService(UsbManager::class.java)
        return JSONObject().put("schema", "nexvary-npse/v1").put("provider", "standard")
            .put("sdkInt", Build.VERSION.SDK_INT).put("manufacturer", Build.MANUFACTURER.take(80))
            .put("model", Build.MODEL.take(80)).put("securityPatch", Build.VERSION.SECURITY_PATCH)
            .put("memory", JSONObject().put("totalBytes", memory.totalMem).put("availableBytes", memory.availMem).put("low", memory.lowMemory))
            .put("batteryPercent", battery.getIntProperty(BatteryManager.BATTERY_PROPERTY_CAPACITY).takeIf { it in 0..100 } ?: JSONObject.NULL)
            .put("usb", JSONObject().put("visibleDevices", usb.deviceList.size).put("grantedDevices", usb.deviceList.values.count { usb.hasPermission(it) }))
            .put("adbTransport", "not-configured")
            .put("oemValidation", if (Build.MANUFACTURER.equals("oppo", true)) "coloros-field-test-required" else "field-test-required")
    }
    fun visiblePackages(): List<String> {
        val launcher = Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_LAUNCHER)
        @Suppress("DEPRECATION")
        return context.packageManager.queryIntentActivities(launcher, 0).map { it.activityInfo.packageName }
            .plus(context.packageName).distinct().sorted().take(200)
    }
    fun ownAppInfo(): JSONObject {
        @Suppress("DEPRECATION") val info = context.packageManager.getPackageInfo(context.packageName, 0)
        return JSONObject().put("package", context.packageName).put("version", info.versionName)
            .put("versionCode", if (Build.VERSION.SDK_INT >= 28) info.longVersionCode else @Suppress("DEPRECATION") info.versionCode.toLong()).put("targetSdk", info.applicationInfo?.targetSdkVersion)
    }
    fun settings(packageName: String): Intent {
        require(packageName in visiblePackages())
        return Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS, Uri.parse("package:$packageName"))
    }
}
