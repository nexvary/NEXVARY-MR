package com.nexvary.npse

import org.json.JSONObject

object DiagnosticsProtocol {
    private val operations = setOf("shell-identity", "verified-boot-state", "bootloader-lock-state", "memory-totals")
    private val statuses = com.nexvary.npse.core.BoundedRunner.Status.values().map { it.name }.toSet()
    fun validate(text: String): JSONObject {
        require(text.toByteArray(Charsets.UTF_8).size <= 24 * 1024)
        val root = JSONObject(text)
        require(root.getString("provider") == "shizuku-shell" && root.getInt("uid") == 2000)
        val rows = root.getJSONArray("observations")
        require(rows.length() <= operations.size)
        val seen = mutableSetOf<String>()
        val clean = org.json.JSONArray()
        for (i in 0 until rows.length()) {
            val row = rows.getJSONObject(i)
            val operation = row.getString("operation")
            val status = row.getString("status")
            val value = row.getString("value")
            require(operation in operations && seen.add(operation) && status in statuses)
            require(value.toByteArray(Charsets.UTF_8).size <= 4096)
            require(value.none { it == '\u0000' || (it.isISOControl() && it != '\n' && it != '\t') })
            require(status == "OK" || value.isEmpty())
            clean.put(JSONObject().put("operation", operation).put("status", status).put("value", value))
        }
        return JSONObject().put("provider", "shizuku-shell").put("uid", 2000).put("observations", clean)
    }
}
