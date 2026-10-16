package com.nexvary.mr.agent

import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test

class ReportProtocolTest {
    private val device = DeviceMetadata("Synthetic", "Training-Only", 35, "15", "2026-01-01")
    private val instant = "2026-10-16T12:00:00Z"
    private fun build(access: AccessState = AccessState("shell", 2000), probes: List<MetadataProbe> = listOf(MetadataProbe("shell-identity", "ok", "uid=2000(shell)")), selected: SelectedFileMetadata? = null, consent: Boolean = true, audit: List<String> = emptyList()) =
        ReportProtocol.build(consent, instant, device, access, emptyList(), probes, selected, audit)
    private fun rejects(action: () -> Unit) {
        try { action(); fail("Invalid report input was accepted") } catch (_: IllegalArgumentException) { }
    }
    @Test fun reportIsExplicitBoundedMetadataAndNotFileContents() {
        val text = build(selected = SelectedFileMetadata("training.bin", 1024, "a".repeat(64)))
        val report = JSONObject(text)
        assertEquals("nexvary-recovery-agent/v1", report.getString("schema"))
        assertTrue(report.getJSONObject("consent").getBoolean("confirmed"))
        assertEquals("device-metadata", report.getJSONObject("consent").getString("scope"))
        assertEquals(1024L, report.getJSONObject("selectedFile").getLong("size"))
        assertEquals(3, report.getJSONObject("selectedFile").length())
        assertFalse(text.contains("content://")); assertFalse(report.has("fileContents")); assertFalse(report.has("serial"))
        assertTrue(text.toByteArray().size <= ProbePolicy.MAX_REPORT_BYTES)
    }
    @Test fun revocationAndRootCannotExportStalePrivilegedResults() {
        for (state in listOf(AccessState("permission-required", 2000), AccessState("denied", 2000), AccessState("unavailable", null), AccessState("unsupported-root", null))) {
            val report = JSONObject(build(access = state))
            assertEquals(0, report.getJSONArray("probes").length())
        }
        rejects { build(access = AccessState("shell", 0)) }
        rejects { build(access = AccessState("unsupported-root", 2000)) }
        rejects { build(consent = false) }
    }
    @Test fun fieldsProbeNamesAndFileBoundsAreValidated() {
        rejects { build(probes = listOf(MetadataProbe("private-files", "ok", "x"))) }
        rejects { build(probes = listOf(MetadataProbe("shell-identity", "ok", "x".repeat(4097)))) }
        rejects { build(probes = listOf(MetadataProbe("shell-identity", "ok", "a"), MetadataProbe("shell-identity", "ok", "b"))) }
        rejects { build(selected = SelectedFileMetadata("file", -1, "a".repeat(64))) }
        rejects { build(selected = SelectedFileMetadata("file", ProbePolicy.MAX_FILE_BYTES + 1, "a".repeat(64))) }
        rejects { build(selected = SelectedFileMetadata("file", 1, "bad-hash")) }
        rejects { build(selected = SelectedFileMetadata("a\u0000hidden", 1, "a".repeat(64))) }
        rejects { build(audit = List(101) { "x" }) }
        rejects { ReportProtocol.build(true, instant, device, AccessState("shell", 2000), List(17) { StorageMetadata("s", "mounted", true, false, true) }, emptyList(), null, emptyList()) }
    }
    @Test fun encodedByteLimitRejectsMultibyteMetadata() {
        rejects { build(audit = List(100) { "界".repeat(250) }) }
    }
    @Test fun helperRootDuplicateAndUnboundedResponsesRejected() {
        rejects { ReportProtocol.parseHelper("{\"uid\":0,\"probes\":[]}") }
        rejects { ReportProtocol.parseHelper("x".repeat(ProbePolicy.MAX_HELPER_BYTES + 1)) }
        val duplicate = """{"uid":2000,"probes":[{"name":"boot-completed","status":"ok","value":"1"},{"name":"boot-completed","status":"ok","value":"0"}]}"""
        rejects { ReportProtocol.parseHelper(duplicate) }
        assertEquals(1, ReportProtocol.parseHelper("""{"uid":2000,"probes":[{"name":"boot-completed","status":"ok","value":"1"}]}""").size)
    }
}
