package com.nexvary.mr.agent

/** Pure policy; only these fixed metadata operations can reach the helper. */
object ProbePolicy {
    const val MAX_PROBE_BYTES = 4096
    const val MAX_HELPER_BYTES = 32 * 1024
    const val MAX_REPORT_BYTES = 64 * 1024
    const val MAX_FILE_BYTES = 16L * 1024 * 1024 * 1024
    val properties: Map<String, String> = linkedMapOf(
        "boot-completed" to "sys.boot_completed",
        "boot-reason" to "ro.boot.bootreason",
        "verified-boot-state" to "ro.boot.verifiedbootstate",
        "bootloader-lock-state" to "ro.boot.flash.locked",
        "reported-crypto-state" to "ro.crypto.state",
        "reported-crypto-type" to "ro.crypto.type"
    )
    val probeNames = properties.keys + setOf("shell-identity", "filesystem-capacity")
    fun commandFor(name: String): List<String>? = com.nexvary.npse.core.Probe.find(name)?.arguments()
    fun permitsHelper(uid: Int, granted: Boolean, alive: Boolean): Boolean =
        com.nexvary.npse.core.AccessPolicy.evaluate(alive, granted, uid, false) == com.nexvary.npse.core.AccessPolicy.State.SHELL
}

data class AccessState(val state: String, val uid: Int?)
object PermissionPolicy {
    fun evaluate(alive: Boolean, granted: Boolean, uid: Int?, denied: Boolean): AccessState = when {
        !alive -> AccessState("unavailable", null)
        uid == 0 -> AccessState("unsupported-root", null)
        uid != 2000 -> AccessState("unavailable", null)
        denied && !granted -> AccessState("denied", 2000)
        !granted -> AccessState("permission-required", 2000)
        else -> AccessState("shell", 2000)
    }
}
