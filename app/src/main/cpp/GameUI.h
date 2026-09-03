#ifndef ANDROIDGLINVESTIGATIONS_GAMEUI_H
#define ANDROIDGLINVESTIGATIONS_GAMEUI_H

#include <string>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <android/asset_manager.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "Shader.h"
#include "FontRenderer.h"
#include "MatrixMath.h"
#include "AudioEngine.h"
#include "TextureAsset.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIBanner.h"
#include "UIDrawHelpers.h"
#include "UISetupScreen.h"
#include "UIWelcomeScreen.h"
#include "UIRoomListScreen.h"
#include "UICreateRoomScreen.h"
#include "UIRoomLobbyScreen.h"
#include "UILanguageScreen.h"
#include "UIInGameOverlay.h"
#include "GameUIStructs.h"
#include "UIScrollContainer.h"
#include "Strings.h"

enum class GameState {
    SETUP,
    WELCOME,
    MENU,
    LOBBY_SELECT,
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
    PLAY,
    SELECT_LANGUAGE_ES,
    SELECT_LANGUAGE_EN,
    OPEN_LANGUAGE_SETTINGS,
    CLOSE_LANGUAGE_SETTINGS,
    SETUP_SELECT_LANGUAGE,
    SETUP_CONTINUE
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
          joinNotificationTimer_(0.0f),
          showLeaveConfirmModal_(false),
          leaveModalFromMatch_(false),
          returnedToRoomFromMatch_(false),
          roomMatchActive_(false),
          pendingTeam_("BLUE"),
          myTeam_("BLUE"),
          pendingJoinRoom_(false),
          reconnectTimer_(0.0f),
          languagePanelOpen_(false),
          searchQuery_("") {
        initQuadGeometry();
        initMiniCubeGeometry();
        serverRooms_.clear();
    }

    GameState getState() const { return state_; }
    void setState(GameState state) { state_ = state; }

    float getMatchTimer() const { return matchTimer_; }
    void setMatchTimer(float t) { matchTimer_ = t; }

    bool isPrivateRoom() const { return isPrivateRoom_; }
    bool isOwner() const { return isOwner_; }
    const std::string& getRoomPin() const { return roomPin_; }
    static std::string sanitizeToUppercase(const std::string& str) {
        std::string out;
        out.reserve(str.size());
        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = static_cast<unsigned char>(str[i]);
            if (c < 0x80) {
                char ch = static_cast<char>(c);
                if (ch >= 'a' && ch <= 'z') {
                    out += static_cast<char>(ch - 'a' + 'A');
                } else {
                    out += ch;
                }
                i += 1;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
                unsigned char c2 = static_cast<unsigned char>(str[i + 1]);
                uint32_t cp = ((c & 0x1F) << 6) | (c2 & 0x3F);
                switch (cp) {
                    case 0x00E1: case 0x00E0: case 0x00E4: case 0x00E2:
                    case 0x00C1: case 0x00C0: case 0x00C4: case 0x00C2:
                        out += 'A'; break;
                    case 0x00E9: case 0x00E8: case 0x00EB: case 0x00EA:
                    case 0x00C9: case 0x00C8: case 0x00CB: case 0x00CA:
                        out += 'E'; break;
                    case 0x00ED: case 0x00EC: case 0x00EF: case 0x00EE:
                    case 0x00CD: case 0x00CC: case 0x00CF: case 0x00CE:
                        out += 'I'; break;
                    case 0x00F3: case 0x00F2: case 0x00F6: case 0x00F4:
                    case 0x00D3: case 0x00D2: case 0x00D6: case 0x00D4:
                        out += 'O'; break;
                    case 0x00FA: case 0x00F9: case 0x00FC: case 0x00FB:
                    case 0x00DA: case 0x00D9: case 0x00DC: case 0x00DB:
                        out += 'U'; break;
                    case 0x00F1: case 0x00D1:
                        out += "\xC3\x91"; break; // Ñ
                    default:
                        out += static_cast<char>(c);
                        out += static_cast<char>(c2);
                        break;
                }
                i += 2;
            } else {
                out += static_cast<char>(c);
                i += 1;
            }
        }
        return out;
    }

    const std::string& getRoomName() const {
        std::string cleanNick = sanitizeToUppercase(userNickname_);
        if (roomName_.empty() || roomName_ == "Lobby" || roomName_ == "LOBBY" || roomName_ == "Lobb") {
            static std::string defaultName;
            defaultName = cleanNick.empty() ? "PLAYER_LOBBY" : (cleanNick + "_LOBBY");
            return defaultName;
        }
        static std::string sanitizedRoomName;
        sanitizedRoomName = sanitizeToUppercase(roomName_);
        return sanitizedRoomName;
    }
    const std::string& getSelectedRoomId() const { return selectedRoomId_; }
    const std::string& getUserNickname() const {
        static std::string cleanNick;
        cleanNick = sanitizeToUppercase(userNickname_);
        return cleanNick;
    }
    bool isServerConnected() const { return isServerConnected_; }
    void setServerConnected(bool connected) { isServerConnected_ = connected; }

    int getConnectedPlayerCount() const { return connectedPlayerCount_; }
    void setConnectedPlayerCount(int count) {
        int prevCount = connectedPlayerCount_;
        connectedPlayerCount_ = std::clamp(count, 1, 8);
        if (connectedPlayerCount_ > prevCount) {
            std::string notif = Strings::get(StringId::PLAYER_JOINED_NOTIF);
            size_t at = notif.find('@');
            if (at != std::string::npos) {
                notif.replace(at, 1, Strings::getLanguage() == Language::SPANISH
                ? "JUGADOR " + std::to_string(connectedPlayerCount_)
                : "PLAYER " + std::to_string(connectedPlayerCount_));
            }
            joinNotificationText_ = notif;
            joinNotificationTimer_ = 3.5f;
        }
    }

    void initFont(AAssetManager* assetManager, const std::string& fontPath = "press_start_2p.ttf") {
        fontRenderer_.loadFont(assetManager, fontPath, 128.0f);
    }

    void initCursorSprites(AAssetManager* assetManager) {
        arrowRedLeftTexture_ = TextureAsset::loadAsset(assetManager, "arrow_red_left.png");
        arrowBlueLeftTexture_ = TextureAsset::loadAsset(assetManager, "arrow_blue_left.png");
        arrowBlueRightTexture_ = TextureAsset::loadAsset(assetManager, "arrow_blue_right.png");
        arrowRedRightTexture_ = TextureAsset::loadAsset(assetManager, "arrow_red_right.png");
        flagEsTexture_ = TextureAsset::loadAsset(assetManager, "flag_es.png");
        flagUsTexture_ = TextureAsset::loadAsset(assetManager, "flag_us.png");
        gearTexture_ = TextureAsset::loadAsset(assetManager, "gear_icon.png");
    }

    void setLanguageFromNative(int language) {
        Strings::setLanguage(language == 0 ? Language::SPANISH : Language::ENGLISH);
    }

    void beginFirstRunSetup() {
        if (state_ == GameState::SETUP) return;
        state_ = GameState::SETUP;
        languagePanelOpen_ = false;
    }

    void finishFirstRunSetup() {
        if (state_ == GameState::SETUP) {
            state_ = GameState::WELCOME;
        }
        languagePanelOpen_ = false;
    }

    static std::string fillTemplate(const std::string& tpl, const std::string& value) {
        std::string out = tpl;
        size_t at = out.find('@');
        if (at != std::string::npos) out.replace(at, 1, value);
        return out;
    }

    void triggerJoinNotification(const std::string& playerName) {
        joinNotificationText_ = fillTemplate(Strings::get(StringId::PLAYER_JOINED_NOTIF), playerName);
        joinNotificationTimer_ = 3.5f;
    }

    void triggerWelcomeNotification(const std::string& nickname) {
        joinNotificationText_ = fillTemplate(Strings::get(StringId::WELCOME_BACK_NOTIF), nickname);
        joinNotificationTimer_ = 4.0f;
    }

    void onTextInputResult(int fieldType, const std::string& text) {
        if (fieldType == 4) {
            searchQuery_ = sanitizeToUppercase(text);
            scrollContainer_.resetScroll();
            return;
        }
        if (text.empty()) return;
        std::string sanitized = sanitizeToUppercase(text);
        if (fieldType == 0) {
            userNickname_ = sanitized;
            if (roomName_.empty() || roomName_ == "Lobby" || roomName_ == "LOBBY" || roomName_.find("_LOBBY") != std::string::npos || roomName_.find("_Lobby") != std::string::npos) {
                roomName_ = userNickname_ + "_LOBBY";
            }
            triggerWelcomeNotification(userNickname_);
        } else if (fieldType == 1) {
            roomName_ = sanitized;
        } else if (fieldType == 2) {
            roomPin_ = sanitized;
        } else if (fieldType == 3) {
            roomPin_ = sanitized;
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
        scrollContainer_.resetScroll();
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
        scrollContainer_.resetScroll();
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

        if (connectedPlayerCount_ > 0) {
            if (actualCount < connectedPlayerCount_) {
                joinNotificationText_ = Strings::get(StringId::PLAYER_LEFT);
                joinNotificationTimer_ = 3.5f;
            } else if (actualCount > connectedPlayerCount_) {
                joinNotificationText_ = Strings::get(StringId::PLAYER_JOINED);
                joinNotificationTimer_ = 3.5f;
            }
        }

        if (!isOwner_ && isOwner) {
            joinNotificationText_ = Strings::get(StringId::NOW_OWNER);
            joinNotificationTimer_ = 4.0f;
        }

        connectedPlayerCount_ = actualCount;
        isOwner_ = isOwner;
    }

    void onGameAborted(const std::string& reason) {
        state_ = GameState::ROOM_LOBBY;
        roomMatchActive_ = false;
        returnedToRoomFromMatch_ = false;
        joinNotificationText_ = Strings::get(StringId::MATCH_OVER_REASON) + reason;
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

    void handleMotionEvent(int actionMasked, float screenX, float screenY, float width, float height) {
        if (state_ != GameState::ROOM_LIST) return;

        float aspect = width / height;
        float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
        float normY = 1.0f - (2.0f * screenY) / height;

        float cardW = std::min(2.70f, aspect * 1.85f);
        float boxX = -0.02f;
        float boxY = -0.09f;
        float boxW = cardW - 0.32f;
        float boxH = 0.66f;

        auto filtered = getFilteredRooms();
        float contentH = static_cast<float>(filtered.size()) * 0.15f;
        scrollContainer_.setContentAndViewportHeight(contentH, boxH);

        if (actionMasked == 0 /* AMOTION_EVENT_ACTION_DOWN */) {
            scrollContainer_.onTouchDown(normX, normY, boxX, boxY, boxW, boxH);
        } else if (actionMasked == 2 /* AMOTION_EVENT_ACTION_MOVE */) {
            scrollContainer_.onTouchMove(normX, normY);
        } else if (actionMasked == 1 /* AMOTION_EVENT_ACTION_UP */) {
            scrollContainer_.onTouchUp();
        }
    }

    TouchAction handleTouch(float screenX, float screenY, float width, float height, AudioEngine* audioEngine = nullptr, android_app* app = nullptr) {
        float aspect = width / height;
        float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
        float normY = 1.0f - (2.0f * screenY) / height;

        // 0a. Language picker panel is modal: intercept every tap while open.
        if (languagePanelOpen_) {
            float flagY = 0.18f, flagW = 0.55f, flagH = 0.34f;
            float esX = -0.35f, usX = 0.35f;
            if (std::abs(normX - esX) <= flagW * 0.5f + 0.04f &&
                std::abs(normY - flagY) <= flagH * 0.5f + 0.04f) {
                Strings::setLanguage(Language::SPANISH);
                persistLanguageToKotlin(app);
                languagePanelOpen_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SELECT_LANGUAGE_ES;
            }
            if (std::abs(normX - usX) <= flagW * 0.5f + 0.04f &&
                std::abs(normY - flagY) <= flagH * 0.5f + 0.04f) {
                Strings::setLanguage(Language::ENGLISH);
                persistLanguageToKotlin(app);
                languagePanelOpen_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SELECT_LANGUAGE_EN;
            }
            languagePanelOpen_ = false;
            if (audioEngine) audioEngine->playCountdownBeep();
            return TouchAction::CLOSE_LANGUAGE_SETTINGS;
        }

        // 0. Modal Confirmation Touch handling
        if (showLeaveConfirmModal_) {
            float yesX = -0.38f, noX = 0.38f, btnY = -0.14f, btnW = 0.62f, btnH = 0.18f;

            if (UIButton::contains(normX, normY, yesX, btnY, btnW, btnH)) {
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
            if (UIButton::contains(normX, normY, noX, btnY, btnW, btnH)) {
                showLeaveConfirmModal_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
            return TouchAction::NONE;
        }

        // 1. In-Game Leave check
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            if (UIDrawHelpers::isBackButtonClicked(normX, normY, aspect)) {
                showLeaveConfirmModal_ = true;
                leaveModalFromMatch_ = true;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
        }

        // 2. FIRST-RUN SETUP Screen Touch
        if (state_ == GameState::SETUP) {
            float flagY = 0.18f, flagW = 0.55f, flagH = 0.34f;
            float esX = -0.35f, usX = 0.35f;
            if (std::abs(normX - esX) <= flagW * 0.5f + 0.05f &&
                std::abs(normY - flagY) <= flagH * 0.5f + 0.05f) {
                Strings::setLanguage(Language::SPANISH);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SETUP_SELECT_LANGUAGE;
            }
            if (std::abs(normX - usX) <= flagW * 0.5f + 0.05f &&
                std::abs(normY - flagY) <= flagH * 0.5f + 0.05f) {
                Strings::setLanguage(Language::ENGLISH);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SETUP_SELECT_LANGUAGE;
            }

            float nickY = -0.22f, nickW = 1.1f, nickH = 0.14f;
            if (std::abs(normX) <= nickW * 0.5f + 0.04f &&
                std::abs(normY - nickY) <= nickH * 0.5f + 0.04f) {
                triggerNativeTextInput(app, 0, Strings::get(StringId::PROMPT_ENTER_NICKNAME), userNickname_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_NICKNAME;
            }

            float contY = -0.52f, contW = 1.1f, contH = 0.15f;
            if (std::abs(normX) <= contW * 0.5f + 0.04f &&
                std::abs(normY - contY) <= contH * 0.5f + 0.04f) {
                if (userNickname_.empty()) {
                    triggerNativeTextInput(app, 0, Strings::get(StringId::PROMPT_ENTER_NICKNAME), "");
                } else {
                    persistLanguageToKotlin(app);
                    finishFirstRunSetup();
                }
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::SETUP_CONTINUE;
            }
            return TouchAction::NONE;
        }

        // 3. MAIN MENU / WELCOME Screen Touch
        if (state_ == GameState::WELCOME) {
            float testX = -aspect + 0.38f, testY = 0.85f, testW = 0.58f, testH = 0.13f;
            if (UIButton::contains(normX, normY, testX, testY, testW, testH)) {
                connectedPlayerCount_ = 2;
                startCountdown(audioEngine, app);
                return TouchAction::START_LOCAL_GAME;
            }

            float gearX = -aspect + 0.16f, gearY = 0.68f, gearW = 0.16f, gearH = 0.16f;
            if (UIButton::contains(normX, normY, gearX, gearY, gearW, gearH)) {
                languagePanelOpen_ = true;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::OPEN_LANGUAGE_SETTINGS;
            }

            float btnY = -0.12f, btnW = 0.78f, btnH = 0.22f;
            float leftX = -0.42f;
            if (UIButton::contains(normX, normY, leftX, btnY, btnW, btnH)) {
                state_ = GameState::ROOM_LIST;
                scrollContainer_.resetScroll();
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::GOTO_ROOM_LIST;
            }

            float rightX = 0.42f;
            if (UIButton::contains(normX, normY, rightX, btnY, btnW, btnH)) {
                state_ = GameState::CREATE_ROOM;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::GOTO_CREATE_ROOM;
            }
        }
        // 4. CREATE ROOM Screen Touch
        else if (state_ == GameState::CREATE_ROOM) {
            const float cardW = 2.05f, margin = 0.14f;
            const float boxW = cardW - margin * 2.0f;
            const float fieldH = 0.17f;
            float boxX = 0.0f;
            float nameY = 0.38f;
            float togY = 0.18f;
            float pinY = -0.02f;
            const float pad = 0.05f;

            if (std::abs(normX - boxX) <= boxW * 0.5f + pad &&
                std::abs(normY - nameY) <= fieldH * 0.5f + pad) {
                triggerNativeTextInput(app, 1, Strings::get(StringId::PROMPT_ROOM_NAME), roomName_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_ROOM_NAME;
            }

            if (std::abs(normX - boxX) <= boxW * 0.5f + pad &&
                std::abs(normY - togY) <= fieldH * 0.5f + pad) {
                isPrivateRoom_ = !isPrivateRoom_;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::TOGGLE_PRIVACY;
            }

            if (isPrivateRoom_ &&
                std::abs(normX - boxX) <= boxW * 0.5f + pad &&
                std::abs(normY - pinY) <= fieldH * 0.5f + pad) {
                triggerNativeTextInput(app, 2, Strings::get(StringId::PROMPT_ROOM_PIN), roomPin_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_PIN;
            }

            float createX = 0.0f, createY = -0.54f, createW = boxW, createH = 0.18f;
            if (std::abs(normX - createX) <= createW * 0.5f + pad &&
                std::abs(normY - createY) <= createH * 0.5f + pad) {
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::CONFIRM_CREATE_ROOM;
            }

            if (UIDrawHelpers::isBackButtonClicked(normX, normY, aspect)) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::BACK_TO_WELCOME;
            }
        }
        // 5. ROOM LOBBY Screen Touch
        else if (state_ == GameState::ROOM_LOBBY) {
            float lobbyCardW = std::min(2.65f, aspect * 1.85f);
            float startX = 0.0f, startY = -0.54f, startW = 1.15f, startH = 0.18f;

            bool hasBlue = false, hasRed = false;
            for (const auto& player : currentRoomPlayers_) {
                if (player.team == "BLUE") hasBlue = true;
                if (player.team == "RED") hasRed = true;
            }
            bool hasOpposing = hasBlue && hasRed;

            if (std::abs(normX - startX) <= startW * 0.5f + 0.25f &&
                std::abs(normY - startY) <= startH * 0.5f + 0.04f) {
                if (isOwner_ && !roomMatchActive_ && connectedPlayerCount_ >= 2 && hasOpposing) {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::START_MULTIPLAYER_GAME;
                } else {
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::NONE;
                }
            }

            float leftX = -lobbyCardW * 0.25f;
            float rightX = lobbyCardW * 0.25f;
            float teamBoxW = std::min(1.15f, lobbyCardW * 0.44f);
            float rowStartY = 0.18f;
            float rowStep = 0.15f;
            size_t blueIndex = 0;
            size_t redIndex = 0;
            for (const auto& player : currentRoomPlayers_) {
                bool isRed = player.team == "RED";
                size_t rowIndex = isRed ? redIndex++ : blueIndex++;
                float rowY = rowStartY - static_cast<float>(rowIndex) * rowStep;
                float arrowX = isRed ? rightX - teamBoxW * 0.5f + 0.10f : leftX + teamBoxW * 0.5f - 0.10f;
                float arrowY = rowY - 0.005f;
                if (player.isLocal && !roomMatchActive_ && std::abs(normX - arrowX) <= 0.10f && std::abs(normY - arrowY) <= 0.08f) {
                    pendingTeam_ = isRed ? "BLUE" : "RED";
                    if (audioEngine) audioEngine->playCountdownBeep();
                    return TouchAction::CHANGE_TEAM;
                }
            }

            if (UIDrawHelpers::isBackButtonClicked(normX, normY, aspect)) {
                showLeaveConfirmModal_ = true;
                leaveModalFromMatch_ = false;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::NONE;
            }
        }
        // 6. ROOM LIST Screen Touch
        else if (state_ == GameState::ROOM_LIST) {
            float cardW = std::min(2.70f, aspect * 1.85f);

            // If user was dragging/swiping the list, do not click items
            if (scrollContainer_.wasDragged()) {
                return TouchAction::NONE;
            }

            // 1. Toolbar Search Input Field Box (Left)
            const float toolY = 0.48f, toolH = 0.12f;
            const float searchW = cardW * 0.48f;
            const float searchX = -0.02f - (cardW * 0.5f) + (searchW * 0.5f) + 0.08f;

            if (UIButton::contains(normX, normY, searchX, toolY, searchW, toolH)) {
                triggerNativeTextInput(app, 4, Strings::get(StringId::SEARCH_ROOMS), searchQuery_);
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_ROOM_NAME;
            }

            // 1b. Clear Search Button [X] (Middle)
            const float clearW = 0.13f;
            const float clearX = searchX + (searchW * 0.5f) + (clearW * 0.5f) + 0.02f;

            if (UIButton::contains(normX, normY, clearX, toolY, clearW, toolH)) {
                searchQuery_ = "";
                scrollContainer_.resetScroll();
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::EDIT_ROOM_NAME;
            }

            // 2. Toolbar Filter Selector [ALL, PUBLIC, PRIVATE] (Right)
            const float filterW = cardW * 0.28f;
            const float filterX = -0.02f + (cardW * 0.5f) - (filterW * 0.5f) - 0.08f;

            if (UIButton::contains(normX, normY, filterX, toolY, filterW, toolH)) {
                filterTypeIndex_ = (filterTypeIndex_ + 1) % 3;
                scrollContainer_.resetScroll();
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::CYCLE_FILTER;
            }

            // 3. Room List Items Click Handling
            const float boxX = -0.02f;
            const float boxY = -0.09f;
            const float boxW = cardW - 0.32f;
            const float boxH = 0.66f;

            if (normY >= boxY - boxH * 0.5f && normY <= boxY + boxH * 0.5f) {
                auto filtered = getFilteredRooms();
                const float itemH = 0.13f;
                const float rowStep = 0.15f;
                const float scrollOffset = scrollContainer_.getScrollOffset();

                for (size_t i = 0; i < filtered.size(); ++i) {
                    const auto& room = filtered[i];
                    float rowY = (boxY + boxH * 0.5f - itemH * 0.5f - 0.015f) - (static_cast<float>(i) * rowStep) + scrollOffset;

                    if (rowY + itemH * 0.5f < boxY - boxH * 0.5f || rowY - itemH * 0.5f > boxY + boxH * 0.5f) {
                        continue;
                    }

                    if (UIButton::contains(normX, normY, boxX, rowY, boxW, itemH)) {
                        // If FULL (playerCount >= maxPlayers), ignore click
                        if (room.playerCount >= room.maxPlayers) {
                            if (audioEngine) audioEngine->playCountdownBeep();
                            return TouchAction::NONE;
                        }

                        selectedRoomId_ = room.id;
                        roomName_ = room.name;
                        isPrivateRoom_ = room.isPrivate;

                        if (room.isPrivate) {
                            triggerNativeTextInput(app, 3,
                                fillTemplate(Strings::get(StringId::PRIVATE_ROOM_PIN_PROMPT), room.name) + ":",
                                "");
                            if (audioEngine) audioEngine->playCountdownBeep();
                            return TouchAction::EDIT_PIN;
                        } else {
                            roomPin_ = "";
                            if (audioEngine) audioEngine->playCountdownBeep();
                            return TouchAction::JOIN_SELECTED_ROOM;
                        }
                    }
                }
            }

            // 4. REFRESH Button
            float refX = 0.0f, refY = -0.52f, refW = 0.85f, refH = 0.15f;
            if (UIButton::contains(normX, normY, refX, refY, refW, refH)) {
                scrollContainer_.resetScroll();
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::REFRESH_ROOMS;
            }

            // 5. VOLVER Button
            if (UIDrawHelpers::isBackButtonClicked(normX, normY, aspect)) {
                state_ = GameState::WELCOME;
                if (audioEngine) audioEngine->playCountdownBeep();
                return TouchAction::BACK_TO_WELCOME;
            }
        }
        // 7. MATCH OVER Touch
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

        // 1. Top Score Panels, Timer & In-Game Leave Button
        if (state_ == GameState::COUNTDOWN || state_ == GameState::PLAYING || state_ == GameState::MATCH_OVER) {
            UIInGameOverlay::renderTopScorePanels(shader, ortho, fontRenderer_, aspect, myTeam_, redCount, blueCount);
            UIInGameOverlay::renderMatchTimerDisplay(shader, ortho, fontRenderer_, matchTimer_);
            UIInGameOverlay::renderMatchPlayersPanel(shader, ortho, fontRenderer_, aspect, myTeam_, currentRoomPlayers_);
            if (state_ == GameState::PLAYING || state_ == GameState::COUNTDOWN || state_ == GameState::MATCH_OVER) {
                UIDrawHelpers::renderBackButton(shader, ortho, fontRenderer_, aspect);
            }
        } else {
            UIBanner::renderServerStatus(shader, ortho, fontRenderer_, isServerConnected_);
            UIBanner::renderNicknameBadge(shader, ortho, fontRenderer_, aspect, userNickname_);

            if (joinNotificationTimer_ > 0.0f) {
                UIBanner::renderNotification(shader, ortho, fontRenderer_, joinNotificationText_);
            }
        }

        // 2. Render State Specific UI Screens
        if (state_ == GameState::SETUP) {
            UISetupScreen::render(shader, ortho, aspect, fontRenderer_, userNickname_, flagEsTexture_, flagUsTexture_);
        } else if (state_ == GameState::WELCOME) {
            UIWelcomeScreen::render(shader, ortho, aspect, fontRenderer_, gearTexture_);
        } else if (state_ == GameState::CREATE_ROOM) {
            UICreateRoomScreen::render(shader, ortho, aspect, fontRenderer_, getRoomName(), isPrivateRoom_, roomPin_, userNickname_);
        } else if (state_ == GameState::ROOM_LOBBY) {
            UIRoomLobbyScreen::render(shader, ortho, aspect, fontRenderer_, roomName_, isPrivateRoom_,
                                     connectedPlayerCount_, isOwner_, roomMatchActive_, userNickname_,
                                     currentRoomPlayers_, arrowBlueLeftTexture_, arrowBlueRightTexture_,
                                     arrowRedLeftTexture_, arrowRedRightTexture_);
        } else if (state_ == GameState::ROOM_LIST) {
            UIRoomListScreen::render(shader, ortho, aspect, width, height, fontRenderer_, filterTypeIndex_, searchQuery_, scrollContainer_, getFilteredRooms());
        } else if (state_ == GameState::COUNTDOWN) {
            UIInGameOverlay::renderCountdown(shader, ortho, fontRenderer_, countdownTimer_);
        } else if (state_ == GameState::MATCH_OVER) {
            UIInGameOverlay::renderWinnerOverlay(shader, ortho, fontRenderer_, redCount, blueCount, myTeam_, isOwner_);
        }

        // 3. Render Modal Confirm Overlay if active
        if (showLeaveConfirmModal_) {
            UIInGameOverlay::renderLeaveConfirmModal(shader, ortho, fontRenderer_, aspect, showLeaveConfirmModal_);
        } else if (state_ == GameState::PLAYING && !isServerConnected_) {
            UIInGameOverlay::renderReconnectingOverlay(shader, ortho, fontRenderer_, aspect, reconnectTimer_);
        }

        // 4. Language picker panel
        if (languagePanelOpen_) {
            UILanguageScreen::render(shader, ortho, aspect, fontRenderer_, flagEsTexture_, flagUsTexture_);
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

    static void persistLanguageToKotlin(android_app* app) {
        if (!app || !app->activity || !app->activity->vm) return;
        JavaVM* vm = app->activity->vm;
        JNIEnv* env = nullptr;
        if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            if (vm->AttachCurrentThread(&env, nullptr) != 0) return;
        }
        jobject activityObj = app->activity->javaGameActivity;
        jclass clazz = env->GetObjectClass(activityObj);
        jmethodID saveMethod = env->GetMethodID(clazz, "saveLanguagePref", "(Ljava/lang/String;)V");
        if (saveMethod) {
            const char* code = (Strings::getLanguage() == Language::ENGLISH) ? "en" : "es";
            jstring langCode = env->NewStringUTF(code);
            env->CallVoidMethod(activityObj, saveMethod, langCode);
            env->DeleteLocalRef(langCode);
        }
        env->DeleteLocalRef(clazz);
    }

    [[nodiscard]] std::vector<ServerRoomEntry> getFilteredRooms() const {
        std::vector<ServerRoomEntry> res;
        std::string queryUpper = searchQuery_;
        std::transform(queryUpper.begin(), queryUpper.end(), queryUpper.begin(), ::toupper);

        for (const auto& room : serverRooms_) {
            if (filterTypeIndex_ == 1 && room.isPrivate) continue;
            if (filterTypeIndex_ == 2 && !room.isPrivate) continue;

            if (!queryUpper.empty()) {
                std::string roomNameUpper = room.name;
                std::transform(roomNameUpper.begin(), roomNameUpper.end(), roomNameUpper.begin(), ::toupper);
                if (roomNameUpper.find(queryUpper) == std::string::npos) {
                    continue;
                }
            }
            res.push_back(room);
        }
        return res;
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
            float angle = (static_cast<float>(i) * 2.0f * 3.14159265f) / static_cast<float>(SEGMENTS);
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
    bool languagePanelOpen_;
    std::string searchQuery_;
    UIScrollContainer scrollContainer_;

    FontRenderer fontRenderer_;
    std::shared_ptr<TextureAsset> arrowRedLeftTexture_;
    std::shared_ptr<TextureAsset> arrowBlueLeftTexture_;
    std::shared_ptr<TextureAsset> arrowBlueRightTexture_;
    std::shared_ptr<TextureAsset> arrowRedRightTexture_;
    std::shared_ptr<TextureAsset> flagEsTexture_;
    std::shared_ptr<TextureAsset> flagUsTexture_;
    std::shared_ptr<TextureAsset> gearTexture_;

    std::vector<Vertex> quadVertices_;
    std::vector<uint16_t> quadIndices_;
    std::vector<Vertex> circleVertices_;
    std::vector<uint16_t> circleIndices_;

    std::vector<Vertex> miniCubeVertices_;
    std::vector<uint16_t> miniCubeIndices_;
    std::vector<float> miniCubeShades_;
};

#endif // ANDROIDGLINVESTIGATIONS_GAMEUI_H
