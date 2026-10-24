package com.nexvary.npse

import android.app.Activity
import android.app.AlertDialog
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.drawable.GradientDrawable
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.View
import android.widget.*
import com.nexvary.npse.core.AccessPolicy
import org.json.JSONObject
import rikka.shizuku.Shizuku
import java.time.Instant
import java.util.concurrent.Executors
import java.util.concurrent.Future

/** Foreground-only control center. No networking, root, settings writes or exported IPC. */
class ControlCenterActivity : Activity() {
    private val handler = Handler(Looper.getMainLooper())
    private val worker = Executors.newSingleThreadExecutor()
    private var task: Future<*>? = null
    private var epoch = 0
    private var optedIn = false
    private var denied = false
    private var binding = false
    private var helper: IDiagnosticsService? = null
    private var args: Shizuku.UserServiceArgs? = null
    private var standard: JSONObject? = null
    private var privileged: JSONObject? = null
    private lateinit var status: TextView
    private lateinit var preview: TextView
    private lateinit var auditLabel: TextView
    private lateinit var grant: Button
    private lateinit var advanced: Button
    private lateinit var cancel: Button
    private lateinit var export: Button
    private val audit = ArrayDeque<String>()
    private val standardProvider by lazy { StandardProvider(this) }
    private val received = Shizuku.OnBinderReceivedListener { refresh() }
    private val dead = Shizuku.OnBinderDeadListener { invalidate(); refresh() }
    private val permission = Shizuku.OnRequestPermissionResultListener { code, result ->
        if (code == REQUEST) { denied = result != PackageManager.PERMISSION_GRANTED; invalidate(); event("permission-result"); refresh() }
    }
    private val poll = object : Runnable {
        override fun run() { refresh(); handler.postDelayed(this, 1000) }
    }
    private val timeout = Runnable { invalidate(); event("timeout"); refresh() }
    private val connection = object : ServiceConnection {
        override fun onServiceConnected(name: ComponentName?, binder: IBinder?) {
            if (!binding || !optedIn || ShizukuAccess.state() != AccessPolicy.State.SHELL || binder == null) { invalidate(); refresh(); return }
            helper = IDiagnosticsService.Stub.asInterface(binder)
            val remote = requireNotNull(helper)
            val generation = epoch
            task = worker.submit {
                val result = runCatching { DiagnosticsProtocol.validate(remote.collectDiagnostics()) }.getOrNull()
                handler.post {
                    if (generation == epoch && optedIn && ShizukuAccess.state() == AccessPolicy.State.SHELL) {
                        privileged = result
                        event(if (result == null) "diagnostics-failed" else "diagnostics-completed")
                        release(); refresh()
                    }
                }
            }
        }
        override fun onServiceDisconnected(name: ComponentName?) { invalidate(); event("helper-disconnected"); refresh() }
        override fun onBindingDied(name: ComponentName?) { invalidate(); refresh() }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        buildUi()
        Shizuku.addBinderReceivedListenerSticky(received)
        Shizuku.addBinderDeadListener(dead)
        Shizuku.addRequestPermissionResultListener(permission)
        event("control-center-opened")
        refresh()
    }
    private fun buildUi() {
        val content = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL; setPadding(dp(18), dp(12), dp(18), dp(18))
            setBackgroundColor(Color.rgb(10, 10, 10))
            layoutDirection = View.LAYOUT_DIRECTION_LOCALE
        }
        val scroll = ScrollView(this).apply { isFillViewport = true; addView(content) }
        scroll.setOnApplyWindowInsetsListener { view, insets ->
            @Suppress("DEPRECATION")
            view.setPadding(insets.systemWindowInsetLeft, insets.systemWindowInsetTop, insets.systemWindowInsetRight, insets.systemWindowInsetBottom)
            insets
        }
        setContentView(scroll)
        fun label(text: String, size: Float = 15f) = TextView(this).apply {
            this.text = text; textSize = size; setTextColor(Color.rgb(205, 210, 218))
            setPadding(0, dp(10), 0, dp(10)); content.addView(this)
        }
        fun button(resource: Int, icon: Int, action: () -> Unit) = Button(this).apply {
            setText(resource); isAllCaps = false; setTextColor(Color.WHITE)
            backgroundTintList = android.content.res.ColorStateList.valueOf(Color.rgb(22, 55, 90))
            setCompoundDrawablesRelativeWithIntrinsicBounds(icon, 0, 0, 0)
            compoundDrawableTintList = android.content.res.ColorStateList.valueOf(Color.rgb(64, 160, 255))
            content.addView(this, LinearLayout.LayoutParams(-1, -2).apply { bottomMargin = dp(8) })
            setOnClickListener { action() }
        }
        button(R.string.npse_back, android.R.drawable.ic_media_previous) { finish() }
        label(getString(R.string.npse_title), 25f)
        label(getString(R.string.npse_scope))
        status = label("").apply {
            background = GradientDrawable().apply { setColor(Color.rgb(22, 25, 32)); cornerRadius = dp(12).toFloat(); setStroke(dp(1), Color.rgb(52, 137, 255)) }
            setPadding(dp(12), dp(12), dp(12), dp(12)); accessibilityLiveRegion = View.ACCESSIBILITY_LIVE_REGION_POLITE
        }
        button(R.string.npse_standard, android.R.drawable.ic_menu_info_details) {
            standard = runCatching { standardProvider.diagnostics().put("application", standardProvider.ownAppInfo()) }.getOrNull()
            event(if (standard == null) "standard-failed" else "standard-completed"); refresh()
        }
        button(R.string.npse_apps, android.R.drawable.ic_menu_manage) { showApps() }
        val toggle = Switch(this).apply {
            setText(R.string.npse_opt_in); setTextColor(Color.WHITE)
            setOnCheckedChangeListener { _, checked -> optedIn = checked; denied = false; invalidate(); event(if (checked) "optional-enabled" else "optional-disabled"); refresh() }
        }
        content.addView(toggle)
        button(R.string.npse_setup, android.R.drawable.ic_menu_help) { setup() }
        grant = button(R.string.npse_grant, android.R.drawable.ic_lock_lock) {
            event("permission-requested")
            if (!ShizukuAccess.request(REQUEST)) message(R.string.npse_permission_help)
            refresh()
        }
        advanced = button(R.string.npse_advanced, android.R.drawable.ic_menu_search) {
            AlertDialog.Builder(this).setTitle(R.string.npse_advanced).setMessage(R.string.npse_confirm)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.npse_continue) { _, _ -> collectShell() }.show()
        }
        cancel = button(R.string.npse_cancel, android.R.drawable.ic_menu_close_clear_cancel) { invalidate(); event("canceled"); refresh() }
        button(R.string.npse_refresh, android.R.drawable.ic_popup_sync) { refresh() }
        export = button(R.string.npse_export, android.R.drawable.ic_menu_save) {
            AlertDialog.Builder(this).setMessage(R.string.npse_export_consent).setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.npse_continue) { _, _ ->
                    startActivityForResult(Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
                        .setType("application/json").putExtra(Intent.EXTRA_TITLE, "nexvary-npse.json").putExtra(Intent.EXTRA_LOCAL_ONLY, true), EXPORT)
                }.show()
        }
        preview = label("").apply { textSize = 12f; setTextIsSelectable(true) }
        label(getString(R.string.npse_audit), 20f)
        auditLabel = label("").apply { textSize = 12f }
        label(getString(R.string.npse_privacy))
    }
    private fun showApps() {
        val packages = runCatching { standardProvider.visiblePackages() }.getOrElse { message(R.string.npse_failed); return }
        AlertDialog.Builder(this).setTitle(R.string.npse_apps).setItems(packages.toTypedArray()) { _, i ->
            runCatching { startActivity(standardProvider.settings(packages[i])) }.onFailure { message(R.string.npse_failed) }
        }.setNegativeButton(android.R.string.cancel, null).show()
    }
    private fun setup() {
        val steps = intArrayOf(R.string.npse_step1, R.string.npse_step2, R.string.npse_step3, R.string.npse_step4)
        fun step(index: Int) {
            AlertDialog.Builder(this).setTitle(getString(R.string.npse_step, index + 1, steps.size)).setMessage(steps[index])
                .setNegativeButton(R.string.npse_back) { _, _ -> if (index > 0) step(index - 1) }
                .setPositiveButton(if (index == steps.lastIndex) android.R.string.ok else R.string.npse_next) { _, _ -> if (index < steps.lastIndex) step(index + 1) }.show()
        }
        step(0)
    }
    private fun collectShell() {
        if (!optedIn || binding || ShizukuAccess.state() != AccessPolicy.State.SHELL) { refresh(); return }
        privileged = null; epoch++
        args = Shizuku.UserServiceArgs(ComponentName(this, DiagnosticsService::class.java))
            .tag("npse-diagnostics-v1").version(1).daemon(false).debuggable(false).processNameSuffix("npse")
        binding = true
        try {
            Shizuku.bindUserService(requireNotNull(args), connection)
            handler.postDelayed(timeout, 12_000)
            event("fixed-diagnostics-consented")
        } catch (_: RuntimeException) { invalidate(); event("bind-failed") }
        refresh()
    }
    private fun release() {
        handler.removeCallbacks(timeout)
        task = null; helper = null
        val previous = args
        args = null; binding = false
        if (previous != null) runCatching { Shizuku.unbindUserService(previous, connection, true) }
    }
    private fun invalidate() {
        epoch++; privileged = null; task?.cancel(true)
        // Killing this dedicated non-daemon UserService also terminates outstanding children.
        release()
    }
    private fun report(): JSONObject? = standard?.let {
        JSONObject(it.toString()).put("generatedAtUtc", Instant.now().toString())
            .put("shizukuState", if (optedIn) ShizukuAccess.state(denied).name else "DISABLED")
            .put("privileged", if (optedIn && ShizukuAccess.state() == AccessPolicy.State.SHELL) privileged ?: JSONObject.NULL else JSONObject.NULL)
    }
    private fun refresh() {
        if (!::status.isInitialized) return
        val state = if (optedIn) ShizukuAccess.state(denied) else null
        if (state != AccessPolicy.State.SHELL && (privileged != null || binding)) invalidate()
        val resource = when(state) {
            null -> R.string.npse_state_disabled
            AccessPolicy.State.SHELL -> R.string.npse_state_shell
            AccessPolicy.State.PERMISSION_REQUIRED -> R.string.npse_state_permission
            AccessPolicy.State.DENIED -> R.string.npse_state_denied
            AccessPolicy.State.UNSUPPORTED_ROOT -> R.string.npse_state_root
            else -> R.string.npse_state_unavailable
        }
        status.setText(resource)
        status.setTextColor(if (state == null || state == AccessPolicy.State.SHELL) Color.rgb(50, 245, 140) else Color.rgb(255, 90, 100))
        grant.isEnabled = optedIn && state == AccessPolicy.State.PERMISSION_REQUIRED && !binding
        advanced.isEnabled = optedIn && state == AccessPolicy.State.SHELL && !binding
        cancel.isEnabled = binding
        export.isEnabled = standard != null && !binding
        preview.text = report()?.toString(2) ?: getString(R.string.npse_no_report)
        auditLabel.text = audit.joinToString("\n")
    }
    private fun event(code: String) {
        audit.addLast("${Instant.now()} $code")
        while (audit.size > 40) audit.removeFirst()
        // Only fixed event codes; never raw output, package names or identifiers.
    }
    @Deprecated("Platform document picker")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != EXPORT || resultCode != RESULT_OK) return
        val uri = data?.data ?: return
        if (!localDocument(uri)) { message(R.string.npse_local_only); return }
        // onStop invalidates shell results; export only fresh currently authorized data.
        val text = report()?.toString(2) ?: return
        val generation = epoch
        task = worker.submit {
            val success = runCatching { contentResolver.openOutputStream(uri, "w")?.use { it.write(text.toByteArray(Charsets.UTF_8)) } ?: error("unavailable") }.isSuccess
            handler.post { if (epoch == generation) { event(if (success) "export-completed" else "export-failed"); task = null; message(if (success) R.string.npse_exported else R.string.npse_failed); refresh() } }
        }
    }
    private fun localDocument(uri: Uri) = uri.scheme == "content" && uri.authority in setOf("com.android.externalstorage.documents", "com.android.providers.downloads.documents", "com.android.providers.media.documents")
    private fun message(id: Int) { Toast.makeText(this, id, Toast.LENGTH_LONG).show() }
    override fun onResume() { super.onResume(); refresh(); handler.post(poll) }
    override fun onStop() { handler.removeCallbacks(poll); invalidate(); super.onStop() }
    override fun onDestroy() {
        invalidate(); worker.shutdownNow(); handler.removeCallbacksAndMessages(null)
        Shizuku.removeBinderReceivedListener(received); Shizuku.removeBinderDeadListener(dead); Shizuku.removeRequestPermissionResultListener(permission)
        super.onDestroy()
    }
    private fun dp(value: Int) = (value * resources.displayMetrics.density).toInt()
    companion object { private const val REQUEST = 710; private const val EXPORT = 711 }
}
