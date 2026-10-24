package com.nexvary.npse

import android.content.pm.PackageManager
import com.nexvary.npse.core.AccessPolicy
import rikka.shizuku.Shizuku

/** API availability is a build dependency; installed manager is never treated as a capability. */
object ShizukuAccess {
    fun state(denied: Boolean = false): AccessPolicy.State = try {
        val alive = Shizuku.pingBinder()
        AccessPolicy.evaluate(alive, alive && Shizuku.checkSelfPermission() == PackageManager.PERMISSION_GRANTED,
            if (alive) Shizuku.getUid() else -1, denied)
    } catch (_: RuntimeException) { AccessPolicy.State.UNAVAILABLE }
    fun request(code: Int): Boolean = try {
        if (state() != AccessPolicy.State.PERMISSION_REQUIRED || Shizuku.shouldShowRequestPermissionRationale()) false
        else { Shizuku.requestPermission(code); true }
    } catch (_: RuntimeException) { false }
}
