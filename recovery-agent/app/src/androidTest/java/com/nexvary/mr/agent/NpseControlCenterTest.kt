package com.nexvary.mr.agent
import android.os.ParcelFileDescriptor
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.Switch
import android.widget.TextView
import androidx.test.core.app.ActivityScenario
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.nexvary.npse.ControlCenterActivity
import com.nexvary.npse.R as NpseR
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
@RunWith(AndroidJUnit4::class)
class NpseControlCenterTest {
    private fun views(view: View): List<View> = listOf(view) + if (view is ViewGroup) (0 until view.childCount).flatMap { views(view.getChildAt(it)) } else emptyList()
    @Test fun worksWithoutManagerAndNeverEnablesShell() {
        ActivityScenario.launch(ControlCenterActivity::class.java).use { scenario ->
            scenario.onActivity { activity ->
                val all = views(activity.window.decorView)
                fun button(id: Int) = all.filterIsInstance<Button>().single { it.text == activity.getString(id) }
                assertFalse(all.filterIsInstance<Switch>().single().isChecked)
                assertFalse(button(NpseR.string.npse_advanced).isEnabled)
                button(NpseR.string.npse_standard).performClick()
                assertTrue(button(NpseR.string.npse_export).isEnabled)
                assertTrue("Standard report schema missing", all.filterIsInstance<TextView>().any {
                    runCatching { org.json.JSONObject(it.text.toString()).optString("schema") == "nexvary-npse/v1" }.getOrDefault(false)
                })
                all.filterIsInstance<Switch>().single().performClick()
                assertFalse(button(NpseR.string.npse_advanced).isEnabled)
                all.filterIsInstance<Switch>().single().performClick()
            }
            InstrumentationRegistry.getInstrumentation().waitForIdleSync()
            val descriptor = InstrumentationRegistry.getInstrumentation().uiAutomation.executeShellCommand("screencap -p /data/local/tmp/nexvary-npse-ui.png")
            ParcelFileDescriptor.AutoCloseInputStream(descriptor).bufferedReader().use { assertEquals("", it.readText().trim()) }
        }
    }
    @Test fun arabicStandardModeAndRtl() {
        if (android.os.Build.VERSION.SDK_INT < 33) return
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        val manager = context.getSystemService(android.app.LocaleManager::class.java)
        val previous = manager.applicationLocales
        try {
            manager.applicationLocales = android.os.LocaleList.forLanguageTags("ar")
            instrumentation.waitForIdleSync()
            ActivityScenario.launch(ControlCenterActivity::class.java).use { scenario ->
                scenario.onActivity { activity ->
                    val all = views(activity.window.decorView)
                    val title = activity.getString(NpseR.string.npse_title)
                    assertTrue("Arabic resources missing", title.contains("مركز"))
                    assertTrue("RTL content missing", all.any { it.layoutDirection == View.LAYOUT_DIRECTION_RTL })
                    all.filterIsInstance<Button>().single { it.text == activity.getString(NpseR.string.npse_standard) }.performClick()
                    assertTrue(all.filterIsInstance<Button>().single { it.text == activity.getString(NpseR.string.npse_export) }.isEnabled)
                }
                instrumentation.waitForIdleSync()
                val descriptor = instrumentation.uiAutomation.executeShellCommand("screencap -p /data/local/tmp/nexvary-npse-ui-ar.png")
                ParcelFileDescriptor.AutoCloseInputStream(descriptor).bufferedReader().use { assertEquals("", it.readText().trim()) }
            }
        } finally {
            manager.applicationLocales = previous
            instrumentation.waitForIdleSync()
        }
    }

}
