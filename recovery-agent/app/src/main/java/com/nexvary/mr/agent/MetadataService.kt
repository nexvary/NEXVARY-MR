package com.nexvary.mr.agent

import android.os.Process
import android.os.StatFs
import org.json.JSONArray
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import kotlin.system.exitProcess

/** Shizuku UserService: fixed metadata only, no client-selected commands or paths. */
class MetadataService : IMetadataService.Stub() {
    override fun collectMetadata(): String {
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
        val command = ProbePolicy.commandFor(name) ?: return MetadataProbe(name, "unavailable", "Unsupported probe")
        var child: java.lang.Process? = null
        val reader = Executors.newSingleThreadExecutor()
        return try {
            child = ProcessBuilder(command).redirectErrorStream(true).start()
            val process = child
            val output = reader.submit<String> {
                val bytes = ByteArrayOutputStream()
                process.inputStream.use { input ->
                    val buffer = ByteArray(512)
                    while (true) {
                        val count = input.read(buffer)
                        if (count < 0) break
                        if (bytes.size() + count > ProbePolicy.MAX_PROBE_BYTES) {
                            process.destroyForcibly()
                            throw IllegalStateException("Probe output exceeded limit")
                        }
                        bytes.write(buffer, 0, count)
                    }
                }
                bytes.toString(Charsets.UTF_8.name()).trim()
            }
            if (!process.waitFor(1500, TimeUnit.MILLISECONDS)) {
                process.destroyForcibly()
                MetadataProbe(name, "unavailable", "Probe timed out")
            } else {
                val text = output.get(500, TimeUnit.MILLISECONDS)
                MetadataProbe(name, if (process.exitValue() == 0) "ok" else "denied", text)
            }
        } catch (_: SecurityException) {
            MetadataProbe(name, "denied", "Metadata probe denied")
        } catch (_: Exception) {
            MetadataProbe(name, "unavailable", "Metadata probe unavailable or exceeded its limit")
        } finally {
            child?.destroyForcibly()
            runCatching { child?.inputStream?.close() }
            reader.shutdownNow()
        }
    }
    override fun destroy() { exitProcess(0) }
}
