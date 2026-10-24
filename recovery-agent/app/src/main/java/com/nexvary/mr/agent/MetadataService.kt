package com.nexvary.mr.agent

import android.os.Process
import android.os.StatFs
import org.json.JSONArray
import org.json.JSONObject
import kotlin.system.exitProcess

/** Shizuku UserService: fixed metadata only, no client-selected commands or paths. */
class MetadataService(context: android.content.Context) : IMetadataService.Stub() {
    private val ownerUid = context.applicationInfo.uid

    override fun collectMetadata(): String {
        check(com.nexvary.npse.core.AccessPolicy.callerAllowed(android.os.Binder.getCallingUid(), ownerUid)) { "Unexpected caller" }
        check(Process.myUid() == 2000) { "Only shell UID 2000 is supported; root is refused" }
        val probes = JSONArray()
        for (name in listOf("shell-identity") + ProbePolicy.properties.keys) {
            val result = runFixedProbe(name)
            probes.put(JSONObject().put("name", result.name).put("status", result.status).put("value", result.value))
        }
        val capacity = try {
            // Aggregate filesystem capacity, never directory listings or private file contents.
            val fs = StatFs("/data")
            MetadataProbe("filesystem-capacity", "ok", "totalBytes=${fs.totalBytes}; availableBytes=${fs.availableBytes}; aggregate-only")
        } catch (_: SecurityException) {
            MetadataProbe("filesystem-capacity", "denied", "Filesystem statistics denied")
        } catch (_: RuntimeException) {
            MetadataProbe("filesystem-capacity", "unavailable", "Filesystem statistics unavailable")
        }
        probes.put(JSONObject().put("name", capacity.name).put("status", capacity.status).put("value", capacity.value))
        return JSONObject().put("uid", Process.myUid()).put("probes", probes).toString().also {
            check(it.toByteArray(Charsets.UTF_8).size <= ProbePolicy.MAX_HELPER_BYTES)
        }
    }
    private fun runFixedProbe(name: String): MetadataProbe {
        val probe = com.nexvary.npse.core.Probe.find(name) ?: return MetadataProbe(name, "unavailable", "Unsupported probe")
        val result = com.nexvary.npse.core.BoundedRunner().run(probe, com.nexvary.npse.core.BoundedRunner.Cancellation())
        val state = when (result.status) {
            com.nexvary.npse.core.BoundedRunner.Status.OK -> "ok"
            com.nexvary.npse.core.BoundedRunner.Status.DENIED -> "denied"
            else -> "unavailable"
        }
        return MetadataProbe(name, state, result.value)
    }
    override fun destroy() { exitProcess(0) }
}
