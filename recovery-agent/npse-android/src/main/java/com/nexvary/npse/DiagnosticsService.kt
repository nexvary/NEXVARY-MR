package com.nexvary.npse

import android.content.Context
import android.os.Binder
import android.os.Process
import com.nexvary.npse.core.AccessPolicy
import com.nexvary.npse.core.BoundedRunner
import com.nexvary.npse.core.Probe
import org.json.JSONArray
import org.json.JSONObject
import java.util.concurrent.atomic.AtomicReference
import kotlin.system.exitProcess

/** Shizuku v13+ context constructor. A caller cannot nominate its own authorized UID. */
class DiagnosticsService(context: Context) : IDiagnosticsService.Stub() {
    private val ownerUid = context.applicationInfo.uid
    private val active = AtomicReference<BoundedRunner.Cancellation?>()
    private fun checkCaller() {
        check(Process.myUid() == 2000 && AccessPolicy.callerAllowed(Binder.getCallingUid(), ownerUid))
    }
    override fun collectDiagnostics(): String {
        checkCaller()
        val cancellation = BoundedRunner.Cancellation()
        check(active.compareAndSet(null, cancellation)) { "Busy" }
        return try {
            val rows = JSONArray()
            val runner = BoundedRunner()
            for (probe in arrayOf(Probe.IDENTITY, Probe.VERIFIED_BOOT, Probe.BOOTLOADER_LOCK, Probe.MEMORY_TOTALS)) {
                if (cancellation.isCanceled) break
                val result = runner.run(probe, cancellation)
                rows.put(JSONObject().put("operation", probe.id).put("status", result.status.name)
                    .put("value", if (probe == Probe.MEMORY_TOTALS && result.status == BoundedRunner.Status.OK)
                        result.value.lineSequence().filter { it.startsWith("MemTotal:") || it.startsWith("MemAvailable:") }.joinToString("\n")
                        else result.value))
            }
            JSONObject().put("provider", "shizuku-shell").put("uid", 2000).put("observations", rows).toString()
        } finally { active.compareAndSet(cancellation, null) }
    }
    override fun cancel() { checkCaller(); active.get()?.cancel() }
    override fun destroy() {
        check(Binder.getCallingUid() == 2000 || AccessPolicy.callerAllowed(Binder.getCallingUid(), ownerUid))
        active.get()?.cancel(); exitProcess(0)
    }
}
