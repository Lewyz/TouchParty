#ifndef ANDROIDGLINVESTIGATIONS_GAMEUI_H
#define ANDROIDGLINVESTIGATIONS_GAMEUI_H

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <cctype>
#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include "MatrixMath.h"
#include "Shader.h"
#include "Model.h"
#include "AudioEngine.h"
#include "FontRenderer.h"
#include "Strings.h"

enum class GameState {
    WELCOME,
    MENU = WELCOME,
    LOBBY_SELECT = WELCOME,
    CREATE_ROOM,
    ROOM_LIST,
    ROOM_LOBBY,
    COUNTDOWN,
    PLAYING,
    MATCH_OVER
};

enum class TouchAction {
    NONE,
    START_GAME,
    GOTO_CREATE_ROOM,
    GOTO_ROOM_LIST,
    START_LOCAL_GAME,
    CONFIRM_CREATE_ROOM,
    TOGGLE_PRIVACY,
    CYCLE_PIN,
    CYCLE_ROOM_NAME,
    EDIT_NICKNAME,
    EDIT_ROOM_NAME,
    EDIT_PIN,
    CYCLE_FILTER,
    SCROLL_UP_ROOMS,
    SCROLL_DOWN_ROOMS,
    JOIN_SELECTED_ROOM,
    REFRESH_ROOMS,
    SIMULATE_ADD_PLAYER,
    START_MULTIPLAYER_GAME,
    RETURN_TO_ROOM_LOBBY,
    CHANGE_TEAM,
    LEAVE_ROOM_LOBBY,
    BACK_TO_WELCOME,
    BACK_TO_LOBBY,
    RESET,
    PLAY
};

struct ServerRoomEntry {
    std::string id;
    std::string name;
    bool isPrivate = false;
    int playerCount = 0;
    int maxPlayers = 8;
    std::string pin;
};

struct PlayerInfo {
    std::string id;
    std::string name;
    std::string team;
    bool isLocal = false;
    bool connected = true;
    bool isOwner = false;
};

class GameUI {
public:
    GameUI()
        : state_(GameState::WELCOME),
          countdownTimer_(0.0f),
          matchTimer_(90.0f),
          uiCubeRot_(0.0f),
          lastCountdownSec_(-1),
          isPrivateRoom_(false),
          roomPin_("1234"),
          roomName_("Lobby"),
          userNickname_(""),
          isServerConnected_(false),
          connectedPlayerCount_(1),
          isOwner_(false),
          filterTypeIndex_(0),
          roomListScrollOffset_(0),
          joinNotificationTimer_(0.0f),
          showLeaveConfirmModal_(false),
          leaveModalFromMatch_(false),
          returnedToRoomFromMatch_(false),
          roomMatchActive_(false),
          pendingTeam_("BLUE"),
          myTeam_("BLUE"),
          pendingJoinRoom_(false),
          reconnectTimer_(0.0f) {
        initQuadGeometry();
        initMiniCubeGeometry();
        // Server rooms are populated ONLY from real server responses (0 fake rooms!)
        serverRooms_.clear();
    }

    GameState getState() const { return state_; }
    void setState(GameState state) { state_ = state; }

    float getMatchTimer() const { return matchTimer_; }
    void setMatchTimer(float t) { matchTimer_ = t; }

    bool isPrivateRoom() const { return isPrivateRoom_; }
    bool isOwner() const { return isOwner_; }
    const std::string& getRoomPin() const { return roomPin_; }
    const std::string& getRoomName() const { return roomName_; }
    const std::string& getSelectedRoomId() const { return selectedRoomId_; }
    const std::string& getUserNickname() const { return userNickname_; }
    bool isServerConnected() const { return isServerConnected_; }
    void setServerConnected(bool connected) { isServerConnected_ = connected; }

    int getConnectedPlayerCount() const { return connectedPlayerCount_; }
    void setConnectedPlayerCount(int count) {
        int prevCount = connectedPlayerCount_;
        connectedPlayerCount_ = std::clamp(count, 1, 8);
        if (connectedPlayerCount_ > prevCount) {
            triggerJoinNotification("JUGADOR " + std::to_string(connectedPlayerCount_));
        }
    }

    void initFont(AAssetManager* assetManager, const std::string& fontPath = "calculator.ttf") {
        fontRenderer_.loadFont(assetManager, fontPath, 128.0f);
    }

    void initCursorSprites(AAssetManager* assetManager) {
        arrowRedLeftTexture_ = TextureAsset::loadAsset(assetManager, "arrow_red_left.png");
        arrowBlueLeftTexture_ = TextureAsset::loadAsset(assetManager, "arrow_blue_left.png");
        arrowBlueRightTexture_ = TextureAsset::loadAsset(assetManager, "arrow_blue_right.png");
        arrowRedRightTexture_ = TextureAsset::loadAsset(assetManager, "arrow_red_right.png");
    }

    void triggerJoinNotification(const std::string& playerName) {
        joinNotificationText_ = "¡" + playerName + Strings::get(StringId::JOIN_NOTIFICATION_SUFFIX);
        joinNotificationTimer_ = 3.5f;
    }

    void triggerWelcomeNotification(const std::string& nickname) {
        joinNotificationText_ = (Strings::getLanguage() == Language::SPANISH) ?
            ("¡BIENVENIDO DE NUEVO, " + nickname + "!") :
            ("WELCOME BACK, " + nickname + "!");
        joinNotificationTimer_ = 4.0f;
    }

    void onTextInputResult(int fieldType, const std::string& text) {
        if (text.empty()) return;
        if (fieldType == 0) {
            userNickname_ = text;
            triggerWelcomeNotification(userNickname_);
        } else if (fieldType == 1) {
            roomName_ = text;
        } else if (fieldType == 2) {
            roomPin_ = text;
        } else if (fieldType == 3) {
            roomPin_ = text;
            pendingJoinRoom_ = true;
        }
    }

    bool checkAndClearPendingJoinRoom() {
        if (pendingJoinRoom_) {
            pendingJoinRoom_ = false;
            return true;
        }
        return false;
    }

    void setServerRooms(const std::vector<ServerRoomEntry>& rooms) {
        serverRooms_ = rooms;
        roomListScrollOffset_ = 0;
    }

    void setServerRoomsJson(const std::string& jsonRooms) {
        serverRooms_.clear();
        size_t pos = 0;
        while ((pos = jsonRooms.find('{', pos)) != std::string::npos) {
            size_t endPos = jsonRooms.find('}', pos);
            if (endPos == std::string::npos) break;
            std::string objStr = jsonRooms.substr(pos, endPos - pos + 1);

            ServerRoomEntry entry;
            auto getVal = [&](const std::string& key) -> std::string {
                std::string pattern = "\"" + key + "\":";
                size_t k = objStr.find(pattern);
                if (k == std::string::npos) return "";
                size_t valStart = k + pattern.length();
                while (valStart < objStr.length() && (objStr[valStart] == ' ' || objStr[valStart] == '"')) {
                    valStart++;
                }
                size_t valEnd = objStr.find_first_of("\",}", valStart);
                if (valEnd == std::string::npos) valEnd = objStr.length();
                return objStr.substr(valStart, valEnd - valStart);
            };

            entry.id = getVal("id");
            entry.name = getVal("name");
            std::string priv = getVal("isPrivate");
            entry.isPrivate = (priv.find("true") != std::string::npos || priv == "1");
            std::string pc = getVal("playerCount");
            if (!pc.empty()) entry.playerCount = std::atoi(pc.c_str());
            std::string mp = getVal("maxPlayers");
            if (!mp.empty()) entry.maxPlayers = std::atoi(mp.c_str());
            entry.pin = getVal("pin");

            if (!entry.id.empty()) {
                bool exists = false;
                for (const auto& existing : serverRooms_) {
                    if (existing.id == entry.id) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    serverRooms_.push_back(entry);
                }
            }
            pos = endPos + 1;
        }
        roomListScrollOffset_ = 0;
    }

    void onRoomJoined(const std::string& roomId, bool isOwner, const std::string& team = "BLUE") {
        bool preserveLobbyAfterReconnect = state_ == GameState::ROOM_LOBBY && returnedToRoomFromMatch_;
        state_ = GameState::ROOM_LOBBY;
        isOwner_ = isOwner;
        myTeam_ = team == "RED" ? "RED" : "BLUE";
        if (!preserveLobbyAfterReconnect) {
            returnedToRoomFromMatch_ = false;
            roomMatchActive_ = false;
        }
        if (isOwner) {
            connectedPlayerCount_ = 1;
        }
    }

    void onRoomStateUpdated(const std::string& name, int playerCount, bool isPrivate, const std::string& ownerId, bool isOwner, const std::string& state, const std::string& playersJson, const std::string& myTeam) {
        if (!name.empty()) roomName_ = name;
        isPrivateRoom_ = isPrivate;
        ownerId_ = ownerId;
        myTeam_ = myTeam == "RED" ? "RED" : "BLUE";

        if (state == "FINISHED") {
            if (returnedToRoomFromMatch_) {
                // A player who voluntarily returned to the room stays there
                // when the match later finishes for the other players.
                state_ = GameState::ROOM_LOBBY;
            } else {
                state_ = GameState::MATCH_OVER;
            }
            roomMatchActive_ = false;
        } else if (state == "PLAYING" && !returnedToRoomFromMatch_ &&
                   (state_ == GameState::ROOM_LOBBY || state_ == GameState::WELCOME)) {
            state_ = GameState::PLAYING;
            roomMatchActive_ = true;
            reconnectTimer_ = 0.0f;
        } else if (state == "PLAYING") {
            roomMatchActive_ = true;
        } else if (state == "LOBBY" &&
                   (state_ == GameState::MATCH_OVER || state_ == GameState::PLAYING || state_ == GameState::COUNTDOWN)) {
            state_ = GameState::ROOM_LOBBY;
            roomMatchActive_ = false;
            returnedToRoomFromMatch_ = false;
        } else if (state == "LOBBY") {
            roomMatchActive_ = false;
        }

        // Parse players JSON array from server: [{"id":"...","name":"..."},...]
        currentRoomPlayers_.clear();
        size_t pos = 0;
        while ((pos = playersJson.find('{', pos)) != std::string::npos) {
            size_t endPos = playersJson.find('}', pos);
            if (endPos == std::string::npos) break;
            std::string objStr = playersJson.substr(pos, endPos - pos + 1);

            auto getVal = [&](const std::string& key) -> std::string {
                size_t k = objStr.find("\"" + key + "\":");
                if (k == std::string::npos) return "";
                size_t valStart = objStr.find_first_not_of(" :\"", k + key.length() + 3);
                if (valStart == std::string::npos) return "";
                size_t valEnd = objStr.find_first_of("\",}", valStart);
                if (valEnd == std::string::npos) valEnd = objStr.length();
                return objStr.substr(valStart, valEnd - valStart);
            };

            PlayerInfo p;
            p.id = getVal("id");
            p.name = getVal("name");
            p.team = getVal("team");
            p.isLocal = getVal("isLocal") == "true" || getVal("isLocal") == "1";
            std::string connectedValue = getVal("connected");
            p.connected = connectedValue.empty() || connectedValue == "true" || connectedValue == "1";
            if (p.team != "RED") p.team = "BLUE";
            p.isOwner = (!p.id.empty() && p.id == ownerId_);
            if (!p.name.empty() && p.connected) {
                currentRoomPlayers_.push_back(p);
            }
            pos = endPos + 1;
        }

        int actualCount = currentRoomPlayers_.empty() ? playerCount : static_cast<int>(currentRoomPlayers_.size());

        // Trigger real-time visual banners for member leave / join / ownership transfer
        if (connectedPlayerCount_ > 0) {
            if (actualCount < connectedPlayerCount_) {
                joinNotificationText_ = "¡UN JUGADOR HA SALIDO DE LA SALA!";
                joinNotificationTimer_ = 3.5f;
            } else if (actualCount > connectedPlayerCount_) {
                joinNotificationText_ = "¡UN JUGADOR SE HA UNIDO A LA SALA!";
                joinNotificationTimer_ = 3.5f;
            }
        }

        if (!isOwner_ && isOwner) {
            joinNotificationText_ = "¡AHORA ERES EL CREADOR DE LA SALA!";
            joinNotificationTimer_ = 4.0f;
        }

        connectedPlayerCount_ = actualCount;
        isOwner_ = isOwner;
    }

    void onGameAborted(const std::string& reason) {
        state_ = GameState::ROOM_LOBBY;
        roomMatchActive_ = false;
        returnedToRoomFromMatch_ = false;
        joinNotificationText_ = "PARTIDA FINALIZADA: " + reason;
        joinNotificationTimer_ = 6.0f;
    }

    const std::string& getPendingTeam() const { return pendingTeam_; }

    void startCountdown(AudioEngine* audioEngine = nullptr, android_app* app = nullptr) {
        state_ = GameState::COUNTDOWN;
        roomMatchActive_ = true;
        returnedToRoomFromMatch_ = false;
        countdownTimer_ = 0.0f;
        matchTimer_ = 90.0f;
        lastCountdownSec_ = -1;
        if (audioEngine) audioEngine->playCountdownBeep();
        if (app) AudioEngine::triggerCountdownAudio(app);
    }

    void update(float deltaTime, AudioEngine* audioEngine = nullptr) {
        uiCubeRot_ += deltaTime * 50.0f;
        if (uiCubeRot_ >= 360.0f) uiCubeRot_ -= 360.0f;

        if (joinNotificationTimer_ > 0.0f) {
            joinNotificationTimer_ -= deltaTime;
            if (joinNotificationTimer_ < 0.0f) joinNotificationTimer_ = 0.0f;
        }

        if (state_ == GameState::COUNTDOWN) {
            int currentSec = static_cast<int>(countdownTimer_);
            countdownTimer_ += deltaTime;
            int newSec = static_cast<int>(countdownTimer_);

            if (newSec != currentSec) {
                if (newSec >= 1 && newSec <= 2 && audioEngine) {
                    audioEngine->playCountdownBeep();
                } else if (newSec == 3 && audioEngine) {
                    audioEngine->playGoChime();
                }
            }

            if (countdownTimer_ >= 4.0f) {
                state_ = GameState::PLAYING;
                matchTimer_ = 90.0f;
            }
        } else if (state_ == GameState::PLAYING) {
            if (!isServerConnected_) {
                reconnectTimer_ += deltaTime;
                if (reconnectTimer_ >= 15.0f) {
                    reconnectTimer_ = 0.0f;
                    state_ = GameState::WELCOME;
                }
            } else {
                reconnectTimer_ = 0.0f;
                matchTimer_ -= deltaTime;
                if (matchTimer_ <= 0.0f) {
                    matchTimer_ = 0.0f;
                    state_ = GameState::MATCH_OVER;
                    if (audioEngine) audioEngine->playGoChime();
                }
            }
        }
    }

    TouchAction handleTouch(float screenX, float screenY, float width, float height, AudioEngine* audioEngine = nullptr, android_app* app = nullptr) {
        float aspect = width / height;
        float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
        float normY = 1.0f - (2.0f * screenY) / height;

        // 0. Modal Confirmation Touch handling
        if (showLeaveConfirmModal_) {
            float yesX = -0.28f, yesY = -0.12f, btnW = 0.42f, btnH = 0.15f;
            float noX = 0.28f, noY = -0.12f;

            if (std::abs(normX - yesX) <= btnW * 0.5f + 0.04f &&
                std::abs(normY - yesY) <= btnH * 0.5f + 0.04f) {
                bool returningFromMatch = leaveModalFromMatch_;
                showLeaveConfirmModal_ = false;
                if (returningFromMatch) {
                    state_ = GameState::ROOM_LOBBY;
                    returnedToRoomFromMatch_ = true;
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::RETURN_TO_ROOM_LOBBY;
                }
                state_ = GameState::WELCOME;
                returnedToRoomFromMatch_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::LEAVE_ROOM_LOBBY;
            }
            if (std::abs(normX - noX) <= btnW * 0.5f + 0.04f &&
                std::abs(normY - noY) <= btnH * 0.5f + 0.04f) {
                showLeaveConfirmModal_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
            return TouchAction::NONE;
        }

        // 1. In-Game Leave check (Shows confirmation modal!)
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            float resetX = -aspect + 0.28f;
            float resetY = -0.82f;
            float resetW = 0.38f;
            float resetH = 0.12f;

            if (std::abs(normX - resetX) <= resetW * 0.5f + 0.04f &&
                std::abs(normY - resetY) <= resetH * 0.5f + 0.04f) {
                showLeaveConfirmModal_ = true;
                leaveModalFromMatch_ = true;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
        }

        // 2. MAIN MENU / WELCOME Screen Touch
        if (state_ == GameState::WELCOME) {
            // Upper-Left Test Mode Button (PARTIDA RAPIDA 1V1 TEST)
            float testX = -aspect + 0.28f, testY = 0.85f, testW = 0.38f, testH = 0.11f;
            if (std::abs(normX - testX) <= testW * 0.5f + 0.04f &&
                std::abs(normY - testY) <= testH * 0.5f + 0.04f) {
                connectedPlayerCount_ = 2; // Local 1v1 practice test
                startCountdown(audioEngine, app);
                return TouchAction::START_LOCAL_GAME;
            }

            // BUSCAR SALAS Button (Left Side)
            float leftX = -0.36f, btnY = -0.10f, btnW = 0.64f, btnH = 0.20f;
            if (std::abs(normX - leftX) <= btnW * 0.5f + 0.04f &&
                std::abs(normY - btnY) <= btnH * 0.5f + 0.04f) {
                state_ = GameState::ROOM_LIST;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::GOTO_ROOM_LIST;
            }

            // CREAR SALA Button (Right Side)
            float rightX = 0.36f;
            if (std::abs(normX - rightX) <= btnW * 0.5f + 0.04f &&
                std::abs(normY - btnY) <= btnH * 0.5f + 0.04f) {
                state_ = GameState::CREATE_ROOM;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::GOTO_CREATE_ROOM;
            }
        }
        // 3. CREATE ROOM Screen Touch
        else if (state_ == GameState::CREATE_ROOM) {
            float boxX = 0.0f, boxW = 0.85f, boxH = 0.12f;

            // Edit Room Name Box (Native Android EditText Keyboard Dialog!)
            float nameY = 0.18f;
            if (std::abs(normX - boxX) <= boxW * 0.5f + 0.04f &&
                std::abs(normY - nameY) <= boxH * 0.5f + 0.04f) {
                triggerNativeTextInput(app, 1, Strings::get(StringId::PROMPT_ROOM_NAME), roomName_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_ROOM_NAME;
            }

            // Toggle Privacy Button
            float togY = 0.04f;
            if (std::abs(normX - boxX) <= boxW * 0.5f + 0.04f &&
                std::abs(normY - togY) <= boxH * 0.5f + 0.04f) {
                isPrivateRoom_ = !isPrivateRoom_;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::TOGGLE_PRIVACY;
            }

            // Edit Password / PIN Box
            float pinY = -0.10f;
            if (std::abs(normX - boxX) <= boxW * 0.5f + 0.04f &&
                std::abs(normY - pinY) <= boxH * 0.5f + 0.04f) {
                triggerNativeTextInput(app, 2, Strings::get(StringId::PROMPT_ROOM_PIN), roomPin_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_PIN;
            }

            // CREAR SALA Button
            float createX = 0.0f, createY = -0.35f, createW = 0.85f, createH = 0.15f;
            if (std::abs(normX - createX) <= createW * 0.5f + 0.04f &&
                std::abs(normY - createY) <= createH * 0.5f + 0.04f) {
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::CONFIRM_CREATE_ROOM;
            }

            // VOLVER Button
            float backX = -aspect + 0.28f, backY = -0.82f, backW = 0.38f, backH = 0.12f;
            if (std::abs(normX - backX) <= backW * 0.5f + 0.04f &&
                std::abs(normY - backY) <= backH * 0.5f + 0.04f) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::BACK_TO_WELCOME;
            }
        }
        // 4. ROOM LOBBY Screen Touch (Waiting Room before start)
        else if (state_ == GameState::ROOM_LOBBY) {
            float lobbyCardW = std::min(2.65f, aspect * 1.85f);
            float startX = 0.0f, startY = -0.40f, startW = 0.90f, startH = 0.18f;
            if (std::abs(normX - startX) <= startW * 0.5f + 0.04f &&
                std::abs(normY - startY) <= startH * 0.5f + 0.04f) {
                    if (isOwner_ && !roomMatchActive_ && connectedPlayerCount_ >= 2) {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::START_MULTIPLAYER_GAME;
                } else {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::NONE;
                }
            }

            // Team switch arrows. A player can change teams while waiting in the lobby.
            float leftX = -lobbyCardW * 0.25f;
            float rightX = lobbyCardW * 0.25f;
            float teamBoxW = std::min(1.15f, lobbyCardW * 0.44f);
            float rowStartY = 0.14f;
            size_t blueIndex = 0;
            size_t redIndex = 0;
            for (const auto& player : currentRoomPlayers_) {
                bool isRed = player.team == "RED";
                size_t rowIndex = isRed ? redIndex++ : blueIndex++;
                float rowY = rowStartY - static_cast<float>(rowIndex) * 0.14f;
                float arrowX = isRed ? rightX - teamBoxW * 0.5f + 0.10f : leftX + teamBoxW * 0.5f - 0.10f;
                float arrowY = rowY - 0.005f;
                if (player.isLocal && !roomMatchActive_ && std::abs(normX - arrowX) <= 0.10f && std::abs(normY - arrowY) <= 0.08f) {
                    pendingTeam_ = isRed ? "BLUE" : "RED";
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::CHANGE_TEAM;
                }
            }

            // SALIR DE SALA Button (Opens confirm modal)
            float backX = -aspect + 0.28f, backY = -0.82f, backW = 0.38f, backH = 0.12f;
            if (std::abs(normX - backX) <= backW * 0.5f + 0.04f &&
                std::abs(normY - backY) <= backH * 0.5f + 0.04f) {
                showLeaveConfirmModal_ = true;
                leaveModalFromMatch_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
        }
        // 5. ROOM LIST Screen Touch (Scrollable & Filterable)
        else if (state_ == GameState::ROOM_LIST) {
            // Filter Selector Box
            float filterX = -0.30f, filterY = 0.28f, filterW = 0.95f, filterH = 0.13f;
            if (std::abs(normX - filterX) <= filterW * 0.5f + 0.05f &&
                std::abs(normY - filterY) <= filterH * 0.5f + 0.05f) {
                filterTypeIndex_ = (filterTypeIndex_ + 1) % 3;
                roomListScrollOffset_ = 0;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::CYCLE_FILTER;
            }

            // Scroll Up / Down Buttons
            float scrollX = 0.72f, scrollW = 0.40f, scrollH = 0.13f;
            float upY = 0.28f, downY = -0.28f;

            if (std::abs(normX - scrollX) <= scrollW * 0.5f + 0.05f &&
                std::abs(normY - upY) <= scrollH * 0.5f + 0.05f) {
                roomListScrollOffset_ = std::max(0, roomListScrollOffset_ - 1);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SCROLL_UP_ROOMS;
            }
            if (std::abs(normX - scrollX) <= scrollW * 0.5f + 0.05f &&
                std::abs(normY - downY) <= scrollH * 0.5f + 0.05f) {
                roomListScrollOffset_++;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SCROLL_DOWN_ROOMS;
            }

            // Join Room Buttons for visible real items (Expanded green button tap area!)
            auto filtered = getFilteredRooms();
            float joinX = 0.72f, joinW = 0.40f, joinH = 0.16f;
            float startY = 0.08f;

            for (size_t i = 0; i < 3 && (i + roomListScrollOffset_) < filtered.size(); ++i) {
                float rowY = startY - static_cast<float>(i) * 0.18f;
                if (std::abs(normX - joinX) <= joinW * 0.5f + 0.06f &&
                    std::abs(normY - rowY) <= joinH * 0.5f + 0.05f) {
                    const auto& room = filtered[i + roomListScrollOffset_];
                    selectedRoomId_ = room.id;
                    roomName_ = room.name;
                    isPrivateRoom_ = room.isPrivate;

                    if (room.isPrivate) {
                        triggerNativeTextInput(app, 3, "PIN DE SALA PRIVADA (" + room.name + "):", "");
                        if (audioEngine) audioEngine->playCountdownBeep();
                        return TouchAction::EDIT_PIN;
                    } else {
                        roomPin_ = "";
                        if (audioEngine) audioEngine->playCountdownBeep();
                        return TouchAction::JOIN_SELECTED_ROOM;
                    }
                }
            }

            // ACTUALIZAR Button
            float refX = 0.0f, refY = -0.48f, refW = 0.65f, refH = 0.14f;
            if (std::abs(normX - refX) <= refW * 0.5f + 0.05f &&
                std::abs(normY - refY) <= refH * 0.5f + 0.05f) {
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::REFRESH_ROOMS;
            }

            // VOLVER Button
            float backX = -aspect + 0.28f, backY = -0.82f, backW = 0.38f, backH = 0.12f;
            if (std::abs(normX - backX) <= backW * 0.5f + 0.04f &&
                std::abs(normY - backY) <= backH * 0.5f + 0.04f) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::BACK_TO_WELCOME;
            }
        }
        // 6. MATCH OVER Touch (Only Leader can press OTRA VEZ)
        else if (state_ == GameState::MATCH_OVER) {
            float playAgainX = 0.0f;
            float playAgainY = -0.24f;
            float playAgainW = 0.62f;
            float playAgainH = 0.15f;

            if (std::abs(normX - playAgainX) <= playAgainW * 0.5f + 0.05f &&
                std::abs(normY - playAgainY) <= playAgainH * 0.5f + 0.05f) {
                if (isOwner_) {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::START_MULTIPLAYER_GAME;
                } else {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::NONE;
                }
            }
        }

        return TouchAction::NONE;
    }

    void render(const Shader& shader, float width, float height, int redCount, int blueCount) const {
        float aspect = width / height;
        float ortho[16];
        MatrixMath::orthographic(ortho, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

        shader.setUseTexture(false);

        // 1. Top Score Panels, Timer & In-Game Leave Button (during active 3D gameplay states)
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            renderTopScorePanels(shader, ortho, aspect, redCount, blueCount);
            renderMatchTimerDisplay(shader, ortho);
            renderMatchPlayersPanel(shader, ortho, aspect);
            if (state_ == GameState::PLAYING || state_ == GameState::COUNTDOWN || state_ == GameState::MATCH_OVER) {
                renderBackButton(shader, ortho, aspect);
            }
        } else {
            // Render Global Server Connection Banner on all Menu Screens
            renderServerStatusBanner(shader, ortho, aspect);

            // Render Top Right Registered User NickName Badge with White Background
            renderTopRightNicknameBadge(shader, ortho, aspect);

            // Render Player Join Banner Notification if active
            if (joinNotificationTimer_ > 0.0f) {
                renderJoinNotificationBanner(shader, ortho);
            }
        }

        // 2. Render State Specific UI
        if (state_ == GameState::WELCOME) {
            renderWelcomeScreen(shader, ortho, aspect);
        } else if (state_ == GameState::CREATE_ROOM) {
            renderCreateRoomScreen(shader, ortho, aspect);
        } else if (state_ == GameState::ROOM_LOBBY) {
            renderRoomLobbyScreen(shader, ortho, aspect);
        } else if (state_ == GameState::ROOM_LIST) {
            renderRoomListScreen(shader, ortho, aspect);
        } else if (state_ == GameState::COUNTDOWN) {
            renderCountdown(shader, ortho);
        } else if (state_ == GameState::MATCH_OVER) {
            renderWinnerOverlay(shader, ortho, redCount, blueCount);
        }

        // 3. Render Modal Confirm Overlay if active
        if (showLeaveConfirmModal_) {
            renderLeaveConfirmModal(shader, ortho, aspect);
        } else if (state_ == GameState::PLAYING && !isServerConnected_) {
            renderReconnectingOverlay(shader, ortho, aspect);
        }
    }

private:
    static void triggerNativeTextInput(android_app* app, int fieldType, const std::string& title, const std::string& currentText) {
        if (!app || !app->activity || !app->activity->vm) return;
        JavaVM* vm = app->activity->vm;
        JNIEnv* env = nullptr;
        if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
        }

        jobject activityObj = app->activity->javaGameActivity;
        jclass clazz = env->GetObjectClass(activityObj);
        jmethodID showMethod = env->GetMethodID(clazz, "showTextInputDialog", "(ILjava/lang/String;Ljava/lang/String;)V");
        if (showMethod) {
            jstring titleStr = env->NewStringUTF(title.c_str());
            jstring textStr = env->NewStringUTF(currentText.c_str());
            env->CallVoidMethod(activityObj, showMethod, fieldType, titleStr, textStr);
            env->DeleteLocalRef(titleStr);
            env->DeleteLocalRef(textStr);
        }
        env->DeleteLocalRef(clazz);
    }

    [[nodiscard]] std::vector<ServerRoomEntry> getFilteredRooms() const {
        std::vector<ServerRoomEntry> res;
        for (const auto& room : serverRooms_) {
            if (filterTypeIndex_ == 1 && room.isPrivate) continue;
            if (filterTypeIndex_ == 2 && !room.isPrivate) continue;
            res.push_back(room);
        }
        return res;
    }

    void renderJoinNotificationBanner(const Shader& shader, const float* ortho) const {
        float banX = 0.0f, banY = 0.70f, banW = 1.25f, banH = 0.12f;
        drawQuad(shader, ortho, banX, banY, banW, banH, 0.10f, 0.65f, 0.35f, 0.95f);
        drawQuad(shader, ortho, banX, banY - banH * 0.5f, banW, 0.008f, 0.40f, 0.95f, 0.60f, 0.95f);
        drawTextString(shader, ortho, banX, banY, joinNotificationText_, 0.098f, 1.0f, 1.0f, 1.0f);
    }

    void renderServerStatusBanner(const Shader& shader, const float* ortho, float aspect) const {
        float bannerX = 0.0f;
        float bannerY = 0.85f;
        float bannerW = 1.15f;
        float bannerH = 0.11f;

        if (isServerConnected_) {
            drawQuad(shader, ortho, bannerX, bannerY, bannerW, bannerH, 0.08f, 0.22f, 0.12f, 0.92f);
            drawQuad(shader, ortho, bannerX, bannerY - bannerH * 0.5f, bannerW, 0.008f, 0.20f, 0.90f, 0.35f, 0.95f);
            drawTextString(shader, ortho, bannerX, bannerY, Strings::get(StringId::SERVER_CONNECTED), 0.098f, 0.30f, 1.0f, 0.45f);
        } else {
            drawQuad(shader, ortho, bannerX, bannerY, bannerW, bannerH, 0.24f, 0.08f, 0.10f, 0.92f);
            drawQuad(shader, ortho, bannerX, bannerY - bannerH * 0.5f, bannerW, 0.008f, 0.95f, 0.25f, 0.25f, 0.95f);
            drawTextString(shader, ortho, bannerX, bannerY, Strings::get(StringId::SERVER_DISCONNECTED), 0.093f, 1.0f, 0.40f, 0.40f);
        }
    }

    void renderTopRightNicknameBadge(const Shader& shader, const float* ortho, float aspect) const {
        if (userNickname_.empty()) return;

        float badgeX = aspect - 0.35f;
        float badgeY = 0.85f;
        float badgeW = 0.52f;
        float badgeH = 0.11f;

        // Pure white background box
        drawQuad(shader, ortho, badgeX, badgeY, badgeW, badgeH, 1.0f, 1.0f, 1.0f, 1.0f);
        // Accent border for depth
        drawQuad(shader, ortho, badgeX, badgeY - badgeH * 0.5f, badgeW, 0.008f, 0.20f, 0.60f, 0.95f, 1.0f);
        // Dark text for maximum contrast on white background
        drawTextString(shader, ortho, badgeX, badgeY, userNickname_, 0.098f, 0.05f, 0.08f, 0.12f);
    }

    void renderWelcomeScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = -0.03f;
        float cardW = 1.45f, cardH = 0.72f;

        // Container card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Upper-Left Corner Test Button (PARTIDA RAPIDA TEST 1V1)
        float testX = -aspect + 0.28f, testY = 0.85f, testW = 0.38f, testH = 0.11f;
        drawQuad(shader, ortho, testX, testY, testW, testH, 0.85f, 0.45f, 0.10f, 0.92f);
        drawTextString(shader, ortho, testX, testY, Strings::get(StringId::TEST_1V1), 0.087f, 1.0f, 1.0f, 1.0f);

        // Header Title
        drawTextString(shader, ortho, cx, cy + 0.18f, Strings::get(StringId::TITLE_GAME), 0.138f, 1.0f, 0.85f, 0.10f);

        // Navigation Buttons Side by Side (Left: BUSCAR SALAS, Right: CREAR SALA)
        float btnY = -0.12f, btnW = 0.64f, btnH = 0.20f;

        // BUSCAR SALAS (Left)
        float leftX = -0.36f;
        drawQuad(shader, ortho, leftX, btnY, btnW, btnH, 0.10f, 0.65f, 0.45f, 0.95f);
        drawQuad(shader, ortho, leftX, btnY - btnH * 0.5f, btnW, 0.01f, 0.40f, 0.95f, 0.70f, 0.95f);
        drawTextString(shader, ortho, leftX, btnY, Strings::get(StringId::SEARCH_ROOMS), 0.117f, 1.0f, 1.0f, 1.0f);

        // CREAR SALA (Right)
        float rightX = 0.36f;
        drawQuad(shader, ortho, rightX, btnY, btnW, btnH, 0.12f, 0.50f, 0.95f, 0.95f);
        drawQuad(shader, ortho, rightX, btnY - btnH * 0.5f, btnW, 0.01f, 0.50f, 0.85f, 1.0f, 0.95f);
        drawTextString(shader, ortho, rightX, btnY, Strings::get(StringId::CREATE_ROOM), 0.117f, 1.0f, 1.0f, 1.0f);
    }

    void renderCreateRoomScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = 0.02f;
        float cardW = 1.35f, cardH = 0.88f;

        // Container card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Title
        drawTextString(shader, ortho, cx, cy + 0.32f, Strings::get(StringId::CREATE_NEW_ROOM_TITLE), 0.135f, 1.0f, 0.90f, 0.20f);

        // 1. Room Name Box (Native Keyboard Editable!)
        float boxX = 0.0f, boxW = 0.85f, boxH = 0.12f;
        float nameY = 0.18f;
        drawQuad(shader, ortho, boxX, nameY, boxW, boxH, 0.12f, 0.16f, 0.24f, 0.92f);
        drawTextString(shader, ortho, boxX, nameY, Strings::get(StringId::ROOM_NAME_LABEL) + roomName_, 0.098f, 0.90f, 0.95f, 1.0f);

        // 2. Privacy Toggle Button
        float togY = 0.04f;
        float r = isPrivateRoom_ ? 0.95f : 0.10f;
        float g = isPrivateRoom_ ? 0.45f : 0.60f;
        float b = isPrivateRoom_ ? 0.10f : 0.95f;

        drawQuad(shader, ortho, boxX, togY, boxW, boxH, r, g, b, 0.95f);
        std::string typeStr = isPrivateRoom_ ? ("TIPO: " + Strings::get(StringId::FILTER_PRIVATE)) : ("TIPO: " + Strings::get(StringId::FILTER_PUBLIC));
        drawTextString(shader, ortho, boxX, togY, typeStr, 0.108f, 1.0f, 1.0f, 1.0f);

        // 3. Password / PIN Box
        float pinY = -0.10f;
        drawQuad(shader, ortho, boxX, pinY, boxW, boxH, 0.14f, 0.18f, 0.26f, 0.92f);
        std::string pinStr = isPrivateRoom_ ? ("PIN: " + roomPin_ + Strings::get(StringId::NICKNAME_EDIT)) : "PIN: ----";
        drawTextString(shader, ortho, boxX, pinY, pinStr, 0.098f, 0.80f, 0.85f, 0.90f);

        // 4. Rules & Info Label (POSITIONED SAFELY AT cy - 0.22f ABOVE THE CREAR BUTTON)
        drawTextString(shader, ortho, cx, cy - 0.22f, Strings::get(StringId::MAX_PLAYERS_NOTE), 0.084f, 0.60f, 0.80f, 1.0f);

        // 5. CREAR SALA Button
        float createX = 0.0f, createY = -0.35f, createW = 0.85f, createH = 0.15f;
        drawQuad(shader, ortho, createX, createY, createW, createH, 0.15f, 0.75f, 0.35f, 0.95f);
        drawQuad(shader, ortho, createX, createY - createH * 0.5f, createW, 0.01f, 0.50f, 0.95f, 0.60f, 0.95f);
        drawTextString(shader, ortho, createX, createY, Strings::get(StringId::CREATE_ROOM), 0.123f, 1.0f, 1.0f, 1.0f);

        // VOLVER Button (Bottom Left)
        renderBackButton(shader, ortho, aspect);
    }

    void renderRoomLobbyScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = -0.03f;
        float cardW = std::min(2.65f, aspect * 1.85f), cardH = 1.25f;

        // Container card (Expanded landscape layout)
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.015f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.015f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Room Name Title & Privacy Header (No accents!)
        drawTextString(shader, ortho, cx, cy + 0.44f, roomName_, 0.175f, 1.0f, 0.85f, 0.10f);
        std::string privStr = isPrivateRoom_ ? ("PRIVADA (PIN: " + roomPin_ + ")") : ("PUBLICA");
        drawTextString(shader, ortho, cx, cy + 0.30f, privStr, 0.115f, 0.60f, 0.80f, 1.0f);

        // --- TEAM COLUMNS (LEFT: BLUE, RIGHT: RED) ---
        float leftX = -cardW * 0.25f;
        float rightX = cardW * 0.25f;
        float teamBoxW = std::min(1.15f, cardW * 0.44f);
        float teamHeaderY = 0.22f;
        float rowStartY = 0.14f;
        float rowH = 0.12f;

        auto renderTeamColumn = [&](const std::string& team, float x, float r, float g, float b) {
            // The team is identified by its color; the text label is omitted.
            drawQuad(shader, ortho, x, teamHeaderY, teamBoxW, 0.025f, r, g, b, 0.95f);

            size_t rowIndex = 0;
            for (const auto& player : currentRoomPlayers_) {
                if (player.team != team || rowIndex >= 4) continue;
                float rowY = rowStartY - static_cast<float>(rowIndex) * 0.14f;
                drawQuad(shader, ortho, x, rowY, teamBoxW, rowH, 0.10f, 0.16f, 0.25f, 0.94f);
                if (player.isOwner) {
                    const float border = 0.008f;
                    drawQuad(shader, ortho, x, rowY + rowH * 0.5f, teamBoxW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x, rowY - rowH * 0.5f, teamBoxW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x - teamBoxW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x + teamBoxW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                }
                drawTextString(shader, ortho, x, rowY, player.name, 0.105f, r, g, b, 1.0f);

                // Only the local player's row is actionable, and only before the match starts.
                if (player.isLocal && !roomMatchActive_) {
                    const float arrowX = team == "BLUE" ? x + teamBoxW * 0.5f - 0.10f : x - teamBoxW * 0.5f + 0.10f;
                    const float arrowY = rowY - 0.005f;
                    // Each sprite already contains transparent padding and glow;
                    // render only the arrow without an extra opaque button.
                    drawCursorSprite(shader, ortho, arrowX, arrowY, team, team == "BLUE");
                }
                rowIndex++;
            }
        };

        renderTeamColumn("BLUE", leftX, 0.20f, 0.75f, 1.0f);
        renderTeamColumn("RED", rightX, 1.0f, 0.30f, 0.30f);

        // Player count and start control remain centered below the team columns.
        float countY = -0.13f;
        drawQuad(shader, ortho, 0.0f, countY, 1.15f, 0.13f, 0.12f, 0.16f, 0.24f, 0.92f);
        drawTextString(shader, ortho, 0.0f, countY, "JUGADORES: " + std::to_string(connectedPlayerCount_) + "/8", 0.115f, 0.20f, 0.90f, 1.0f);

        float startY = -0.40f, startW = 0.90f, startH = 0.18f;
        const float requiredW = 1.45f;
        if (roomMatchActive_) {
            drawQuad(shader, ortho, 0.0f, startY, startW, startH, 0.12f, 0.18f, 0.28f, 0.92f);
            drawTextString(shader, ortho, 0.0f, startY, "PARTIDA EN CURSO", 0.105f, 1.0f, 0.75f, 0.20f);
        } else if (isOwner_ && connectedPlayerCount_ >= 2) {
            drawQuad(shader, ortho, 0.0f, startY, startW, startH, 0.15f, 0.75f, 0.35f, 0.95f);
            drawQuad(shader, ortho, 0.0f, startY - startH * 0.5f, startW, 0.012f, 0.50f, 0.95f, 0.60f, 0.95f);
            drawTextString(shader, ortho, 0.0f, startY,
                           Strings::get(StringId::LOBBY_START_MATCH) + " (" + std::to_string(connectedPlayerCount_) + "/8)",
                           0.115f, 1.0f, 1.0f, 1.0f);
        } else if (isOwner_) {
            drawQuad(shader, ortho, 0.0f, startY, requiredW, startH, 0.12f, 0.18f, 0.28f, 0.95f);
            drawTextString(shader, ortho, 0.0f, startY, Strings::get(StringId::LOBBY_REQUIRED_PLAYERS), 0.105f, 1.0f, 0.85f, 0.30f);
        } else {
            drawQuad(shader, ortho, 0.0f, startY, startW, startH, 0.12f, 0.18f, 0.28f, 0.92f);
            drawTextString(shader, ortho, 0.0f, startY, "ESPERANDO AL CREADOR...", 0.100f, 0.90f, 0.95f, 1.0f);
        }

        // SALIR DE SALA Button (Bottom Left)
        renderBackButton(shader, ortho, aspect);
    }

    void renderRoomListScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = -0.02f;
        float cardW = std::min(2.45f, aspect * 1.80f), cardH = 1.35f;

        // Container card (Expanded landscape layout)
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.014f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.014f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Title
        drawTextString(shader, ortho, cx, cy + 0.48f, Strings::get(StringId::AVAILABLE_ROOMS_TITLE), 0.165f, 0.20f, 0.90f, 1.0f);

        // Search / Filter Selector Box
        std::vector<std::string> filterNames = {
            "BUSCAR: " + Strings::get(StringId::FILTER_ALL),
            "BUSCAR: " + Strings::get(StringId::FILTER_PUBLIC),
            "BUSCAR: " + Strings::get(StringId::FILTER_PRIVATE)
        };
        float filterX = -0.30f, filterY = 0.28f, filterW = 0.95f, filterH = 0.13f;
        drawQuad(shader, ortho, filterX, filterY, filterW, filterH, 0.12f, 0.16f, 0.24f, 0.92f);
        drawTextString(shader, ortho, filterX, filterY, filterNames[filterTypeIndex_], 0.110f, 0.80f, 0.90f, 1.0f);

        // Scroll Up ▲ & Scroll Down ▼ Control Buttons
        float scrollX = 0.72f, scrollW = 0.40f, scrollH = 0.13f;
        float upY = 0.28f, downY = -0.28f;
        drawQuad(shader, ortho, scrollX, upY, scrollW, scrollH, 0.14f, 0.22f, 0.35f, 0.92f);
        drawTextString(shader, ortho, scrollX, upY, Strings::get(StringId::SCROLL_UP), 0.110f, 1.0f, 1.0f, 1.0f);

        drawQuad(shader, ortho, scrollX, downY, scrollW, scrollH, 0.14f, 0.22f, 0.35f, 0.92f);
        drawTextString(shader, ortho, scrollX, downY, Strings::get(StringId::SCROLL_DOWN), 0.110f, 1.0f, 1.0f, 1.0f);

        // Filter Rooms
        auto filtered = getFilteredRooms();

        if (filtered.empty()) {
            float emptyY = -0.04f;
            drawQuad(shader, ortho, 0.0f, emptyY, 1.60f, 0.30f, 0.12f, 0.15f, 0.22f, 0.92f);
            drawTextString(shader, ortho, 0.0f, emptyY + 0.06f, Strings::get(StringId::EMPTY_ROOMS_LINE1), 0.110f, 1.0f, 0.45f, 0.35f);
            drawTextString(shader, ortho, 0.0f, emptyY - 0.06f, Strings::get(StringId::EMPTY_ROOMS_LINE2), 0.100f, 0.60f, 0.80f, 1.0f);
        } else {
            // Render visible real room rows with scroll offset
            float startY = 0.08f;
            size_t maxVisible = 3;
            size_t startIndex = std::min(static_cast<size_t>(roomListScrollOffset_), filtered.size() - 1);

            for (size_t i = 0; i < maxVisible && (startIndex + i) < filtered.size(); ++i) {
                const auto& room = filtered[startIndex + i];
                float rowY = startY - static_cast<float>(i) * 0.18f;

                // Room info box (Clean name without "PUBLICAS"/"PRIVADAS" appended!)
                drawQuad(shader, ortho, -0.20f, rowY, 1.30f, 0.16f, 0.13f, 0.17f, 0.25f, 0.92f);
                std::string label = room.name + " (" + std::to_string(room.playerCount) + "/" + std::to_string(room.maxPlayers) + ")";

                // Private rooms: Bright Orange text. Public rooms: Bright Cyan text.
                float r = room.isPrivate ? 1.00f : 0.20f;
                float g = room.isPrivate ? 0.55f : 0.90f;
                float b = room.isPrivate ? 0.10f : 1.00f;
                drawTextString(shader, ortho, -0.20f, rowY, label, 0.112f, r, g, b);

                // Join button (PIN for private, UNIRSE for public)
                drawQuad(shader, ortho, 0.72f, rowY, 0.40f, 0.16f, 0.12f, 0.78f, 0.35f, 0.95f);
                std::string btnText = room.isPrivate ? "PIN" : Strings::get(StringId::JOIN_ROOM);
                drawTextString(shader, ortho, 0.72f, rowY, btnText, 0.122f, 1.0f, 1.0f, 1.0f);
            }
        }

        // ACTUALIZAR Button
        float refX = 0.0f, refY = -0.48f, refW = 0.65f, refH = 0.14f;
        drawQuad(shader, ortho, refX, refY, refW, refH, 0.12f, 0.45f, 0.85f, 0.95f);
        drawTextString(shader, ortho, refX, refY, Strings::get(StringId::REFRESH), 0.110f, 1.0f, 1.0f, 1.0f);

        // VOLVER Button (Bottom Left)
        renderBackButton(shader, ortho, aspect);
    }

    void renderBackButton(const Shader& shader, const float* ortho, float aspect) const {
        float backX = -aspect + 0.28f;
        float backY = -0.82f;
        float backW = 0.38f;
        float backH = 0.12f;

        drawQuad(shader, ortho, backX, backY, backW, backH, 0.14f, 0.18f, 0.25f, 0.92f);
        drawQuad(shader, ortho, backX, backY - backH * 0.5f, backW, 0.01f, 0.4f, 0.7f, 1.0f, 0.95f);
        drawTextString(shader, ortho, backX, backY, Strings::get(StringId::BACK), 0.108f, 0.9f, 0.95f, 1.0f);
    }

    void renderMatchTimerDisplay(const Shader& shader, const float* ortho) const {
        float timerX = 0.0f;
        float timerY = 0.85f;
        float timerW = 0.44f;
        float timerH = 0.16f;

        // Container panel
        drawQuad(shader, ortho, timerX, timerY, timerW, timerH, 0.10f, 0.13f, 0.20f, 0.92f);

        // Pulse color if time is low
        float r = 0.1f, g = 0.9f, b = 1.0f;
        if (matchTimer_ <= 15.0f) {
            r = 1.0f; g = 0.3f; b = 0.2f;
        }

        drawQuad(shader, ortho, timerX, timerY - timerH * 0.5f, timerW, 0.01f, r, g, b, 0.95f);

        // Render "01:30" (or 90 seconds display)
        int totalSec = std::max(0, static_cast<int>(std::ceil(matchTimer_)));
        int mins = totalSec / 60;
        int secs = totalSec % 60;

        std::string timeStr = (mins < 10 ? "0" : "") + std::to_string(mins) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);

        drawTextString(shader, ortho, timerX, timerY, timeStr, 0.135f, r, g, b, 1.0f);
    }

    void renderWinnerOverlay(const Shader& shader, const float* ortho, int redCount, int blueCount) const {
        float cx = 0.0f;
        float cy = 0.03f;
        float cardW = 1.35f;
        float cardH = 0.82f;

        // Backdrop card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.09f, 0.12f, 0.17f, 0.95f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.4f, 0.7f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.4f, 0.7f, 1.0f, 0.95f);

        // 1. Victory / Defeat Status Banner for THIS User
        float statusY = cy + 0.28f;
        bool isMyTeamBlue = myTeam_ != "RED";
        bool isBlueWinner = blueCount > redCount;
        bool isRedWinner = redCount > blueCount;
        bool isTie = (redCount == blueCount);

        if (isTie) {
            std::string tieStr = (Strings::getLanguage() == Language::SPANISH) ? "¡EMPATE!" : "TIE GAME!";
            drawTextString(shader, ortho, cx, statusY, tieStr, 0.145f, 1.0f, 0.90f, 0.20f);
        } else {
            bool iWon = (isMyTeamBlue && isBlueWinner) || (!isMyTeamBlue && isRedWinner);
            if (iWon) {
                std::string winStr = (Strings::getLanguage() == Language::SPANISH) ? "¡HAS GANADO!" : "YOU WON!";
                drawTextString(shader, ortho, cx, statusY, winStr, 0.155f, 0.20f, 1.0f, 0.40f);
            } else {
                std::string loseStr = (Strings::getLanguage() == Language::SPANISH) ? "HAS PERDIDO" : "YOU LOST";
                drawTextString(shader, ortho, cx, statusY, loseStr, 0.155f, 1.0f, 0.30f, 0.30f);
            }
        }

        // 2. Winner Team Text
        float textY = cy + 0.12f;
        if (redCount > blueCount) {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::RED_WINS), 0.120f, 1.0f, 0.25f, 0.25f);
        } else if (blueCount > redCount) {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::BLUE_WINS), 0.120f, 0.1f, 0.65f, 1.0f);
        } else {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::DRAW), 0.120f, 1.0f, 0.9f, 0.2f);
        }

        // 3. Final Score Summary Text: RED: X | BLUE: Y
        float scoreY = cy - 0.05f;
        drawDigits(shader, ortho, cx - 0.22f, scoreY, redCount, 1.0f, 0.35f, 0.35f);
        drawQuad(shader, ortho, cx, scoreY, 0.01f, 0.08f, 0.5f, 0.5f, 0.6f, 0.8f);
        drawDigits(shader, ortho, cx + 0.12f, scoreY, blueCount, 0.25f, 0.68f, 1.0f);

        // 4. "OTRA VEZ" Button (for Leader) or "ESPERANDO AL LÍDER..." Card (for Member)
        float btnX = cx;
        float btnY = cy - 0.24f;
        float btnW = 0.62f;
        float btnH = 0.15f;

        if (isOwner_) {
            drawQuad(shader, ortho, btnX, btnY, btnW, btnH, 0.12f, 0.75f, 0.35f, 0.95f);
            drawQuad(shader, ortho, btnX, btnY - btnH * 0.5f, btnW, 0.01f, 0.40f, 0.95f, 0.60f, 0.95f);
            std::string btnStr = (Strings::getLanguage() == Language::SPANISH) ? "OTRA VEZ" : "PLAY AGAIN";
            drawTextString(shader, ortho, btnX, btnY, btnStr, 0.118f, 1.0f, 1.0f, 1.0f);
        } else {
            drawQuad(shader, ortho, btnX, btnY, btnW, btnH, 0.14f, 0.18f, 0.25f, 0.92f);
            drawQuad(shader, ortho, btnX, btnY - btnH * 0.5f, btnW, 0.01f, 0.20f, 0.80f, 1.0f, 0.95f);
            std::string waitStr = (Strings::getLanguage() == Language::SPANISH) ? "ESPERANDO AL LÍDER..." : "WAITING FOR LEADER...";
            drawTextString(shader, ortho, btnX, btnY, waitStr, 0.098f, 0.80f, 0.90f, 1.0f);
        }
    }

    void renderLeaveConfirmModal(const Shader& shader, const float* ortho, float aspect) const {
        if (!showLeaveConfirmModal_) return;

        // Dark background overlay
        drawQuad(shader, ortho, 0.0f, 0.0f, aspect * 2.1f, 2.1f, 0.0f, 0.0f, 0.0f, 0.75f);

        // Modal Card Container
        float cx = 0.0f, cy = 0.02f;
        float cardW = 1.25f, cardH = 0.52f;
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.12f, 0.20f, 0.96f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Modal Question Title
        std::string titleStr = (Strings::getLanguage() == Language::SPANISH) ?
            "¿DESEAS SALIR DE LA SALA?" : "DO YOU WANT TO LEAVE THE ROOM?";
        drawTextString(shader, ortho, cx, cy + 0.14f, titleStr, 0.115f, 1.0f, 0.90f, 0.20f);

        // Option Buttons
        float yesX = -0.28f, yesY = cy - 0.12f, btnW = 0.42f, btnH = 0.15f;
        float noX = 0.28f, noY = cy - 0.12f;

        // SÍ / YES Button (Red/Orange)
        drawQuad(shader, ortho, yesX, yesY, btnW, btnH, 0.85f, 0.25f, 0.20f, 0.95f);
        drawQuad(shader, ortho, yesX, yesY - btnH * 0.5f, btnW, 0.01f, 0.95f, 0.40f, 0.30f, 0.95f);
        std::string yesStr = (Strings::getLanguage() == Language::SPANISH) ? "SÍ, SALIR" : "YES, LEAVE";
        drawTextString(shader, ortho, yesX, yesY, yesStr, 0.105f, 1.0f, 1.0f, 1.0f);

        // NO Button (Green/Cyan)
        drawQuad(shader, ortho, noX, noY, btnW, btnH, 0.12f, 0.65f, 0.45f, 0.95f);
        drawQuad(shader, ortho, noX, noY - btnH * 0.5f, btnW, 0.01f, 0.40f, 0.95f, 0.70f, 0.95f);
        std::string noStr = (Strings::getLanguage() == Language::SPANISH) ? "CANCELAR" : "CANCEL";
        drawTextString(shader, ortho, noX, noY, noStr, 0.105f, 1.0f, 1.0f, 1.0f);
    }

    void renderReconnectingOverlay(const Shader& shader, const float* ortho, float aspect) const {
        float banX = 0.0f, banY = 0.0f, banW = 1.65f, banH = 0.35f;
        drawQuad(shader, ortho, banX, banY, banW, banH, 0.12f, 0.14f, 0.22f, 0.94f);
        drawQuad(shader, ortho, banX, banY - banH * 0.5f, banW, 0.01f, 1.0f, 0.60f, 0.10f, 0.95f);
        int secsLeft = std::max(0, 15 - static_cast<int>(reconnectTimer_));
        std::string msg = "RECONECTANDO A LA PARTIDA... (" + std::to_string(secsLeft) + "s)";
        drawTextString(shader, ortho, banX, banY, msg, 0.115f, 1.0f, 0.85f, 0.20f);
    }

    void renderMatchPlayersPanel(const Shader& shader, const float* ortho, float aspect) const {
        const float panelW = 0.72f;
        const float rowH = 0.095f;
        const float headerH = 0.105f;
        const float startY = 0.53f;
        const float leftX = -aspect + panelW * 0.5f + 0.06f;
        const float rightX = aspect - panelW * 0.5f - 0.06f;

        auto drawTeamPanel = [&](const std::string& team, float x, float r, float g, float b) {
            // The colored strip and cubes identify the team without a text label.
            drawQuad(shader, ortho, x, startY, panelW, 0.025f, r, g, b, 0.95f);

            size_t rowIndex = 0;
            for (const auto& player : currentRoomPlayers_) {
                if (player.team != team || rowIndex >= 4) continue;
                float rowY = startY - headerH * 0.5f - rowH * 0.75f - static_cast<float>(rowIndex) * rowH;
                drawQuad(shader, ortho, x, rowY, panelW, rowH, 0.08f, 0.14f, 0.22f, 0.90f);
                if (player.isOwner) {
                    const float border = 0.006f;
                    drawQuad(shader, ortho, x, rowY + rowH * 0.5f, panelW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x, rowY - rowH * 0.5f, panelW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x - panelW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                    drawQuad(shader, ortho, x + panelW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                }
                drawTextString(shader, ortho, x, rowY, player.name, 0.078f, r, g, b, 1.0f);
                rowIndex++;
            }
        };

        const bool localTeamIsRed = myTeam_ == "RED";
        const std::string localTeam = localTeamIsRed ? "RED" : "BLUE";
        const std::string opponentTeam = localTeamIsRed ? "BLUE" : "RED";
        drawTeamPanel(localTeam, leftX,
                      localTeamIsRed ? 1.0f : 0.20f,
                      localTeamIsRed ? 0.30f : 0.75f,
                      localTeamIsRed ? 0.30f : 1.0f);
        drawTeamPanel(opponentTeam, rightX,
                      localTeamIsRed ? 0.20f : 1.0f,
                      localTeamIsRed ? 0.75f : 0.30f,
                      localTeamIsRed ? 1.0f : 0.30f);
    }

    void renderTopScorePanels(const Shader& shader, const float* ortho, float aspect, int redCount, int blueCount) const {
        float panelY = 0.85f;
        float panelW = 0.52f;
        float panelH = 0.16f;

        const bool localTeamIsRed = myTeam_ == "RED";
        const int localScore = localTeamIsRed ? redCount : blueCount;
        const int opponentScore = localTeamIsRed ? blueCount : redCount;
        const float localR = localTeamIsRed ? 1.0f : 0.05f;
        const float localG = localTeamIsRed ? 0.22f : 0.55f;
        const float localB = localTeamIsRed ? 0.22f : 1.0f;
        const float opponentR = localTeamIsRed ? 0.05f : 1.0f;
        const float opponentG = localTeamIsRed ? 0.55f : 0.22f;
        const float opponentB = localTeamIsRed ? 1.0f : 0.22f;

        // The local player's team is always shown on the left.
        float localX = -aspect + panelW * 0.5f + 0.06f;
        drawQuad(shader, ortho, localX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, localX, panelY - panelH * 0.5f, panelW, 0.01f, localR, localG, localB, 0.95f);
        draw3DMiniCube(shader, ortho, localX - panelW * 0.30f, panelY, localR, localG, localB);
        drawDigits(shader, ortho, localX + 0.04f, panelY, localScore, localR, localG, localB);

        float opponentX = aspect - panelW * 0.5f - 0.06f;
        drawQuad(shader, ortho, opponentX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, opponentX, panelY - panelH * 0.5f, panelW, 0.01f, opponentR, opponentG, opponentB, 0.95f);
        draw3DMiniCube(shader, ortho, opponentX - panelW * 0.30f, panelY, opponentR, opponentG, opponentB);
        drawDigits(shader, ortho, opponentX + 0.04f, panelY, opponentScore, opponentR, opponentG, opponentB);
    }

    void drawTextString(const Shader& shader, const float* ortho, float cx, float cy, const std::string& text,
                        float size, float r, float g, float b, float a = 1.0f) const {
        if (fontRenderer_.isLoaded()) {
            fontRenderer_.drawText(shader, ortho, cx, cy, text, size, r, g, b, a, true);
        }
    }

    void drawDigits(const Shader& shader, const float* ortho, float startX, float startY, int number,
                    float r, float g, float b) const {
        std::string s = std::to_string(number);
        if (fontRenderer_.isLoaded()) {
            fontRenderer_.drawText(shader, ortho, startX, startY, s, 0.14f, r, g, b, 1.0f, false);
        }
    }

    void draw3DMiniCube(const Shader& shader, const float* ortho, float cx, float cy, float r, float g, float b) const {
        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, cx, cy, 0.0f);
        MatrixMath::rotateY(model, degToRad(uiCubeRot_));
        MatrixMath::rotateX(model, degToRad(22.0f));
        MatrixMath::scale(model, 0.048f, 0.048f, 0.048f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);

        for (size_t f = 0; f < 6; ++f) {
            float shade = miniCubeShades_[f];
            shader.setColor(r * shade, g * shade, b * shade, 1.0f);
            shader.drawIndexed(
                miniCubeVertices_.data() + f * 4,
                sizeof(Vertex),
                0,
                sizeof(Vector3),
                miniCubeIndices_.data() + f * 6,
                6
            );
        }
    }

    void renderCountdown(const Shader& shader, const float* ortho) const {
        int sec = static_cast<int>(countdownTimer_);
        float fraction = countdownTimer_ - static_cast<float>(sec);

        float scale = 1.0f + std::sin(fraction * PI) * 0.35f;
        float alpha = 1.0f - std::pow(fraction, 2.5f);

        if (sec == 0) {
            drawTextString(shader, ortho, 0.0f, 0.0f, "3", scale * 0.4f, 1.0f, 0.9f, 0.2f, alpha);
        } else if (sec == 1) {
            drawTextString(shader, ortho, 0.0f, 0.0f, "2", scale * 0.4f, 1.0f, 0.6f, 0.2f, alpha);
        } else if (sec == 2) {
            drawTextString(shader, ortho, 0.0f, 0.0f, "1", scale * 0.4f, 0.2f, 1.0f, 0.4f, alpha);
        } else if (sec == 3) {
            drawTextString(shader, ortho, 0.0f, 0.0f, Strings::get(StringId::COUNTDOWN_GO), scale * 0.35f, 0.1f, 0.9f, 1.0f, alpha);
        }
    }

    void drawQuad(const Shader& shader, const float* ortho, float x, float y, float w, float h,
                  float r, float g, float b, float a) const {
        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, x, y, 0.0f);
        MatrixMath::scale(model, w * 0.5f, h * 0.5f, 1.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(r, g, b, a);

        shader.drawIndexed(
            quadVertices_.data(),
            sizeof(Vertex),
            0,
            sizeof(Vector3),
            quadIndices_.data(),
            quadIndices_.size()
        );
    }

    void drawCursorSprite(const Shader& shader, const float* ortho, float x, float y,
                          const std::string& team, bool pointsRight) const {
        const std::shared_ptr<TextureAsset>& texture = team == "BLUE"
                ? (pointsRight ? arrowBlueRightTexture_ : arrowBlueLeftTexture_)
                : (pointsRight ? arrowRedRightTexture_ : arrowRedLeftTexture_);
        if (!texture) return;

        const float w = 0.18f;
        const float h = 0.125f;

        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, x, y, 0.0f);
        MatrixMath::scale(model, w * 0.5f, h * 0.5f, 1.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader.setUseTexture(true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getTextureID());

        const Vertex vertices[] = {
            Vertex(Vector3{-1.0f,  1.0f, 0.0f}, Vector2{0.0f, 1.0f}),
            Vertex(Vector3{ 1.0f,  1.0f, 0.0f}, Vector2{1.0f, 1.0f}),
            Vertex(Vector3{ 1.0f, -1.0f, 0.0f}, Vector2{1.0f, 0.0f}),
            Vertex(Vector3{-1.0f, -1.0f, 0.0f}, Vector2{0.0f, 0.0f})
        };
        static const uint16_t indices[] = {0, 1, 2, 0, 2, 3};
        shader.drawIndexed(vertices, sizeof(Vertex), 0, sizeof(Vector3), indices, 6);
        shader.setUseTexture(false);
    }

    void drawSingleDigit(const Shader& shader, const float* ortho, float x, float y, int digit, float size,
                         float r, float g, float b, float a) const {
        static const bool segs[10][7] = {
            {true, true, true, false, true, true, true},   // 0
            {false, false, true, false, false, true, false}, // 1
            {true, false, true, true, true, false, true},   // 2
            {true, false, true, true, false, true, true},   // 3
            {false, true, true, true, false, true, false},  // 4
            {true, true, false, true, false, true, true},   // 5
            {true, true, false, true, true, true, true},   // 6
            {true, false, true, false, false, true, false}, // 7
            {true, true, true, true, true, true, true},   // 8
            {true, true, true, true, false, true, true}    // 9
        };

        if (digit < 0 || digit > 9) return;
        const bool* s = segs[digit];
        float t = size * 0.12f;
        float w = size * 0.45f;
        float h = size * 0.45f;

        if (s[0]) drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        if (s[1]) drawQuad(shader, ortho, x - w * 0.5f, y + h * 0.5f, t, h, r, g, b, a);
        if (s[2]) drawQuad(shader, ortho, x + w * 0.5f, y + h * 0.5f, t, h, r, g, b, a);
        if (s[3]) drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
        if (s[4]) drawQuad(shader, ortho, x - w * 0.5f, y - h * 0.5f, t, h, r, g, b, a);
        if (s[5]) drawQuad(shader, ortho, x + w * 0.5f, y - h * 0.5f, t, h, r, g, b, a);
        if (s[6]) drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void initQuadGeometry() {
        quadVertices_ = {
            Vertex(Vector3{-1.0f,  1.0f, 0.0f}, Vector2{0, 0}),
            Vertex(Vector3{ 1.0f,  1.0f, 0.0f}, Vector2{1, 0}),
            Vertex(Vector3{ 1.0f, -1.0f, 0.0f}, Vector2{1, 1}),
            Vertex(Vector3{-1.0f, -1.0f, 0.0f}, Vector2{0, 1})
        };
        quadIndices_ = { 0, 1, 2, 0, 2, 3 };

        circleVertices_.clear();
        circleIndices_.clear();
        circleVertices_.emplace_back(Vector3{0, 0, 0}, Vector2{0.5f, 0.5f});

        constexpr int SEGMENTS = 24;
        for (int i = 0; i <= SEGMENTS; ++i) {
            float angle = (static_cast<float>(i) * 2.0f * PI) / static_cast<float>(SEGMENTS);
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            circleVertices_.emplace_back(Vector3{cosA, sinA, 0.0f}, Vector2{0.5f + cosA * 0.5f, 0.5f + sinA * 0.5f});
        }
        for (int i = 1; i <= SEGMENTS; ++i) {
            circleIndices_.push_back(0);
            circleIndices_.push_back(i);
            circleIndices_.push_back(i + 1);
        }
    }

    void initMiniCubeGeometry() {
        float h = 1.0f;
        miniCubeVertices_.clear();
        miniCubeIndices_.clear();

        addMiniCubeFace(Vec3(-h, h,  h), Vec3( h, h,  h), Vec3( h, h, -h), Vec3(-h, h, -h), 1.0f);
        addMiniCubeFace(Vec3(-h, -h, -h), Vec3( h, -h, -h), Vec3( h, -h,  h), Vec3(-h, -h,  h), 0.5f);
        addMiniCubeFace(Vec3(-h, -h,  h), Vec3( h, -h,  h), Vec3( h,  h,  h), Vec3(-h,  h,  h), 0.9f);
        addMiniCubeFace(Vec3( h, -h, -h), Vec3(-h, -h, -h), Vec3(-h,  h, -h), Vec3( h,  h, -h), 0.7f);
        addMiniCubeFace(Vec3( h, -h,  h), Vec3( h, -h, -h), Vec3( h,  h, -h), Vec3( h,  h,  h), 0.85f);
        addMiniCubeFace(Vec3(-h, -h, -h), Vec3(-h, -h,  h), Vec3(-h,  h,  h), Vec3(-h,  h, -h), 0.75f);
    }

    void addMiniCubeFace(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, float shade) {
        miniCubeVertices_.emplace_back(Vector3{p0.x, p0.y, p0.z}, Vector2{0, 0});
        miniCubeVertices_.emplace_back(Vector3{p1.x, p1.y, p1.z}, Vector2{1, 0});
        miniCubeVertices_.emplace_back(Vector3{p2.x, p2.y, p2.z}, Vector2{1, 1});
        miniCubeVertices_.emplace_back(Vector3{p3.x, p3.y, p3.z}, Vector2{0, 1});

        miniCubeIndices_.push_back(0); miniCubeIndices_.push_back(1); miniCubeIndices_.push_back(2);
        miniCubeIndices_.push_back(0); miniCubeIndices_.push_back(2); miniCubeIndices_.push_back(3);

        miniCubeShades_.push_back(shade);
    }

    GameState state_;
    float countdownTimer_;
    float matchTimer_;
    float uiCubeRot_;
    int lastCountdownSec_;
    bool isPrivateRoom_;
    bool pendingJoinRoom_;
    std::string roomPin_;
    std::string roomName_;
    std::string selectedRoomId_;
    std::string userNickname_;
    bool isServerConnected_;
    int connectedPlayerCount_;
    bool isOwner_;
    std::string ownerId_;
    size_t filterTypeIndex_;
    int roomListScrollOffset_;
    std::vector<ServerRoomEntry> serverRooms_;
    std::vector<PlayerInfo> currentRoomPlayers_;
    std::string joinNotificationText_;
    float joinNotificationTimer_;
    bool showLeaveConfirmModal_;
    bool leaveModalFromMatch_;
    bool returnedToRoomFromMatch_;
    bool roomMatchActive_;
    std::string pendingTeam_;
    std::string myTeam_;
    float reconnectTimer_;
    FontRenderer fontRenderer_;
    std::shared_ptr<TextureAsset> arrowRedLeftTexture_;
    std::shared_ptr<TextureAsset> arrowBlueLeftTexture_;
    std::shared_ptr<TextureAsset> arrowBlueRightTexture_;
    std::shared_ptr<TextureAsset> arrowRedRightTexture_;

    std::vector<Vertex> quadVertices_;
    std::vector<uint16_t> quadIndices_;
    std::vector<Vertex> circleVertices_;
    std::vector<uint16_t> circleIndices_;

    std::vector<Vertex> miniCubeVertices_;
    std::vector<uint16_t> miniCubeIndices_;
    std::vector<float> miniCubeShades_;
};

#endif // ANDROIDGLINVESTIGATIONS_GAMEUI_H
