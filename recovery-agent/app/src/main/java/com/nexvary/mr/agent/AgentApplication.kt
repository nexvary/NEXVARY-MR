package com.nexvary.mr.agent

import android.app.Application
import android.content.Context
import rikka.shizuku.ShizukuProvider

class AgentApplication : Application() {
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base)
        // This agent deliberately supports shell mode only; never initialize Sui/root.
        ShizukuProvider.disableAutomaticSuiInitialization()
    }
}
