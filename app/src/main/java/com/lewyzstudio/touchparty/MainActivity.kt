package com.lewyzstudio.touchparty

import android.app.AlertDialog
import android.content.Context
import android.content.res.Configuration
import android.media.MediaPlayer
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.text.InputType
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.EditText
import android.widget.Toast
import com.google.androidgamesdk.GameActivity
import java.net.HttpURLConnection
import java.net.URL
import java.text.Normalizer
import java.util.Locale

class MainActivity : GameActivity() {
    companion object {
        @Volatile
        var instance: MainActivity? = null

        fun sanitizeToUppercase(input: String): String {
            if (input.isEmpty()) return ""
            val nfdNormalized = Normalizer.normalize(input, Normalizer.Form.NFD)
            val regex = "\\p{InCombiningDiacriticalMarks}+".toRegex()
            return regex.replace(nfdNormalized, "").uppercase(Locale.ROOT)
        }

        fun showToast(message: String) {
            instance?.runOnUiThread {
                Toast.makeText(instance, message, Toast.LENGTH_LONG).show()
            }
        }

        init {
            System.loadLibrary("touchparty")
        }

        @JvmStatic
        external fun nativeOnTextInputResult(fieldType: Int, text: String): Boolean

        @JvmStatic
        external fun nativeSetServerConnected(connected: Boolean)

        @JvmStatic
        external fun nativeSetServerRooms(jsonRooms: String): Boolean

        @JvmStatic
        external fun nativeOnRoomJoined(roomId: String, isOwner: Boolean, team: String): Boolean

        @JvmStatic
        external fun nativeOnRoomStateUpdated(roomId: String, roomName: String, playerCount: Int, isPrivate: Boolean, ownerId: String, isOwner: Boolean, state: String, playersJson: String, myTeam: String): Boolean

        @JvmStatic
        external fun nativeOnGameAborted(reason: String): Boolean

        @JvmStatic
        external fun nativeUpdateBoardCell(x: Int, y: Int, colorState: Int): Boolean

        @JvmStatic
        external fun nativeStartGameFromNetwork(): Boolean

        @JvmStatic
        external fun nativeSetLanguage(language: Int): Boolean

        @JvmStatic
        external fun nativeBeginFirstRunSetup(): Boolean

        @JvmStatic
        fun sendServerRoomsToNative(roomsJson: String) {
            Thread {
                var retries = 0
                while (retries < 30) {
                    try {
                        if (nativeSetServerRooms(roomsJson)) break
                    } catch (_: Exception) {}
                    try { Thread.sleep(50) } catch (_: InterruptedException) { break }
                    retries++
                }
            }.start()
        }

        @JvmStatic
        fun sendRoomJoinedToNative(roomId: String, isOwner: Boolean, team: String) {
            Thread {
                var retries = 0
                while (retries < 30) {
                    try {
                        if (nativeOnRoomJoined(roomId, isOwner, team)) break
                    } catch (_: Exception) {}
                    try { Thread.sleep(50) } catch (_: InterruptedException) { break }
                    retries++
                }
            }.start()
        }

        @JvmStatic
        fun sendRoomStateToNative(roomId: String, roomName: String, playerCount: Int, isPrivate: Boolean, ownerId: String, isOwner: Boolean, state: String, playersJson: String, myTeam: String) {
            Thread {
                var retries = 0
                while (retries < 30) {
                    try {
                        if (nativeOnRoomStateUpdated(roomId, roomName, playerCount, isPrivate, ownerId, isOwner, state, playersJson, myTeam)) break
                    } catch (_: Exception) {}
                    try { Thread.sleep(50) } catch (_: InterruptedException) { break }
                    retries++
                }
            }.start()
        }

        @JvmStatic
        fun sendGameAbortedToNative(reason: String) {
            Thread {
                var retries = 0
                while (retries < 30) {
                    try {
                        if (nativeOnGameAborted(reason)) break
                    } catch (_: Exception) {}
                    try { Thread.sleep(50) } catch (_: InterruptedException) { break }
                    retries++
                }
            }.start()
        }

        @JvmStatic
        fun sendUpdateBoardCellToNative(x: Int, y: Int, colorState: Int) {
            try {
                nativeUpdateBoardCell(x, y, colorState)
            } catch (_: Exception) {}
        }

        @JvmStatic
        fun sendStartGameToNative() {
            Thread {
                var retries = 0
                while (retries < 30) {
                    try {
                        if (nativeStartGameFromNetwork()) break
                    } catch (_: Exception) {}
                    try { Thread.sleep(50) } catch (_: InterruptedException) { break }
                    retries++
                }
            }.start()
        }
    }

    private var vibrator: Vibrator? = null
    private var mediaPlayer: MediaPlayer? = null
    private var healthCheckThread: Thread? = null
    @Volatile
    private var isCheckingHealth = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        instance = this

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            window.attributes.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        }

        vibrator = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            val vibratorManager = getSystemService(VIBRATOR_MANAGER_SERVICE) as VibratorManager
            vibratorManager.defaultVibrator
        } else {
            @Suppress("DEPRECATION")
            getSystemService(VIBRATOR_SERVICE) as Vibrator
        }

        pushLanguageToNative()
        if (getSavedNickname().isEmpty()) {
            // First launch: native UI shows the language/flag selector + NickName screen.
            pushFirstRunSetupToNative()
        } else {
            checkSavedNicknameOnLaunch()
            Handler(Looper.getMainLooper()).postDelayed({
                checkSavedNicknameOnLaunch()
            }, 1000)
        }
        startServerHealthCheck()
        GameWebSocketManager.init()
    }

    fun getAppDeviceId(): String {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        var devId = prefs.getString("device_id", null)
        if (devId.isNullOrEmpty()) {
            devId = "dev-" + java.util.UUID.randomUUID().toString().take(8)
            prefs.edit().putString("device_id", devId).apply()
        }
        return devId
    }

    fun requestCreateRoom(roomName: String, isPrivate: Boolean, pin: String) {
        val nick = getSavedNickname()
        if (nick.isNotEmpty()) sendSavedNicknameToNative(nick)
        val devId = getAppDeviceId()
        GameWebSocketManager.createRoom(roomName, isPrivate, pin, nick, devId)
    }

    fun requestListRooms() {
        GameWebSocketManager.listRooms()
    }

    fun requestJoinRoom(roomId: String, pin: String) {
        val nick = getSavedNickname()
        if (nick.isNotEmpty()) sendSavedNicknameToNative(nick)
        val devId = getAppDeviceId()
        GameWebSocketManager.joinRoom(roomId, pin, nick, devId)
    }

    fun requestLeaveRoom() {
        GameWebSocketManager.leaveRoom()
        GameWebSocketManager.listRooms()
    }

    fun requestReturnToRoom() {
        GameWebSocketManager.returnToRoom()
    }

    fun requestSetTeam(team: String) {
        GameWebSocketManager.setTeam(team)
    }

    fun requestStartGame() {
        GameWebSocketManager.sendStartGame()
    }

    fun requestSendTap(x: Int, y: Int) {
        GameWebSocketManager.sendTap(x, y)
    }

    private fun checkSavedNicknameOnLaunch() {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        val savedNick = prefs.getString("user_nickname", null)
        if (!savedNick.isNullOrEmpty()) {
            sendSavedNicknameToNative(savedNick)
            val uiCtx = localizedContext()
            val welcome = uiCtx.getString(R.string.toast_welcome_back, savedNick)
            Toast.makeText(this, welcome, Toast.LENGTH_SHORT).show()
        }
    }

    fun getSavedLanguageCode(): String {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        return prefs.getString("app_language", null) ?: detectSystemLanguage()
    }

    private fun detectSystemLanguage(): String {
        val lang = java.util.Locale.getDefault().language.lowercase()
        return if (lang == "es") "es" else "en"
    }

    fun languageCodeToNative(langCode: String): Int {
        return if (langCode == "es") 0 else 1
    }

    fun saveLanguagePref(langCode: String) {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        prefs.edit().putString("app_language", langCode).apply()
    }

    fun pushLanguageToNative() {
        val langCode = getSavedLanguageCode()
        val nativeLang = languageCodeToNative(langCode)
        Thread {
            var retries = 0
            while (retries < 60) {
                try {
                    val sent = nativeSetLanguage(nativeLang)
                    if (sent) break
                } catch (_: Exception) {}
                try {
                    Thread.sleep(100)
                } catch (_: InterruptedException) {
                    break
                }
                retries++
            }
        }.start()
    }

    fun pushFirstRunSetupToNative() {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        val savedNick = prefs.getString("user_nickname", null)
        if (!savedNick.isNullOrEmpty()) return
        Thread {
            var retries = 0
            while (retries < 60) {
                try {
                    val sent = nativeBeginFirstRunSetup()
                    if (sent) break
                } catch (_: Exception) {}
                try {
                    Thread.sleep(100)
                } catch (_: InterruptedException) {
                    break
                }
                retries++
            }
        }.start()
    }

    //only send nickname
    private fun sendSavedNicknameToNative(nick: String) {
        Thread {
            var retries = 0
            while (retries < 60) {
                try {
                    val sent = nativeOnTextInputResult(0, nick)
                    if (sent) {
                        break
                    }
                } catch (_: Exception) {}
                try {
                    Thread.sleep(100)
                } catch (_: InterruptedException) {
                    break
                }
                retries++
            }
        }.start()
    }

    fun getSavedNickname(): String {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        val raw = prefs.getString("user_nickname", "") ?: ""
        return sanitizeToUppercase(raw)
    }

    fun saveNickname(nick: String) {
        val prefs = getSharedPreferences("touchparty_prefs", MODE_PRIVATE)
        val sanitized = sanitizeToUppercase(nick)
        prefs.edit().putString("user_nickname", sanitized).apply()
    }

    fun localizedContext(): Context {
        val langCode = getSavedLanguageCode()
        val config = Configuration(resources.configuration)
        config.setLocale(
            if (langCode == "es") java.util.Locale.forLanguageTag("es") else java.util.Locale.forLanguageTag("en")
        )
        return createConfigurationContext(config)
    }

    fun showTextInputDialog(fieldType: Int, title: String, currentText: String) {
        runOnUiThread {
            // Guard against showing a dialog when the activity has no valid
            // window token (prevents BadTokenException at builder.show()).
            if (isFinishing || isDestroyed) return@runOnUiThread

            // Build the dialog with THIS activity context so it has a valid
            // window token; only the button strings come from the localized
            // context (createConfigurationContext has no window token).
            val uiCtx = localizedContext()
            val builder = AlertDialog.Builder(this)
            builder.setTitle(title)

            val input = EditText(this)
            input.setText(currentText)
            if (fieldType == 2 || fieldType == 3) {
                input.inputType = InputType.TYPE_CLASS_NUMBER
            } else {
                input.inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS
            }
            input.setSelection(input.text.length)
            builder.setView(input)

            val acceptLabel = uiCtx.getString(R.string.btn_accept)
            val cancelLabel = uiCtx.getString(R.string.btn_cancel)

            builder.setPositiveButton(acceptLabel) { _, _ ->
                val enteredText = input.text.toString().trim()
                if (enteredText.isNotEmpty()) {
                    val sanitized = sanitizeToUppercase(enteredText)
                    if (fieldType == 0) {
                        saveNickname(sanitized)
                        sendSavedNicknameToNative(sanitized)
                    } else {
                        nativeOnTextInputResult(fieldType, sanitized)
                    }
                }
            }
            builder.setNegativeButton(cancelLabel) { dialog, _ ->
                dialog.cancel()
            }
            builder.show()
        }
    }

    fun playCountdownAudio() {
        runOnUiThread {
            try {
                mediaPlayer?.release()
                mediaPlayer = MediaPlayer.create(this, R.raw.countdown_party)
                mediaPlayer?.start()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    fun triggerVibration(colorState: Int) {
        val vib = vibrator ?: return
        if (!vib.hasVibrator()) return

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val effect = when (colorState) {
                0 -> VibrationEffect.createOneShot(15L, VibrationEffect.DEFAULT_AMPLITUDE)
                1 -> VibrationEffect.createWaveform(longArrayOf(0, 20, 20, 25), -1)
                2 -> VibrationEffect.createWaveform(longArrayOf(0, 45, 25, 45), -1)
                3 -> VibrationEffect.createWaveform(longArrayOf(0, 15, 15, 15, 15, 20), -1)
                4 -> VibrationEffect.createWaveform(longArrayOf(0, 75), -1)
                else -> VibrationEffect.createOneShot(20L, VibrationEffect.DEFAULT_AMPLITUDE)
            }
            vib.vibrate(effect)
        } else {
            @Suppress("DEPRECATION")
            val ms = when (colorState) {
                0 -> 15L
                1 -> 35L
                2 -> 60L
                3 -> 40L
                4 -> 75L
                else -> 20L
            }
            @Suppress("DEPRECATION")
            vib.vibrate(ms)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    private fun startServerHealthCheck() {
        isCheckingHealth = true
        healthCheckThread = Thread {
            val rawUrl = BuildConfig.GAME_SERVER_HTTP_URL
            val healthUrlStr = if (rawUrl.endsWith("/")) "${rawUrl}health" else "$rawUrl/health"

            while (isCheckingHealth && !isFinishing) {
                var connected = false
                try {
                    val url = URL(healthUrlStr)
                    val conn = url.openConnection() as HttpURLConnection
                    conn.requestMethod = "GET"
                    conn.connectTimeout = 3000
                    conn.readTimeout = 3000
                    val responseCode = conn.responseCode
                    if (responseCode == 200) {
                        connected = true
                    }
                    conn.disconnect()
                } catch (_: Exception) {
                    connected = false
                }

                try {
                    nativeSetServerConnected(connected)
                } catch (e: Exception) {
                    e.printStackTrace()
                }

                try {
                    Thread.sleep(5000)
                } catch (_: InterruptedException) {
                    break
                }
            }
        }.apply {
            isDaemon = true
            start()
        }
    }

    override fun onDestroy() {
        if (instance == this) instance = null
        GameWebSocketManager.leaveRoom()
        GameWebSocketManager.disconnect()
        isCheckingHealth = false
        healthCheckThread?.interrupt()
        healthCheckThread = null
        try {
            mediaPlayer?.release()
            mediaPlayer = null
        } catch (e: Exception) {
            e.printStackTrace()
        }
        super.onDestroy()
    }

    private fun hideSystemUi() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false)
            window.insetsController?.apply {
                hide(WindowInsets.Type.systemBars())
                systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN
            )
        }
    }
}