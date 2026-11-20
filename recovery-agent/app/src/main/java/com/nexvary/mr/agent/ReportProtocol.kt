package com.nexvary.mr.agent

import org.json.JSONArray
import org.json.JSONObject
import java.time.Instant

/** Bounded, offline metadata report. A report is an observation, not evidence of ownership. */
data class DeviceMetadata(val manufacturer: String, val model: String, val sdkInt: Int,
                          val release: String, val securityPatch: String)
data class StorageMetadata(val description: String, val state: String, val primary: Boolean,
                           val removable: Boolean, val emulated: Boolean)
data class MetadataProbe(val name: String, val status: String, val value: String)
data class SelectedFileMetadata(val name: String, val size: Long, val sha256: String)

object ReportProtocol {
    private fun bounded(value: String, maximum: Int, multiline: Boolean = false): String {
        require(value.length <= maximum) { "Metadata field exceeds its limit" }
        require(value.none { it == '\u0000' || (it.isISOControl() && !(multiline && (it == '\n' || it == '\t'))) }) { "Invalid control character" }
        return value
    }
    fun build(consent: Boolean, generatedAtUtc: String, device: DeviceMetadata,
              access: AccessState, storage: List<StorageMetadata>, probes: List<MetadataProbe>,
              selectedFile: SelectedFileMetadata?, audit: List<String>): String {
        require(consent) { "Explicit owner consent is required" }
        Instant.parse(generatedAtUtc)
        require(device.sdkInt in 26..100)
        require(storage.size <= 16 && probes.size <= 16 && audit.size <= 100)
        require(access.state in setOf("unavailable", "permission-required", "denied", "shell", "unsupported-root"))
        require(access.uid == null || access.uid == 2000)
        require(access.state != "shell" || access.uid == 2000)
        require(access.state !in setOf("unavailable", "unsupported-root") || access.uid == null)
        require(access.state !in setOf("permission-required", "denied") || access.uid == 2000)
        val volumes = JSONArray()
        storage.forEach { volumes.put(JSONObject().put("description", bounded(it.description, 160))
            .put("state", bounded(it.state, 40)).put("primary", it.primary)
            .put("removable", it.removable).put("emulated", it.emulated)) }
        val observations = JSONArray()
        // Never export cached privileged observations after revocation, death or root detection.
        if (access.state == "shell") {
            require(probes.map { it.name }.distinct().size == probes.size)
            probes.forEach {
            require(it.name in ProbePolicy.probeNames)
            require(it.status in setOf("ok", "denied", "unavailable"))
            observations.put(JSONObject().put("name", it.name).put("status", it.status)
                .put("value", bounded(it.value, ProbePolicy.MAX_PROBE_BYTES, true)))
            }
        }
        val selected = selectedFile?.let {
            require(it.size in 0..ProbePolicy.MAX_FILE_BYTES)
            require(it.sha256.matches(Regex("[0-9a-f]{64}")))
            require(it.name.isNotBlank())
            JSONObject().put("name", bounded(it.name, 200)).put("size", it.size).put("sha256", it.sha256)
        } ?: JSONObject.NULL
        val history = JSONArray()
        audit.forEach { history.put(bounded(it, 256)) }
        val report = JSONObject().put("schema", "nexvary-recovery-agent/v1")
            .put("generatedAtUtc", generatedAtUtc)
            .put("consent", JSONObject().put("confirmed", true).put("scope", "device-metadata"))
            .put("device", JSONObject().put("manufacturer", bounded(device.manufacturer, 128))
                .put("model", bounded(device.model, 128)).put("sdkInt", device.sdkInt)
                .put("release", bounded(device.release, 64)).put("securityPatch", bounded(device.securityPatch, 32)))
            .put("shizuku", JSONObject().put("state", access.state).put("uid", access.uid ?: JSONObject.NULL))
            .put("storage", volumes).put("probes", observations).put("selectedFile", selected).put("audit", history)
        return report.toString(2).also { require(it.toByteArray(Charsets.UTF_8).size <= ProbePolicy.MAX_REPORT_BYTES) { "Report exceeds 64 KiB" } }
    }
    fun parseHelper(json: String): List<MetadataProbe> {
        require(json.toByteArray(Charsets.UTF_8).size <= ProbePolicy.MAX_HELPER_BYTES)
        val objectValue = JSONObject(json)
        require(objectValue.getInt("uid") == 2000) { "Helper is not running as shell" }
        val array = objectValue.getJSONArray("probes")
        require(array.length() <= 16)
        val result = mutableListOf<MetadataProbe>()
        val seen = mutableSetOf<String>()
        for (i in 0 until array.length()) {
            val probe = array.getJSONObject(i)
            val name = probe.getString("name")
            val status = probe.getString("status")
            require(name in ProbePolicy.probeNames && seen.add(name))
            require(status in setOf("ok", "denied", "unavailable"))
            result += MetadataProbe(name, status, bounded(probe.getString("value"), ProbePolicy.MAX_PROBE_BYTES, true))
        }
        return result
    }
}
