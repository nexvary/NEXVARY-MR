package com.nexvary.mr.agent

import android.app.Activity
import android.os.ParcelFileDescriptor
import android.os.SystemClock
import android.view.View
import android.view.ViewGroup
import android.view.accessibility.AccessibilityNodeInfo
import android.widget.Button
import android.widget.Switch
import android.widget.TextView
import androidx.test.core.app.ActivityScenario
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.nexvary.npse.ControlCenterActivity
import com.nexvary.npse.ShizukuAccess
import com.nexvary.npse.core.AccessPolicy
import com.nexvary.npse.R as NpseR
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Assume.assumeTrue
import org.junit.Test
import org.junit.runner.RunWith
import rikka.shizuku.Shizuku

/** Opt-in only: disposable CI emulator with a verified manager started as shell. */
@RunWith(AndroidJUnit4::class)
class ShizukuShellIntegrationTest {
    private val instrumentation get() = InstrumentationRegistry.getInstrumentation()
    private fun descendants(v: View): List<View> = listOf(v) + if (v is ViewGroup) (0 until v.childCount).flatMap { descendants(v.getChildAt(it)) } else emptyList()
    private fun await(label: String, condition: () -> Boolean) {
        val deadline = SystemClock.elapsedRealtime() + 20_000
        while (SystemClock.elapsedRealtime() < deadline) {
            instrumentation.waitForIdleSync()
            if (condition()) return
            SystemClock.sleep(100)
        }
        throw AssertionError("Timed out: $label")
    }
    private fun button(activity: Activity, id: Int) = descendants(activity.window.decorView).filterIsInstance<Button>().single { it.text == activity.getString(id) }
    private fun report(activity: Activity): JSONObject? = descendants(activity.window.decorView).filterIsInstance<TextView>().firstNotNullOfOrNull {
        runCatching { JSONObject(it.text.toString()).takeIf { obj -> obj.optString("schema") == "nexvary-npse/v1" } }.getOrNull()
    }
    private fun node(root: AccessibilityNodeInfo?, id: String): AccessibilityNodeInfo? {
        if (root == null) return null
        if (root.viewIdResourceName == id) return root
        for (i in 0 until root.childCount) node(root.getChild(i), id)?.let { return it }
        return null
    }
    @Test fun consentedShellCollectionAndRealServiceDeathFallBackToStandard() {
        assumeTrue(InstrumentationRegistry.getArguments().getString("npseManagerIntegration") == "true")
        await("manager Binder delivery") { Shizuku.pingBinder() }
        assertEquals("Root must not be used", 2000, Shizuku.getUid())
        assertEquals(AccessPolicy.State.PERMISSION_REQUIRED, ShizukuAccess.state())
        ActivityScenario.launch(ControlCenterActivity::class.java).use { scenario ->
            scenario.onActivity { activity ->
                button(activity, NpseR.string.npse_standard).performClick()
                descendants(activity.window.decorView).filterIsInstance<Switch>().single().performClick()
                assertTrue(button(activity, NpseR.string.npse_grant).isEnabled)
                button(activity, NpseR.string.npse_grant).performClick()
            }
            // The source-reviewed manager dialog uses android:id/button1 in both variants.
            // Never click a similarly named control belonging to a different application.
            await("explicit manager authorization dialog") {
                val root = instrumentation.uiAutomation.rootInActiveWindow
                if (root?.packageName?.toString() != "moe.shizuku.privileged.api") false
                else node(root, "android:id/button1")?.performAction(AccessibilityNodeInfo.ACTION_CLICK) == true
            }
            await("shell permission granted") { ShizukuAccess.state() == AccessPolicy.State.SHELL }
            await("control center resumed") {
                var ready = false
                scenario.onActivity { ready = button(it, NpseR.string.npse_advanced).isEnabled }
                ready
            }
            scenario.onActivity { button(it, NpseR.string.npse_advanced).performClick() }
            await("per-action collection confirmation") {
                val root = instrumentation.uiAutomation.rootInActiveWindow
                if (root?.packageName?.toString() != instrumentation.targetContext.packageName) false
                else node(root, "android:id/button1")?.performAction(AccessibilityNodeInfo.ACTION_CLICK) == true
            }
            await("validated live shell observations") {
                var complete = false
                scenario.onActivity { activity ->
                    val privileged = report(activity)?.optJSONObject("privileged")
                    val rows = privileged?.optJSONArray("observations")
                    if (rows != null && rows.length() == 4) {
                        assertEquals(2000, privileged.getInt("uid"))
                        val identity = (0 until rows.length()).map { rows.getJSONObject(it) }.single { it.getString("operation") == "shell-identity" }
                        assertEquals("OK", identity.getString("status"))
                        assertTrue(identity.getString("value").contains("uid=2000"))
                        complete = true
                    }
                }
                complete
            }
            val capture = instrumentation.uiAutomation.executeShellCommand("screencap -p /data/local/tmp/npse-shizuku-shell.png")
            ParcelFileDescriptor.AutoCloseInputStream(capture).use { it.readBytes() }
            // Fixed test-only operation, on the emulator-only process started by our CI script.
            val stop = instrumentation.uiAutomation.executeShellCommand("/system/bin/killall -TERM shizuku_server")
            ParcelFileDescriptor.AutoCloseInputStream(stop).use { it.readBytes() }
            await("actual Binder death") { !Shizuku.pingBinder() }
            await("privileged results invalidated") {
                var cleared = false
                scenario.onActivity { activity ->
                    cleared = !button(activity, NpseR.string.npse_advanced).isEnabled && report(activity)?.isNull("privileged") == true
                }
                cleared
            }
            scenario.onActivity { activity ->
                button(activity, NpseR.string.npse_standard).performClick()
                assertTrue(button(activity, NpseR.string.npse_export).isEnabled)
                assertTrue(report(activity)?.isNull("privileged") == true)
            }
        }
    }
}
