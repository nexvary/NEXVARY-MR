package com.nexvary.npse.core;
import java.util.List;
/** Fixed read-only operations. No caller-selected argv, shell scripts or paths. */
public enum Probe {
    IDENTITY("shell-identity", "/system/bin/id"),
    BOOT_COMPLETED("boot-completed", "/system/bin/getprop", "sys.boot_completed"),
    BOOT_REASON("boot-reason", "/system/bin/getprop", "ro.boot.bootreason"),
    VERIFIED_BOOT("verified-boot-state", "/system/bin/getprop", "ro.boot.verifiedbootstate"),
    BOOTLOADER_LOCK("bootloader-lock-state", "/system/bin/getprop", "ro.boot.flash.locked"),
    CRYPTO_STATE("reported-crypto-state", "/system/bin/getprop", "ro.crypto.state"),
    CRYPTO_TYPE("reported-crypto-type", "/system/bin/getprop", "ro.crypto.type"),
    MEMORY_TOTALS("memory-totals", "/system/bin/cat", "/proc/meminfo");
    public final String id;
    private final List<String> argv;
    Probe(String id, String... argv) { this.id = id; this.argv = java.util.Collections.unmodifiableList(java.util.Arrays.asList(argv.clone())); }
    public List<String> arguments() { return argv; }
    public static Probe find(String id) {
        for (Probe probe : values()) if (probe.id.equals(id)) return probe;
        return null;
    }
}
