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
    private var myPlayerId: String? = null
    private var isRoomOwner = false
    private var myTeam = "BLUE"
    private var returnedToRoom = false

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
                val activeRoom = currentRoomId
                val nick = MainActivity.instance?.getSavedNickname() ?: ""
                val devId = MainActivity.instance?.getAppDeviceId() ?: ""
                if (!activeRoom.isNullOrEmpty() && nick.isNotEmpty()) {
                    Log.d(TAG, "Auto-rejoining room '$activeRoom' after WS reconnect")
                    joinRoom(activeRoom, "", nick, devId)
                } else {
                    // Auto-fetch rooms on initial connection
                    listRooms()
                }
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
                scheduleReconnect()
            }

            override fun onFailure(ws: WebSocket, t: Throwable, response: Response?) {
                Log.w(TAG, "WS Disconnected: ${t.message ?: "Connection reset"}")
                isConnected = false
                webSocket = null
                scheduleReconnect()
            }
        })
    }

    private var isReconnecting = false
    private val mainHandler = android.os.Handler(android.os.Looper.getMainLooper())

    private fun scheduleReconnect() {
        if (isReconnecting) return
        isReconnecting = true
        mainHandler.postDelayed({
            isReconnecting = false
            connect()
        }, 3000)
    }

    private fun playersJsonWithLocalMarker(playersArray: JSONArray?): String {
        if (playersArray == null) return "[]"
        for (i in 0 until playersArray.length()) {
            val playerObj = playersArray.optJSONObject(i) ?: continue
            playerObj.put("isLocal", playerObj.optString("id") == myPlayerId)
        }
        return playersArray.toString()
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
                    val playerId = json.optString("playerId", "")
                    val team = json.optString("team", "BLUE")
                    val wasRejoiningSameRoom = currentRoomId == roomId && myPlayerId != null
                    if (playerId.isNotEmpty()) myPlayerId = playerId
                    currentRoomId = roomId
                    isRoomOwner = isOwner
                    myTeam = team
                    if (!wasRejoiningSameRoom) returnedToRoom = false
                    MainActivity.sendRoomJoinedToNative(roomId, isOwner, myTeam)
                }
                "game_started" -> {
                    // A player who already returned to the room does not
                    // re-enter a later match started by another player.
                    if (!returnedToRoom) {
                        MainActivity.sendStartGameToNative()
                    }
                }
                "game_aborted" -> {
                    val reason = json.optString("reason", "La partida finalizó porque quedaron menos de 2 jugadores.")
                    MainActivity.sendGameAbortedToNative(reason)
                }
                "game_over" -> {
                    val roomObj = json.optJSONObject("room")
                    val roomId = json.optString("roomId", currentRoomId ?: "")
                    if (roomObj != null) {
                        val roomName = roomObj.optString("name", "")
                        val playerCount = roomObj.optInt("playerCount", 1)
                        val isPrivate = roomObj.optBoolean("isPrivate", false)
                        val ownerId = roomObj.optString("ownerId", "")
                        val state = "FINISHED"

                        val isOwner = if (!myPlayerId.isNullOrEmpty() && ownerId.isNotEmpty()) (myPlayerId == ownerId) else isRoomOwner

                        val playersArray = roomObj.optJSONArray("players")
                        val playersJson = playersJsonWithLocalMarker(playersArray)

                        MainActivity.sendRoomStateToNative(roomId, roomName, playerCount, isPrivate, ownerId, isOwner, state, playersJson, myTeam)
                    } else {
                        MainActivity.sendRoomStateToNative(roomId, "", 1, false, "", isRoomOwner, "FINISHED", "[]", myTeam)
                    }
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

                        val playersArray = roomObj.optJSONArray("players")
                        if (playersArray != null && !myPlayerId.isNullOrEmpty()) {
                            for (i in 0 until playersArray.length()) {
                                val playerObj = playersArray.optJSONObject(i) ?: continue
                                if (playerObj.optString("id") == myPlayerId) {
                                    myTeam = playerObj.optString("team", myTeam)
                                    break
                                }
                            }
                        }

                        // The server is authoritative. This also updates the
                        // next player immediately after an owner leaves.
                        isRoomOwner = !myPlayerId.isNullOrEmpty() && ownerId.isNotEmpty() && myPlayerId == ownerId
                        val isOwner = isRoomOwner

                        val playersJson = playersJsonWithLocalMarker(playersArray)

                        MainActivity.sendRoomStateToNative(roomId, roomName, playerCount, isPrivate, ownerId, isOwner, state, playersJson, myTeam)

                        val boardArray = roomObj.optJSONArray("board")
                        if (boardArray != null) {
                            for (col in 0 until boardArray.length()) {
                                val colArray = boardArray.optJSONArray(col) ?: continue
                                for (row in 0 until colArray.length()) {
                                    val cellObj = colArray.optJSONObject(row) ?: continue
                                    val ownerIdCell = cellObj.optString("ownerId", "")
                                    val colorHex = cellObj.optString("color", "#cccccc").lowercase()
                                    var colorState = 0
                                    if (ownerIdCell.isNotEmpty() && ownerIdCell != "null") {
                                        colorState = if (colorHex.contains("ff2244") || colorHex.contains("ff0000") || colorHex.contains("red")) 2 else 1
                                    }
                                    MainActivity.sendUpdateBoardCellToNative(col, row, colorState)
                                }
                            }
                        }
                    }
                }
                "error" -> {
                    val errorMsg = json.optString("error", "Unknown server error")
                    Log.w(TAG, "Server reported error: $errorMsg")
                    if (!errorMsg.contains("Game is not active", ignoreCase = true)) {
                        MainActivity.showToast(errorMsg)
                    }
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

    fun createRoom(roomName: String, isPrivate: Boolean, pin: String, playerName: String, deviceId: String = "") {
        val json = JSONObject()
        json.put("type", "join")
        json.put("roomName", roomName)
        json.put("isPrivate", isPrivate)
        json.put("pin", pin)
        json.put("playerName", playerName)
        json.put("playerColor", "#0088ff")
        if (deviceId.isNotEmpty()) json.put("deviceId", deviceId)
        sendJson(json)
    }

    fun joinRoom(roomId: String, pin: String, playerName: String, deviceId: String = "") {
        val json = JSONObject()
        json.put("type", "join")
        json.put("roomId", roomId)
        json.put("pin", pin)
        json.put("playerName", playerName)
        json.put("playerColor", "#ff2244")
        if (deviceId.isNotEmpty()) json.put("deviceId", deviceId)
        sendJson(json)
    }

    fun leaveRoom() {
        val json = JSONObject()
        json.put("type", "leave")
        sendJson(json)
        currentRoomId = null
        myPlayerId = null
        isRoomOwner = false
        myTeam = "BLUE"
        returnedToRoom = false
    }

    fun returnToRoom() {
        returnedToRoom = true
        val json = JSONObject()
        json.put("type", "return_to_room")
        sendJson(json)
    }

    fun setTeam(team: String) {
        val normalizedTeam = if (team.uppercase() == "RED") "RED" else "BLUE"
        myTeam = normalizedTeam
        val json = JSONObject()
        json.put("type", "set_team")
        json.put("team", normalizedTeam)
        sendJson(json)
    }

    fun sendStartGame() {
        returnedToRoom = false
        val json = JSONObject()
        json.put("type", "start")
        sendJson(json)
    }

    fun sendTap(x: Int, y: Int) {
        val json = JSONObject()
        json.put("type", "tap")
        json.put("x", x)
        json.put("y", y)
        sendJson(json)
    }

    fun disconnect() {
        webSocket?.close(1000, "App closing")
        webSocket = null
        isConnected = false
    }
}
