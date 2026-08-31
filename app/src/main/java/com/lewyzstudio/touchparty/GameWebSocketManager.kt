package com.lewyzstudio.touchparty

import android.util.Log
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import okhttp3.WebSocket
import okhttp3.WebSocketListener
import org.json.JSONArray
import org.json.JSONObject
import java.util.concurrent.TimeUnit

object GameWebSocketManager {
    private const val TAG = "GameWebSocketManager"
    private var webSocket: WebSocket? = null
    private var client: OkHttpClient? = null
    private var isConnected = false
    private var currentRoomId: String? = null

    fun init() {
        if (client == null) {
            client = OkHttpClient.Builder()
                .connectTimeout(5, TimeUnit.SECONDS)
                .readTimeout(5, TimeUnit.SECONDS)
                .writeTimeout(5, TimeUnit.SECONDS)
                .build()
        }
        connect()
    }

    @Synchronized
    fun connect() {
        if (isConnected && webSocket != null) return

        val wsUrl = BuildConfig.GAME_SERVER_WS_URL
        Log.d(TAG, "Connecting WebSocket to: $wsUrl")

        val request = Request.Builder()
            .url(wsUrl)
            .build()

        webSocket = client?.newWebSocket(request, object : WebSocketListener() {
            override fun onOpen(ws: WebSocket, response: Response) {
                Log.d(TAG, "WebSocket connected successfully!")
                isConnected = true
                // Auto-fetch rooms on initial connection
                listRooms()
            }

            override fun onMessage(ws: WebSocket, text: String) {
                Log.d(TAG, "WS Message Received: $text")
                handleMessage(text)
            }

            override fun onClosing(ws: WebSocket, code: Int, reason: String) {
                Log.d(TAG, "WS Closing: $code / $reason")
                isConnected = false
            }

            override fun onClosed(ws: WebSocket, code: Int, reason: String) {
                Log.d(TAG, "WS Closed: $code / $reason")
                isConnected = false
                webSocket = null
            }

            override fun onFailure(ws: WebSocket, t: Throwable, response: Response?) {
                Log.e(TAG, "WS Failure: ${t.message}", t)
                isConnected = false
                webSocket = null
            }
        })
    }

    private fun handleMessage(text: String) {
        try {
            val json = JSONObject(text)
            when (json.optString("type")) {
                "rooms_list" -> {
                    val roomsArray = json.optJSONArray("rooms") ?: JSONArray()
                    MainActivity.sendServerRoomsToNative(roomsArray.toString())
                }
                "joined" -> {
                    val roomId = json.optString("roomId", "")
                    val isOwner = json.optBoolean("isOwner", false)
                    currentRoomId = roomId
                    MainActivity.sendRoomJoinedToNative(roomId, isOwner)
                }
                "room_state" -> {
                    val roomObj = json.optJSONObject("room")
                    if (roomObj != null) {
                        val roomId = roomObj.optString("roomId", "")
                        val roomName = roomObj.optString("name", "")
                        val playerCount = roomObj.optInt("playerCount", 1)
                        val isPrivate = roomObj.optBoolean("isPrivate", false)
                        val ownerId = roomObj.optString("ownerId", "")
                        val state = roomObj.optString("state", "LOBBY")
                        MainActivity.sendRoomStateToNative(roomId, roomName, playerCount, isPrivate, ownerId, state)
                    }
                }
                "error" -> {
                    val errorMsg = json.optString("error", "Unknown server error")
                    Log.w(TAG, "Server reported error: $errorMsg")
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error parsing server WS message: ${e.message}", e)
        }
    }

    private fun sendJson(json: JSONObject) {
        if (!isConnected || webSocket == null) {
            Log.w(TAG, "WS not connected, reconnecting before send...")
            connect()
        }
        val sent = webSocket?.send(json.toString()) ?: false
        Log.d(TAG, "Sent JSON (success=$sent): ${json.toString()}")
    }

    fun listRooms() {
        val json = JSONObject()
        json.put("type", "list_rooms")
        sendJson(json)
    }

    fun createRoom(roomName: String, isPrivate: Boolean, pin: String, playerName: String) {
        val json = JSONObject()
        json.put("type", "join")
        json.put("roomName", roomName)
        json.put("isPrivate", isPrivate)
        json.put("pin", pin)
        json.put("playerName", playerName)
        json.put("playerColor", "#0088ff")
        sendJson(json)
    }

    fun joinRoom(roomId: String, pin: String, playerName: String) {
        val json = JSONObject()
        json.put("type", "join")
        json.put("roomId", roomId)
        json.put("pin", pin)
        json.put("playerName", playerName)
        json.put("playerColor", "#ff2244")
        sendJson(json)
    }

    fun leaveRoom() {
        val json = JSONObject()
        json.put("type", "leave")
        sendJson(json)
        currentRoomId = null
    }

    fun sendStartGame() {
        val json = JSONObject()
        json.put("type", "start")
        sendJson(json)
    }

    fun disconnect() {
        webSocket?.close(1000, "App closing")
        webSocket = null
        isConnected = false
    }
}
