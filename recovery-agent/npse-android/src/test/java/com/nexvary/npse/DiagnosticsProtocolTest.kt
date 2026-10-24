package com.nexvary.npse
import org.junit.Assert.*
import org.junit.Test
import org.json.JSONObject
class DiagnosticsProtocolTest {
    private fun sample() = """{"provider":"shizuku-shell","uid":2000,"observations":[{"operation":"shell-identity","status":"OK","value":"uid=2000(shell)"}]}"""
    @Test fun validBoundedObservation() { assertEquals(2000, DiagnosticsProtocol.validate(sample()).getInt("uid")) }
    @Test fun rejectsRootUnknownOperationsAndStatuses() {
        for (text in listOf(sample().replace("2000", "0"), sample().replace("shell-identity", "reboot"), sample().replace("OK", "PASSED"), sample().replace("uid=2000(shell)", "x".repeat(4097)), sample().replace("OK", "DENIED"))) {
            assertThrows(IllegalArgumentException::class.java) { DiagnosticsProtocol.validate(text) }
        }
    }
    @Test fun rejectsDuplicatesAndControls() {
        val json = JSONObject(sample()); val rows = json.getJSONArray("observations"); rows.put(rows.getJSONObject(0))
        assertThrows(IllegalArgumentException::class.java) { DiagnosticsProtocol.validate(json.toString()) }
        val controls = JSONObject(sample()); controls.getJSONArray("observations").getJSONObject(0).put("value", "bad\u0000data")
        assertThrows(IllegalArgumentException::class.java) { DiagnosticsProtocol.validate(controls.toString()) }
    }
    @Test fun discardsUnrecognizedFields() {
        val json = JSONObject(sample()).put("extra", "private-data")
        assertFalse(DiagnosticsProtocol.validate(json.toString()).has("extra"))
    }
}
