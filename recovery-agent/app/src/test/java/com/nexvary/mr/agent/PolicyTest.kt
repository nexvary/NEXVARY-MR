package com.nexvary.mr.agent

import org.junit.Assert.*
import org.junit.Test

class PolicyTest {
    @Test fun permissionStatesAndRevocation() {
        assertEquals(AccessState("unavailable", null), PermissionPolicy.evaluate(false, true, 2000, false))
        assertEquals(AccessState("permission-required", 2000), PermissionPolicy.evaluate(true, false, 2000, false))
        assertEquals(AccessState("denied", 2000), PermissionPolicy.evaluate(true, false, 2000, true))
        assertEquals(AccessState("shell", 2000), PermissionPolicy.evaluate(true, true, 2000, false))
        assertEquals(AccessState("permission-required", 2000), PermissionPolicy.evaluate(true, false, 2000, false))
        assertFalse(ProbePolicy.permitsHelper(2000, false, true))
        assertFalse(ProbePolicy.permitsHelper(2000, true, false))
    }
    @Test fun rootAndUnknownUidsRefusedEvenWithPermission() {
        assertEquals(AccessState("unsupported-root", null), PermissionPolicy.evaluate(true, true, 0, false))
        assertFalse(ProbePolicy.permitsHelper(0, true, true))
        assertFalse(ProbePolicy.permitsHelper(1000, true, true))
        assertEquals(AccessState("unavailable", null), PermissionPolicy.evaluate(true, true, 1000, false))
        assertTrue(ProbePolicy.permitsHelper(2000, true, true))
    }
    @Test fun commandSurfaceHasNoClientProvidedArguments() {
        assertEquals(listOf("/system/bin/id"), ProbePolicy.commandFor("shell-identity"))
        assertEquals(listOf("/system/bin/getprop", "sys.boot_completed"), ProbePolicy.commandFor("boot-completed"))
        assertEquals(listOf("/system/bin/getprop", "ro.crypto.state"), ProbePolicy.commandFor("reported-crypto-state"))
        assertEquals(listOf("/system/bin/getprop", "ro.crypto.type"), ProbePolicy.commandFor("reported-crypto-type"))
        for (name in listOf("sh", "getprop;reboot", "/data/user/0", "su", "id --context", "../../x", "ro.product.model")) assertNull(ProbePolicy.commandFor(name))
    }
}
