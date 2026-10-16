package com.nexvary.mr.agent

import android.graphics.Bitmap
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.CheckBox
import androidx.test.core.app.ActivityScenario
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.util.concurrent.atomic.AtomicBoolean

/** Runs on a real Android emulator in CI; never claims phone or Shizuku coverage. */
@RunWith(AndroidJUnit4::class)
class NativeUiTest {
    private fun descendants(view: View): List<View> = listOf(view) + if (view is ViewGroup)
        (0 until view.childCount).flatMap { descendants(view.getChildAt(it)) } else emptyList()
    @Test fun consentPublicMetadataAndShellGate() {
        ActivityScenario.launch(MainActivity::class.java).use { scenario ->
            scenario.onActivity { activity ->
                val views = descendants(activity.window.decorView)
                val consent = views.filterIsInstance<CheckBox>().single()
                fun button(id: Int) = views.filterIsInstance<Button>().single { it.text == activity.getString(id) }
                assertFalse(consent.isChecked)
                assertFalse(button(R.string.collect_public).isEnabled)
                assertFalse(button(R.string.collect_shell).isEnabled)
                assertFalse(button(R.string.export_report).isEnabled)
                consent.performClick()
                assertTrue(button(R.string.collect_public).isEnabled)
                button(R.string.collect_public).performClick()
            }
            val completed = AtomicBoolean(false)
            val deadline = System.nanoTime() + 10_000_000_000L
            while (System.nanoTime() < deadline && !completed.get()) {
                InstrumentationRegistry.getInstrumentation().waitForIdleSync()
                scenario.onActivity { activity ->
                    completed.set(descendants(activity.window.decorView).filterIsInstance<Button>()
                        .single { it.text == activity.getString(R.string.export_report) }.isEnabled)
                }
                if (!completed.get()) Thread.sleep(50)
            }
            assertTrue("Public metadata did not finish", completed.get())
            InstrumentationRegistry.getInstrumentation().waitForIdleSync()
            val instrumentation = InstrumentationRegistry.getInstrumentation()
            val image = requireNotNull(instrumentation.uiAutomation.takeScreenshot())
            File(instrumentation.targetContext.cacheDir, "emulator-native-ui.png").outputStream().use {
                assertTrue(image.compress(Bitmap.CompressFormat.PNG, 100, it))
            }
            scenario.onActivity { activity ->
                val views = descendants(activity.window.decorView)
                val consent = views.filterIsInstance<CheckBox>().single()
                val shell = views.filterIsInstance<Button>().single { it.text == activity.getString(R.string.collect_shell) }
                // CI has no Shizuku manager; optional privileged collection must stay gated.
                assertFalse(shell.isEnabled)
                consent.performClick()
                assertFalse(views.filterIsInstance<Button>().single { it.text == activity.getString(R.string.export_report) }.isEnabled)
            }
        }
    }
}
