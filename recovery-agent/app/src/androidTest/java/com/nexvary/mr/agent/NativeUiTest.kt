package com.nexvary.mr.agent

import android.os.ParcelFileDescriptor
import android.content.pm.ApplicationInfo
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
import java.util.concurrent.atomic.AtomicBoolean
import org.xmlpull.v1.XmlPullParser

/** Runs on a real Android emulator in CI; never claims phone or Shizuku coverage. */
@RunWith(AndroidJUnit4::class)
class NativeUiTest {
    private fun descendants(view: View): List<View> = listOf(view) + if (view is ViewGroup)
        (0 until view.childCount).flatMap { descendants(view.getChildAt(it)) } else emptyList()
    @Test fun backupScopesExcludeAllAppDataDomains() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        assertEquals(0, context.applicationInfo.flags and ApplicationInfo.FLAG_ALLOW_BACKUP)
        val expected = setOf("root", "file", "database", "sharedpref", "external",
            "device_root", "device_file", "device_database", "device_sharedpref")
        val scopes = mutableMapOf<String, MutableSet<String>>()
        var scope: String? = null
        context.resources.getXml(R.xml.data_extraction_rules).use { xml ->
            while (xml.eventType != XmlPullParser.END_DOCUMENT) {
                if (xml.eventType == XmlPullParser.START_TAG) {
                    if (xml.name in setOf("cloud-backup", "device-transfer")) {
                        scope = xml.name; scopes[xml.name] = mutableSetOf()
                    } else if (xml.name == "exclude") {
                        assertEquals(".", xml.getAttributeValue(null, "path"))
                        scopes.getValue(requireNotNull(scope)).add(xml.getAttributeValue(null, "domain"))
                    }
                } else if (xml.eventType == XmlPullParser.END_TAG && xml.name == scope) scope = null
                xml.next()
            }
        }
        assertEquals(setOf("cloud-backup", "device-transfer"), scopes.keys)
        scopes.values.forEach { assertEquals(expected, it) }
    }
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
            // UTP uninstalls both APKs at the end of connectedDebugAndroidTest.
            // Keep this synthetic emulator-only artifact in shell-owned temporary
            // storage, rather than the application's subsequently removed cache.
            val capture = instrumentation.uiAutomation.executeShellCommand(
                "screencap -p /data/local/tmp/nexvary-agent-emulator-ui.png")
            ParcelFileDescriptor.AutoCloseInputStream(capture).bufferedReader().use {
                assertEquals("Emulator screenshot capture failed", "", it.readText().trim())
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
