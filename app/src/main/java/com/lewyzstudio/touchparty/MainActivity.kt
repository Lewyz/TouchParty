package com.lewyzstudio.touchparty

import android.app.AlertDialog
import android.content.Context
import android.media.MediaPlayer
import android.os.Build
import android.os.Bundle
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import android.text.InputType
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.EditText
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("touchparty")
        }

        @JvmStatic
        external fun nativeOnTextInputResult(fieldType: Int, text: String)
    }

    private var vibrator: Vibrator? = null
    private var mediaPlayer: MediaPlayer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

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

        checkAndPromptInitialNickname()
    }

    private fun checkAndPromptInitialNickname() {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        val savedNick = prefs.getString("user_nickname", null)
        if (savedNick.isNullOrEmpty()) {
            showTextInputDialog(0, "BIENVENIDO A TOUCHPARTY", "JUGADOR_1")
        }
    }

    fun getSavedNickname(): String {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        return prefs.getString("user_nickname", "JUGADOR_1") ?: "JUGADOR_1"
    }

    fun saveNickname(nick: String) {
        val prefs = getSharedPreferences("touchparty_prefs", Context.MODE_PRIVATE)
        prefs.edit().putString("user_nickname", nick).apply()
    }

    fun showTextInputDialog(fieldType: Int, title: String, currentText: String) {
        runOnUiThread {
            val builder = AlertDialog.Builder(this)
            builder.setTitle(title)

            val input = EditText(this)
            input.setText(currentText)
            if (fieldType == 2) {
                input.inputType = InputType.TYPE_CLASS_NUMBER
            } else {
                input.inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS
            }
            input.setSelection(input.text.length)
            builder.setView(input)

            builder.setPositiveButton("ACEPTAR") { _, _ ->
                val enteredText = input.text.toString().trim()
                if (enteredText.isNotEmpty()) {
                    if (fieldType == 0) {
                        saveNickname(enteredText)
                    }
                    nativeOnTextInputResult(fieldType, enteredText)
                }
            }
            builder.setNegativeButton("CANCELAR") { dialog, _ ->
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

    override fun onDestroy() {
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