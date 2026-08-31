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
    LEAVE_ROOM_LOBBY,
    BACK_TO_WELCOME,
    BACK_TO_LOBBY,
    RESET,
    PLAY
};

struct ServerRoomEntry {
    std::string name;
    bool isPrivate = false;
    int playerCount = 0;
    int maxPlayers = 8;
    std::string pin;
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
          roomName_("SALA DE JUGADOR"),
          userNickname_("JUGADOR 1"),
          isServerConnected_(true),
          connectedPlayerCount_(1),
          filterTypeIndex_(0),
          roomListScrollOffset_(0),
          joinNotificationTimer_(0.0f) {
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
    const std::string& getRoomPin() const { return roomPin_; }
    const std::string& getRoomName() const { return roomName_; }
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

    void triggerJoinNotification(const std::string& playerName) {
        joinNotificationText_ = "¡" + playerName + Strings::get(StringId::JOIN_NOTIFICATION_SUFFIX);
        joinNotificationTimer_ = 3.5f;
    }

    void onTextInputResult(int fieldType, const std::string& text) {
        if (text.empty()) return;
        if (fieldType == 0) {
            userNickname_ = text;
        } else if (fieldType == 1) {
            roomName_ = text;
        } else if (fieldType == 2) {
            roomPin_ = text;
        }
    }

    void setServerRooms(const std::vector<ServerRoomEntry>& rooms) {
        serverRooms_ = rooms;
        roomListScrollOffset_ = 0;
    }

    void startCountdown(AudioEngine* audioEngine = nullptr) {
        state_ = GameState::COUNTDOWN;
        countdownTimer_ = 0.0f;
        matchTimer_ = 90.0f;
        lastCountdownSec_ = -1;
        if (audioEngine) audioEngine->playCountdownBeep();
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
            matchTimer_ -= deltaTime;
            if (matchTimer_ <= 0.0f) {
                matchTimer_ = 0.0f;
                state_ = GameState::MATCH_OVER;
                if (audioEngine) audioEngine->playGoChime();
            }
        }
    }

    TouchAction handleTouch(float screenX, float screenY, float width, float height, AudioEngine* audioEngine = nullptr, android_app* app = nullptr) {
        float aspect = width / height;
        float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
        float normY = 1.0f - (2.0f * screenY) / height;

        // 1. Reset / Exit Match check
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            float resetX = -aspect + 0.28f;
            float resetY = 0.65f;
            float resetW = 0.38f;
            float resetH = 0.14f;

            if (std::abs(normX - resetX) <= resetW * 0.5f + 0.03f &&
                std::abs(normY - resetY) <= resetH * 0.5f + 0.03f) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::RESET;
            }
        }

        // 2. MAIN MENU / WELCOME Screen Touch
        if (state_ == GameState::WELCOME) {
            // Upper-Left Test Mode Button (PARTIDA RAPIDA 1V1 TEST)
            float testX = -aspect + 0.28f, testY = 0.85f, testW = 0.38f, testH = 0.11f;
            if (std::abs(normX - testX) <= testW * 0.5f + 0.04f &&
                std::abs(normY - testY) <= testH * 0.5f + 0.04f) {
                connectedPlayerCount_ = 2; // Local 1v1 practice test
                startCountdown(audioEngine);
                return TouchAction::START_LOCAL_GAME;
            }

            // Edit Nickname Box
            float nickX = 0.0f, nickY = 0.20f, nickW = 1.15f, nickH = 0.14f;
            if (std::abs(normX - nickX) <= nickW * 0.5f + 0.04f &&
                std::abs(normY - nickY) <= nickH * 0.5f + 0.04f) {
                triggerNativeTextInput(app, 0, Strings::get(StringId::PROMPT_ENTER_NICKNAME), userNickname_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_NICKNAME;
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
                state_ = GameState::ROOM_LOBBY;
                connectedPlayerCount_ = 1; // Starts with 1 player (Owner)
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
            // Add Player Simulation Button (+1 Player)
            float simX = 0.0f, simY = -0.06f, simW = 0.85f, simH = 0.12f;
            if (std::abs(normX - simX) <= simW * 0.5f + 0.04f &&
                std::abs(normY - simY) <= simH * 0.5f + 0.04f) {
                connectedPlayerCount_++;
                if (connectedPlayerCount_ > 8) connectedPlayerCount_ = 1;
                triggerJoinNotification("JUGADOR " + std::to_string(connectedPlayerCount_));
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SIMULATE_ADD_PLAYER;
            }

            // START GAME Button (Requires at least 2 players!)
            float startX = 0.0f, startY = -0.26f, startW = 0.85f, startH = 0.16f;
            if (std::abs(normX - startX) <= startW * 0.5f + 0.04f &&
                std::abs(normY - startY) <= startH * 0.5f + 0.04f) {
                if (connectedPlayerCount_ >= 2) {
                    startCountdown(audioEngine);
                    return TouchAction::START_MULTIPLAYER_GAME;
                } else {
                    // Less than 2 players -> blocked!
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::NONE;
                }
            }

            // SALIR DE SALA Button
            float backX = -aspect + 0.28f, backY = -0.82f, backW = 0.38f, backH = 0.12f;
            if (std::abs(normX - backX) <= backW * 0.5f + 0.04f &&
                std::abs(normY - backY) <= backH * 0.5f + 0.04f) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::LEAVE_ROOM_LOBBY;
            }
        }
        // 5. ROOM LIST Screen Touch (Scrollable & Filterable)
        else if (state_ == GameState::ROOM_LIST) {
            // Filter Selector Box
            float filterX = -0.15f, filterY = 0.20f, filterW = 0.70f, filterH = 0.10f;
            if (std::abs(normX - filterX) <= filterW * 0.5f + 0.04f &&
                std::abs(normY - filterY) <= filterH * 0.5f + 0.04f) {
                filterTypeIndex_ = (filterTypeIndex_ + 1) % 3;
                roomListScrollOffset_ = 0;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::CYCLE_FILTER;
            }

            // Scroll Up / Down Buttons
            float scrollX = 0.52f, scrollW = 0.22f, scrollH = 0.10f;
            float upY = 0.20f, downY = -0.22f;

            if (std::abs(normX - scrollX) <= scrollW * 0.5f + 0.04f &&
                std::abs(normY - upY) <= scrollH * 0.5f + 0.04f) {
                roomListScrollOffset_ = std::max(0, roomListScrollOffset_ - 1);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SCROLL_UP_ROOMS;
            }
            if (std::abs(normX - scrollX) <= scrollW * 0.5f + 0.04f &&
                std::abs(normY - downY) <= scrollH * 0.5f + 0.04f) {
                roomListScrollOffset_++;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SCROLL_DOWN_ROOMS;
            }

            // Join Room Buttons for visible real items
            auto filtered = getFilteredRooms();
            float joinX = 0.42f, joinW = 0.32f, joinH = 0.11f;
            float startY = 0.06f;

            for (size_t i = 0; i < 3 && (i + roomListScrollOffset_) < filtered.size(); ++i) {
                float rowY = startY - static_cast<float>(i) * 0.14f;
                if (std::abs(normX - joinX) <= joinW * 0.5f + 0.04f &&
                    std::abs(normY - rowY) <= joinH * 0.5f + 0.04f) {
                    const auto& room = filtered[i + roomListScrollOffset_];
                    roomName_ = room.name;
                    isPrivateRoom_ = room.isPrivate;
                    roomPin_ = room.pin;
                    connectedPlayerCount_ = room.playerCount + 1;
                    state_ = GameState::ROOM_LOBBY;
                    triggerJoinNotification(userNickname_);
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::JOIN_SELECTED_ROOM;
                }
            }

            // ACTUALIZAR Button
            float refX = 0.0f, refY = -0.38f, refW = 0.55f, refH = 0.12f;
            if (std::abs(normX - refX) <= refW * 0.5f + 0.04f &&
                std::abs(normY - refY) <= refH * 0.5f + 0.04f) {
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
        // 6. MATCH OVER Touch
        else if (state_ == GameState::MATCH_OVER) {
            float playAgainX = 0.0f;
            float playAgainY = -0.20f;
            float playAgainW = 0.55f;
            float playAgainH = 0.16f;

            if (std::abs(normX - playAgainX) <= playAgainW * 0.5f + 0.04f &&
                std::abs(normY - playAgainY) <= playAgainH * 0.5f + 0.04f) {
                startCountdown(audioEngine);
                return TouchAction::PLAY;
            }
        }

        return TouchAction::NONE;
    }

    void render(const Shader& shader, float width, float height, int redCount, int blueCount) const {
        float aspect = width / height;
        float ortho[16];
        MatrixMath::orthographic(ortho, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

        shader.setUseTexture(false);

        // 1. Top Score Panels & Timer (only during active 3D gameplay states)
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            renderTopScorePanels(shader, ortho, aspect, redCount, blueCount);
            renderMatchTimerDisplay(shader, ortho);
        } else {
            // Render Global Server Connection Banner on all Menu Screens
            renderServerStatusBanner(shader, ortho, aspect);

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

    void renderWelcomeScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = -0.03f;
        float cardW = 1.45f, cardH = 0.85f;

        // Container card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Upper-Left Corner Test Button (PARTIDA RAPIDA TEST 1V1)
        float testX = -aspect + 0.28f, testY = 0.85f, testW = 0.38f, testH = 0.11f;
        drawQuad(shader, ortho, testX, testY, testW, testH, 0.85f, 0.45f, 0.10f, 0.92f);
        drawTextString(shader, ortho, testX, testY, Strings::get(StringId::TEST_1V1), 0.087f, 1.0f, 1.0f, 1.0f);

        // Header Title
        drawTextString(shader, ortho, cx, cy + 0.32f, Strings::get(StringId::TITLE_GAME), 0.138f, 1.0f, 0.85f, 0.10f);

        // User Profile Nickname Row
        float nickX = 0.0f, nickY = 0.18f, nickW = 1.15f, nickH = 0.14f;
        drawQuad(shader, ortho, nickX, nickY, nickW, nickH, 0.12f, 0.16f, 0.25f, 0.92f);
        drawTextString(shader, ortho, nickX, nickY, Strings::get(StringId::NICKNAME_LABEL) + userNickname_ + Strings::get(StringId::NICKNAME_EDIT), 0.098f, 0.20f, 0.90f, 1.0f);

        // Navigation Buttons Side by Side (Left: BUSCAR SALAS, Right: CREAR SALA)
        float btnY = -0.10f, btnW = 0.64f, btnH = 0.20f;

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
        float cardW = 1.40f, cardH = 0.85f;

        // Container card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Room Name Title & Privacy
        drawTextString(shader, ortho, cx, cy + 0.31f, roomName_, 0.132f, 1.0f, 0.85f, 0.10f);
        std::string privStr = isPrivateRoom_ ? ("SALA PRIVADA (PIN: " + roomPin_ + ")") : ("SALA PÚBLICA");
        drawTextString(shader, ortho, cx, cy + 0.22f, privStr, 0.087f, 0.60f, 0.80f, 1.0f);

        // Connected Players Banner (Count display: e.g. 1/8 or 3/8)
        float pBoxX = 0.0f, pBoxY = 0.10f, pBoxW = 1.15f, pBoxH = 0.11f;
        drawQuad(shader, ortho, pBoxX, pBoxY, pBoxW, pBoxH, 0.12f, 0.16f, 0.24f, 0.92f);
        std::string countStr = Strings::get(StringId::LOBBY_REQUIRED_PLAYERS);
        drawTextString(shader, ortho, pBoxX, pBoxY, "JUGADORES EN SALA: " + std::to_string(connectedPlayerCount_) + "/8", 0.102f, 0.20f, 0.90f, 1.0f);

        // Simulation Button (+1 Player)
        float simX = 0.0f, simY = -0.06f, simW = 0.85f, simH = 0.12f;
        drawQuad(shader, ortho, simX, simY, simW, simH, 0.14f, 0.22f, 0.35f, 0.92f);
        drawTextString(shader, ortho, simX, simY, Strings::get(StringId::LOBBY_ADD_TEST_PLAYER), 0.093f, 0.85f, 0.95f, 1.0f);

        // START GAME Button Rule
        float startX = 0.0f, startY = -0.26f, startW = 0.85f, startH = 0.16f;
        if (connectedPlayerCount_ < 2) {
            // Disabled state (Red/Dark)
            drawQuad(shader, ortho, startX, startY, startW, startH, 0.30f, 0.12f, 0.14f, 0.92f);
            drawQuad(shader, ortho, startX, startY - startH * 0.5f, startW, 0.01f, 0.70f, 0.20f, 0.20f, 0.95f);
            drawTextString(shader, ortho, startX, startY, Strings::get(StringId::LOBBY_REQUIRED_PLAYERS), 0.098f, 0.95f, 0.40f, 0.40f);
        } else {
            // Enabled state (Green/Cyan)
            drawQuad(shader, ortho, startX, startY, startW, startH, 0.15f, 0.75f, 0.35f, 0.95f);
            drawQuad(shader, ortho, startX, startY - startH * 0.5f, startW, 0.01f, 0.50f, 0.95f, 0.60f, 0.95f);
            std::string startLabel = Strings::get(StringId::LOBBY_START_MATCH) + " (" + std::to_string(connectedPlayerCount_) + "/8)";
            drawTextString(shader, ortho, startX, startY, startLabel, 0.117f, 1.0f, 1.0f, 1.0f);
        }

        // SALIR DE SALA Button (Bottom Left)
        renderBackButton(shader, ortho, aspect);
    }

    void renderRoomListScreen(const Shader& shader, const float* ortho, float aspect) const {
        float cx = 0.0f, cy = -0.03f;
        float cardW = 1.45f, cardH = 0.85f;

        // Container card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.08f, 0.11f, 0.18f, 0.94f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.10f, 0.70f, 1.0f, 0.95f);

        // Title
        drawTextString(shader, ortho, cx, cy + 0.31f, Strings::get(StringId::AVAILABLE_ROOMS_TITLE), 0.135f, 0.20f, 0.90f, 1.0f);

        // Search / Filter Selector Box
        std::vector<std::string> filterNames = {
            "BUSCAR: " + Strings::get(StringId::FILTER_ALL),
            "BUSCAR: " + Strings::get(StringId::FILTER_PUBLIC),
            "BUSCAR: " + Strings::get(StringId::FILTER_PRIVATE)
        };
        float filterX = -0.15f, filterY = 0.20f, filterW = 0.70f, filterH = 0.10f;
        drawQuad(shader, ortho, filterX, filterY, filterW, filterH, 0.12f, 0.16f, 0.24f, 0.92f);
        drawTextString(shader, ortho, filterX, filterY, filterNames[filterTypeIndex_], 0.093f, 0.80f, 0.90f, 1.0f);

        // Scroll Up ▲ & Scroll Down ▼ Control Buttons
        float scrollX = 0.52f, scrollW = 0.22f, scrollH = 0.10f;
        float upY = 0.20f, downY = -0.22f;
        drawQuad(shader, ortho, scrollX, upY, scrollW, scrollH, 0.14f, 0.22f, 0.35f, 0.92f);
        drawTextString(shader, ortho, scrollX, upY, Strings::get(StringId::SCROLL_UP), 0.093f, 1.0f, 1.0f, 1.0f);

        drawQuad(shader, ortho, scrollX, downY, scrollW, scrollH, 0.14f, 0.22f, 0.35f, 0.92f);
        drawTextString(shader, ortho, scrollX, downY, Strings::get(StringId::SCROLL_DOWN), 0.093f, 1.0f, 1.0f, 1.0f);

        // Filter Rooms
        auto filtered = getFilteredRooms();

        if (filtered.empty()) {
            float emptyY = -0.04f;
            drawQuad(shader, ortho, 0.0f, emptyY, 1.15f, 0.24f, 0.12f, 0.15f, 0.22f, 0.92f);
            drawTextString(shader, ortho, 0.0f, emptyY + 0.04f, Strings::get(StringId::EMPTY_ROOMS_LINE1), 0.087f, 1.0f, 0.45f, 0.35f);
            drawTextString(shader, ortho, 0.0f, emptyY - 0.04f, Strings::get(StringId::EMPTY_ROOMS_LINE2), 0.081f, 0.60f, 0.80f, 1.0f);
        } else {
            // Render visible real room rows with scroll offset
            float startY = 0.06f;
            size_t maxVisible = 3;
            size_t startIndex = std::min(static_cast<size_t>(roomListScrollOffset_), filtered.size() - 1);

            for (size_t i = 0; i < maxVisible && (startIndex + i) < filtered.size(); ++i) {
                const auto& room = filtered[startIndex + i];
                float rowY = startY - static_cast<float>(i) * 0.14f;

                // Room info box
                drawQuad(shader, ortho, -0.15f, rowY, 0.80f, 0.12f, 0.13f, 0.17f, 0.25f, 0.92f);
                std::string typeTag = room.isPrivate ? Strings::get(StringId::FILTER_PRIVATE) : Strings::get(StringId::FILTER_PUBLIC);
                std::string label = room.name + " " + typeTag + " " + std::to_string(room.playerCount) + "/" + std::to_string(room.maxPlayers);

                float r = room.isPrivate ? 0.95f : 0.90f;
                float g = room.isPrivate ? 0.60f : 0.90f;
                float b = room.isPrivate ? 0.20f : 0.90f;
                drawTextString(shader, ortho, -0.15f, rowY, label, 0.087f, r, g, b);

                // Join button
                drawQuad(shader, ortho, 0.42f, rowY, 0.32f, 0.12f, 0.15f, 0.75f, 0.35f, 0.95f);
                std::string btnText = room.isPrivate ? ("PIN " + room.pin) : Strings::get(StringId::JOIN_ROOM);
                drawTextString(shader, ortho, 0.42f, rowY, btnText, 0.098f, 1.0f, 1.0f, 1.0f);
            }
        }

        // ACTUALIZAR Button
        float refX = 0.0f, refY = -0.38f, refW = 0.55f, refH = 0.12f;
        drawQuad(shader, ortho, refX, refY, refW, refH, 0.12f, 0.45f, 0.85f, 0.95f);
        drawTextString(shader, ortho, refX, refY, Strings::get(StringId::REFRESH), 0.102f, 1.0f, 1.0f, 1.0f);

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
        float cy = 0.05f;
        float cardW = 1.20f;
        float cardH = 0.75f;

        // Backdrop card
        drawQuad(shader, ortho, cx, cy, cardW, cardH, 0.09f, 0.12f, 0.17f, 0.95f);
        drawQuad(shader, ortho, cx, cy + cardH * 0.5f, cardW, 0.012f, 0.4f, 0.7f, 1.0f, 0.95f);
        drawQuad(shader, ortho, cx, cy - cardH * 0.5f, cardW, 0.012f, 0.4f, 0.7f, 1.0f, 0.95f);

        // Winner Text
        float textY = cy + 0.20f;
        if (redCount > blueCount) {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::RED_WINS), 0.150f, 1.0f, 0.25f, 0.25f);
        } else if (blueCount > redCount) {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::BLUE_WINS), 0.150f, 0.1f, 0.65f, 1.0f);
        } else {
            drawTextString(shader, ortho, cx, textY, Strings::get(StringId::DRAW), 0.155f, 1.0f, 0.9f, 0.2f);
        }

        // Final Score Summary Text: RED: X | BLUE: Y
        float scoreY = cy + 0.02f;
        drawDigits(shader, ortho, cx - 0.22f, scoreY, redCount, 1.0f, 0.35f, 0.35f);
        drawQuad(shader, ortho, cx, scoreY, 0.01f, 0.08f, 0.5f, 0.5f, 0.6f, 0.8f);
        drawDigits(shader, ortho, cx + 0.12f, scoreY, blueCount, 0.25f, 0.68f, 1.0f);

        // "PLAY AGAIN" Button
        float btnX = cx;
        float btnY = cy - 0.20f;
        float btnW = 0.55f;
        float btnH = 0.16f;

        drawQuad(shader, ortho, btnX, btnY, btnW, btnH, 0.12f, 0.45f, 0.90f, 0.95f);
        drawQuad(shader, ortho, btnX, btnY - btnH * 0.5f, btnW, 0.01f, 0.6f, 0.85f, 1.0f, 0.95f);
        drawTextString(shader, ortho, btnX, btnY, Strings::get(StringId::PLAY_AGAIN), 0.125f, 1.0f, 1.0f, 1.0f);
    }

    void renderTopScorePanels(const Shader& shader, const float* ortho, float aspect, int redCount, int blueCount) const {
        float panelY = 0.85f;
        float panelW = 0.52f;
        float panelH = 0.16f;

        // Top-Left Panel (RED Score)
        float redX = -aspect + panelW * 0.5f + 0.06f;
        drawQuad(shader, ortho, redX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, redX, panelY - panelH * 0.5f, panelW, 0.01f, 1.0f, 0.22f, 0.22f, 0.95f);
        draw3DMiniCube(shader, ortho, redX - panelW * 0.30f, panelY, 1.0f, 0.22f, 0.22f);
        drawDigits(shader, ortho, redX + 0.04f, panelY, redCount, 1.0f, 0.35f, 0.35f);

        // Reset Button (Left side, directly below RED Score Panel)
        float resetX = -aspect + 0.28f;
        float resetY = 0.65f;
        float resetW = 0.38f;
        float resetH = 0.14f;

        drawQuad(shader, ortho, resetX, resetY, resetW, resetH, 0.14f, 0.18f, 0.25f, 0.92f);
        drawQuad(shader, ortho, resetX, resetY - resetH * 0.5f, resetW, 0.01f, 0.4f, 0.7f, 1.0f, 0.95f);
        drawTextString(shader, ortho, resetX, resetY, Strings::get(StringId::EXIT), 0.110f, 0.9f, 0.95f, 1.0f);

        // Top-Right Panel (BLUE Score)
        float blueX = aspect - panelW * 0.5f - 0.06f;
        drawQuad(shader, ortho, blueX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, blueX, panelY - panelH * 0.5f, panelW, 0.01f, 0.05f, 0.55f, 1.0f, 0.95f);
        draw3DMiniCube(shader, ortho, blueX - panelW * 0.30f, panelY, 0.05f, 0.55f, 1.0f);
        drawDigits(shader, ortho, blueX + 0.04f, panelY, blueCount, 0.25f, 0.68f, 1.0f);
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
    std::string roomPin_;
    std::string roomName_;
    std::string userNickname_;
    bool isServerConnected_;
    int connectedPlayerCount_;
    size_t filterTypeIndex_;
    int roomListScrollOffset_;
    std::vector<ServerRoomEntry> serverRooms_;
    std::string joinNotificationText_;
    float joinNotificationTimer_;
    FontRenderer fontRenderer_;

    std::vector<Vertex> quadVertices_;
    std::vector<uint16_t> quadIndices_;
    std::vector<Vertex> circleVertices_;
    std::vector<uint16_t> circleIndices_;

    std::vector<Vertex> miniCubeVertices_;
    std::vector<uint16_t> miniCubeIndices_;
    std::vector<float> miniCubeShades_;
};

#endif // ANDROIDGLINVESTIGATIONS_GAMEUI_H