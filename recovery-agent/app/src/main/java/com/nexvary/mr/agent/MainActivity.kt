package com.nexvary.mr.agent

import android.app.Activity
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.graphics.Color
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.CancellationSignal
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.storage.StorageManager
import android.provider.OpenableColumns
import android.view.View
import android.view.WindowInsets
import android.widget.Button
import android.widget.CheckBox
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.ScrollView
import android.widget.TextView
import rikka.shizuku.Shizuku
import java.io.Closeable
import java.security.MessageDigest
import java.time.Instant
import java.util.concurrent.Executors
import java.util.concurrent.Future
import java.util.concurrent.atomic.AtomicReference

/** No network, desktop installation, private-data collection, or recovery writes. */
class MainActivity : Activity() {
    private val handler = Handler(Looper.getMainLooper())
    private val worker = Executors.newSingleThreadExecutor()
    private var active: Future<*>? = null
    private var generation = 0
    private var inShellTask = false
    private var permissionDenied = false
    private var serviceRequested = false
    private var serviceArgs: Shizuku.UserServiceArgs? = null
    private val openResource = AtomicReference<Closeable?>()
    private val readCancellation = AtomicReference<CancellationSignal?>()
    private var device: DeviceMetadata? = null
    private var volumes = emptyList<StorageMetadata>()
    private var probes = emptyList<MetadataProbe>()
    private var selected: SelectedFileMetadata? = null
    private var pendingReport: String? = null
    private val audit = mutableListOf<String>()
    private lateinit var consent: CheckBox
    private lateinit var status: TextView
    private lateinit var accessLabel: TextView
    private lateinit var preview: TextView
    private lateinit var progress: ProgressBar
    private lateinit var collectButton: Button
    private lateinit var permissionButton: Button
    private lateinit var shellButton: Button
    private lateinit var fileButton: Button
    private lateinit var exportButton: Button
    private lateinit var cancelButton: Button
    private val binderReceived = Shizuku.OnBinderReceivedListener { refreshAccess() }
    private val binderDead = Shizuku.OnBinderDeadListener {
        probes = emptyList()
        if (inShellTask) cancelActive(getString(R.string.service_stopped))
        refreshAccess()
    }
    private val permissionResult = Shizuku.OnRequestPermissionResultListener { requestCode, grantResult ->
        if (requestCode == PERMISSION_REQUEST) {
            permissionDenied = grantResult != PackageManager.PERMISSION_GRANTED
            probes = emptyList()
            event(if (permissionDenied) "Shizuku permission denied" else "Shizuku permission granted")
            refreshAccess()
        }
    }
    private val connection = object : ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, binder: IBinder?) {
            if (!consent.isChecked || currentAccess().state != "shell" || binder == null) {
                releaseService(); return
            }
            handler.removeCallbacks(bindTimeout)
            val helper = IMetadataService.Stub.asInterface(binder)
            inShellTask = true
            runTask(getString(R.string.collecting_shell), 18_000L, {
                ReportProtocol.parseHelper(helper.collectMetadata())
            }) { result ->
                if (currentAccess().state == "shell" && consent.isChecked) {
                    probes = result
                    event("Fixed shell metadata probes completed")
                    status.text = getString(R.string.shell_complete)
                } else probes = emptyList()
                releaseService()
                refreshAccess()
            }
        }
        override fun onServiceDisconnected(name: ComponentName?) {
            probes = emptyList()
            if (inShellTask) cancelActive(getString(R.string.service_stopped))
            refreshAccess()
        }
    }
    private val bindTimeout = Runnable {
        if (serviceRequested && active == null) {
            releaseService(); status.text = getString(R.string.bind_timeout); render()
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        buildUi()
        Shizuku.addBinderReceivedListenerSticky(binderReceived)
        Shizuku.addBinderDeadListener(binderDead)
        Shizuku.addRequestPermissionResultListener(permissionResult)
        refreshAccess()
    }
    private fun buildUi() {
        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(Color.rgb(5, 17, 29))
            setPadding(dp(18), dp(18), dp(18), dp(18))
        }
        root.setOnApplyWindowInsetsListener { view, insets ->
            if (Build.VERSION.SDK_INT >= 30) {
                val bars = insets.getInsets(WindowInsets.Type.systemBars())
                view.setPadding(dp(18) + bars.left, dp(18) + bars.top, dp(18) + bars.right, dp(18) + bars.bottom)
            } else {
                @Suppress("DEPRECATION")
                view.setPadding(dp(18), dp(18) + insets.systemWindowInsetTop, dp(18), dp(18) + insets.systemWindowInsetBottom)
            }
            insets
        }
        val scroll = ScrollView(this)
        val content = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL }
        scroll.addView(content)
        root.addView(scroll, LinearLayout.LayoutParams(-1, -1))
        fun label(text: String, size: Float = 15f, warning: Boolean = false): TextView = TextView(this).apply {
            this.text = text; textSize = size
            setTextColor(if (warning) Color.rgb(244, 188, 112) else Color.rgb(193, 215, 233))
            setPadding(0, dp(8), 0, dp(8)); content.addView(this)
        }
        fun button(resource: Int, action: () -> Unit): Button = Button(this).apply {
            setText(resource); isAllCaps = false
            content.addView(this, LinearLayout.LayoutParams(-1, -2))
            setOnClickListener { action() }
        }
        label(getString(R.string.app_name), 25f)
        label(getString(R.string.subtitle))
        label(getString(R.string.install_warning), 15f, true)
        consent = CheckBox(this).apply {
            setText(R.string.consent); setTextColor(Color.rgb(220, 235, 247)); content.addView(this)
            setOnCheckedChangeListener { _, checked ->
                pendingReport = null
                if (!checked) {
                    cancelActive(getString(R.string.consent_revoked)); device = null; volumes = emptyList()
                    probes = emptyList(); selected = null; event("Owner consent revoked; volatile metadata cleared")
                } else event("Explicit owner consent confirmed for device metadata")
                render()
            }
        }
        accessLabel = label("")
        collectButton = button(R.string.collect_public) { collectPublic() }
        permissionButton = button(R.string.request_shizuku) { requestShizuku() }
        shellButton = button(R.string.collect_shell) { collectShell() }
        fileButton = button(R.string.choose_file) { chooseFile() }
        label(getString(R.string.file_scope), 13f)
        progress = ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal).apply {
            max = 100; visibility = View.GONE; content.addView(this, LinearLayout.LayoutParams(-1, dp(12)))
        }
        cancelButton = button(R.string.cancel) { cancelActive(getString(R.string.canceled)) }
        exportButton = button(R.string.export_report) { requestExport() }
        label(getString(R.string.export_warning), 13f, true)
        status = label(getString(R.string.ready))
        preview = label("").apply { setTextIsSelectable(true); textSize = 12f }
        setContentView(root)
        root.requestApplyInsets()
        render()
    }
    private fun currentAccess(): AccessState = try {
        val alive = Shizuku.pingBinder()
        if (!alive || Shizuku.getVersion() < 12) AccessState("unavailable", null)
        else PermissionPolicy.evaluate(true, Shizuku.checkSelfPermission() == PackageManager.PERMISSION_GRANTED,
            Shizuku.getUid(), permissionDenied)
    } catch (_: RuntimeException) { AccessState("unavailable", null) }
    private fun refreshAccess() {
        if (!::accessLabel.isInitialized) return
        val access = currentAccess()
        if (access.state != "shell") {
            probes = emptyList()
            if (inShellTask) cancelActive(getString(R.string.permission_revoked))
        }
        accessLabel.text = getString(R.string.shizuku_state, access.state)
        if (access.state == "unsupported-root") status.setText(R.string.root_refused)
        render()
    }
    private fun render() {
        if (!::collectButton.isInitialized) return
        val access = currentAccess()
        val idle = active == null && !serviceRequested
        val allowed = consent.isChecked && idle
        collectButton.isEnabled = allowed
        permissionButton.isEnabled = allowed && access.state in setOf("permission-required", "denied", "unavailable")
        shellButton.isEnabled = allowed && access.state == "shell"
        fileButton.isEnabled = allowed
        exportButton.isEnabled = allowed && device != null
        cancelButton.isEnabled = !idle
        progress.visibility = if (idle) View.GONE else View.VISIBLE
        if (::preview.isInitialized) preview.text = if (consent.isChecked && device != null) runCatching { buildReport() }.getOrElse { getString(R.string.report_not_ready) } else ""
    }
    private fun collectPublic() {
        if (!consent.isChecked) return
        runTask(getString(R.string.collecting_public), 5_000L, {
            val metadata = DeviceMetadata(Build.MANUFACTURER.take(128), Build.MODEL.take(128), Build.VERSION.SDK_INT,
                Build.VERSION.RELEASE.take(64), Build.VERSION.SECURITY_PATCH.take(32))
            val manager = getSystemService(StorageManager::class.java)
            val storage = runCatching { manager.storageVolumes.take(16).map {
                StorageMetadata(it.getDescription(this).take(160), it.state.take(40), it.isPrimary, it.isRemovable, it.isEmulated)
            } }.getOrDefault(emptyList())
            metadata to storage
        }) { (metadata, storage) ->
            device = metadata; volumes = storage
            event("Public SDK metadata collected; visible volumes=${storage.size}")
            status.text = getString(R.string.public_complete); render()
        }
    }
    private fun requestShizuku() {
        if (!consent.isChecked) return
        try {
            val access = currentAccess()
            when {
                access.state == "unsupported-root" -> status.setText(R.string.root_refused)
                access.state == "unavailable" -> status.setText(R.string.shizuku_setup)
                Shizuku.shouldShowRequestPermissionRationale() -> status.setText(R.string.permission_settings)
                else -> Shizuku.requestPermission(PERMISSION_REQUEST)
            }
        } catch (_: RuntimeException) { status.setText(R.string.shizuku_setup) }
        refreshAccess()
    }
    private fun collectShell() {
        val access = currentAccess()
        if (!consent.isChecked || !ProbePolicy.permitsHelper(access.uid ?: -1, access.state == "shell", Shizuku.pingBinder())) {
            probes = emptyList(); status.setText(R.string.root_or_permission); refreshAccess(); return
        }
        try {
            probes = emptyList()
            serviceArgs = Shizuku.UserServiceArgs(ComponentName(this, MetadataService::class.java))
                .tag("nexvary-metadata-v1").version(1).daemon(false).debuggable(BuildConfig.DEBUG).processNameSuffix("metadata")
            serviceRequested = true
            progress.isIndeterminate = true
            status.setText(R.string.connecting_shell)
            Shizuku.bindUserService(requireNotNull(serviceArgs), connection)
            handler.postDelayed(bindTimeout, 5_000L)
            event("Explicit fixed-metadata shell helper requested")
            render()
        } catch (_: RuntimeException) {
            releaseService(); status.setText(R.string.bind_timeout); render()
        }
    }
    private fun releaseService() {
        handler.removeCallbacks(bindTimeout)
        if (serviceRequested) runCatching { serviceArgs?.let { Shizuku.unbindUserService(it, connection, true) } }
        serviceRequested = false; serviceArgs = null; inShellTask = false
    }
    private fun chooseFile() {
        if (!consent.isChecked) return
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("*/*").putExtra(Intent.EXTRA_LOCAL_ONLY, true).addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        startActivityForResult(intent, PICK_FILE)
    }
    private fun localProvider(uri: Uri): Boolean = uri.scheme == "content" && uri.authority in setOf(
        "com.android.externalstorage.documents", "com.android.providers.downloads.documents", "com.android.providers.media.documents")
    private fun hashFile(uri: Uri) {
        selected = null
        if (!consent.isChecked || !localProvider(uri)) { status.setText(R.string.local_provider_required); render(); return }
        runTask(getString(R.string.hashing), 300_000L, {
            val cancellation = CancellationSignal(); readCancellation.set(cancellation)
            var name = "selected-file"
            var declaredSize = -1L
            contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE), null, null, null, cancellation)?.use {
                if (it.moveToFirst()) {
                    name = it.getString(0)?.take(200) ?: name
                    if (!it.isNull(1)) declaredSize = it.getLong(1)
                }
            }
            require(declaredSize <= ProbePolicy.MAX_FILE_BYTES) { "File exceeds 16 GiB" }
            val digest = MessageDigest.getInstance("SHA-256")
            var bytes = 0L
            val descriptor = contentResolver.openAssetFileDescriptor(uri, "r", cancellation) ?: error("File cannot be opened")
            descriptor.use {
                it.createInputStream().use { input ->
                    openResource.set(input)
                    val buffer = ByteArray(128 * 1024)
                    while (true) {
                        check(!Thread.currentThread().isInterrupted) { "Canceled" }
                        val count = input.read(buffer)
                        if (count < 0) break
                        if (count == 0) continue
                        bytes += count
                        require(bytes <= ProbePolicy.MAX_FILE_BYTES) { "File exceeds 16 GiB" }
                        digest.update(buffer, 0, count)
                    }
                    openResource.compareAndSet(input, null)
                }
            }
            if (declaredSize >= 0) require(bytes == declaredSize) { "File size changed or the document was incomplete" }
            SelectedFileMetadata(name, bytes, digest.digest().joinToString("") { "%02x".format(it) })
        }) { metadata ->
            selected = metadata; event("User-selected local file hashed; content not copied into report")
            status.text = getString(R.string.hash_complete); render()
        }
    }
    private fun buildReport(): String = ReportProtocol.build(consent.isChecked, Instant.now().toString(), requireNotNull(device),
        currentAccess(), volumes, probes, selected, audit.toList())
    private fun requestExport() {
        if (!consent.isChecked || device == null) return
        pendingReport = runCatching { buildReport() }.getOrElse { status.setText(R.string.report_not_ready); return }
        val intent = Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("application/json").putExtra(Intent.EXTRA_TITLE, "nexvary-device-metadata.json")
            .putExtra(Intent.EXTRA_LOCAL_ONLY, true)
        startActivityForResult(intent, EXPORT_REPORT)
    }
    @Deprecated("Uses the platform result API to keep this small agent free of an AndroidX Activity dependency")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode != RESULT_OK || data?.data == null || !consent.isChecked) { pendingReport = null; return }
        val uri = requireNotNull(data.data)
        if (requestCode == PICK_FILE) hashFile(uri)
        else if (requestCode == EXPORT_REPORT) {
            val requested = pendingReport; pendingReport = null
            if (requested == null || !localProvider(uri)) { status.setText(R.string.local_provider_required); return }
            // Re-evaluate current permission before exporting; a picker may have
            // remained open while the optional Shizuku grant was revoked.
            val report = runCatching { buildReport() }.getOrElse { status.setText(R.string.report_not_ready); return }
            runTask(getString(R.string.exporting), 10_000L, {
                contentResolver.openOutputStream(uri, "w")?.use { output ->
                    openResource.set(output)
                    output.write(report.toByteArray(Charsets.UTF_8)); output.flush()
                    openResource.compareAndSet(output, null)
                } ?: error("Cannot create the selected report")
            }) { event("Metadata report exported to user-selected local document"); status.setText(R.string.export_complete); render() }
        }
    }
    private fun <T> runTask(label: String, timeoutMs: Long, task: () -> T, deliver: (T) -> Unit) {
        if (!consent.isChecked || active != null) return
        val token = ++generation
        status.text = label; progress.isIndeterminate = true
        active = worker.submit {
            try {
                val result = task()
                handler.post { if (generation == token && consent.isChecked) deliver(result) }
            } catch (_: Exception) {
                handler.post {
                    if (generation == token) {
                        status.setText(R.string.operation_failed); event("Metadata operation failed or was denied")
                        if (inShellTask) { probes = emptyList(); releaseService() }
                    }
                }
            } finally {
                readCancellation.getAndSet(null)?.cancel()
                runCatching { openResource.getAndSet(null)?.close() }
                handler.post { if (generation == token) { active = null; render() } }
            }
        }
        handler.postDelayed({ if (generation == token && active != null) cancelActive(getString(R.string.timed_out)) }, timeoutMs)
        render()
    }
    private fun cancelActive(message: String) {
        generation++
        active?.cancel(true); active = null
        readCancellation.getAndSet(null)?.cancel()
        runCatching { openResource.getAndSet(null)?.close() }
        if (inShellTask) probes = emptyList()
        releaseService()
        if (::status.isInitialized) { status.text = message; render() }
    }
    private fun event(action: String) {
        audit += "${Instant.now()} ${action.take(190)}"
        while (audit.size > 100) audit.removeAt(0)
    }
    override fun onResume() { super.onResume(); if (::accessLabel.isInitialized) refreshAccess() }
    override fun onStop() {
        if (::status.isInitialized) cancelActive(getString(R.string.background_canceled))
        super.onStop()
    }
    override fun onDestroy() {
        cancelActive(getString(R.string.canceled))
        Shizuku.removeBinderReceivedListener(binderReceived)
        Shizuku.removeBinderDeadListener(binderDead)
        Shizuku.removeRequestPermissionResultListener(permissionResult)
        handler.removeCallbacksAndMessages(null)
        worker.shutdownNow()
        super.onDestroy()
    }
    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()
    companion object { private const val PERMISSION_REQUEST = 41; private const val PICK_FILE = 42; private const val EXPORT_REPORT = 43 }
}
