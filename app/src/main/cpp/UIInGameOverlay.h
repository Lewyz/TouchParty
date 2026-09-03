#ifndef TOUCHPARTY_UIINGAMEOVERLAY_H
#define TOUCHPARTY_UIINGAMEOVERLAY_H

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "GameUIStructs.h"
#include "Strings.h"

class UIInGameOverlay {
public:
    static void renderMatchTimerDisplay(const Shader& shader, const float* ortho,
                                        const FontRenderer& fontRenderer, float matchTimer) {
        float timerX = 0.0f, timerY = 0.85f, timerW = 0.44f, timerH = 0.16f;

        UIDrawHelpers::drawQuad(shader, ortho, timerX, timerY, timerW, timerH, 0.10f, 0.13f, 0.20f, 0.92f);

        float r = 0.1f, g = 0.9f, b = 1.0f;
        if (matchTimer <= 15.0f) {
            r = 1.0f; g = 0.3f; b = 0.2f;
        }

        UIDrawHelpers::drawQuad(shader, ortho, timerX, timerY - timerH * 0.5f, timerW, 0.01f, r, g, b, 0.95f);

        int totalSec = std::max(0, static_cast<int>(std::ceil(matchTimer)));
        int mins = totalSec / 60;
        int secs = totalSec % 60;
        std::string timeStr = (mins < 10 ? "0" : "") + std::to_string(mins) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);

        if (fontRenderer.isLoaded()) {
            fontRenderer.drawText(shader, ortho, timerX, timerY, timeStr, 0.135f, r, g, b, 1.0f, true);
        }
    }

    static void renderWinnerOverlay(const Shader& shader, const float* ortho,
                                    const FontRenderer& fontRenderer,
                                    int redCount, int blueCount,
                                    const std::string& myTeam, bool isOwner) {
        float cx = 0.0f, cy = 0.03f, cardW = 1.35f, cardH = 0.82f;

        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.showUnderline = false;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        float statusY = cy + 0.28f;
        bool isMyTeamBlue = myTeam != "RED";
        bool isBlueWinner = blueCount > redCount;
        bool isRedWinner = redCount > blueCount;
        bool isTie = (redCount == blueCount);

        if (isTie) {
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, statusY, Strings::get(StringId::TIE_GAME), 0.135f, cardW - 0.24f, 1.0f, 0.90f, 0.20f);
        } else {
            bool iWon = (isMyTeamBlue && isBlueWinner) || (!isMyTeamBlue && isRedWinner);
            if (iWon) {
                UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, statusY, Strings::get(StringId::YOU_WON), 0.145f, cardW - 0.24f, 0.20f, 1.0f, 0.40f);
            } else {
                UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, statusY, Strings::get(StringId::YOU_LOST), 0.145f, cardW - 0.24f, 1.0f, 0.30f, 0.30f);
            }
        }

        float textY = cy + 0.12f;
        if (redCount > blueCount) {
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, textY, Strings::get(StringId::RED_WINS), 0.112f, cardW - 0.24f, 1.0f, 0.25f, 0.25f);
        } else if (blueCount > redCount) {
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, textY, Strings::get(StringId::BLUE_WINS), 0.112f, cardW - 0.24f, 0.1f, 0.65f, 1.0f);
        } else {
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, textY, Strings::get(StringId::DRAW), 0.112f, cardW - 0.24f, 1.0f, 0.9f, 0.2f);
        }

        float scoreY = cy - 0.05f;
        if (fontRenderer.isLoaded()) {
            fontRenderer.drawText(shader, ortho, cx - 0.22f, scoreY, std::to_string(redCount), 0.14f, 1.0f, 0.35f, 0.35f, 1.0f, false);
            UIDrawHelpers::drawQuad(shader, ortho, cx, scoreY, 0.01f, 0.08f, 0.5f, 0.5f, 0.6f, 0.8f);
            fontRenderer.drawText(shader, ortho, cx + 0.12f, scoreY, std::to_string(blueCount), 0.14f, 0.25f, 0.68f, 1.0f, 1.0f, false);
        }

        float btnX = cx, btnY = cy - 0.24f, btnW = 0.62f, btnH = 0.15f;
        UIButtonSpec playAgainSpec;
        playAgainSpec.x = btnX;
        playAgainSpec.y = btnY;
        playAgainSpec.w = btnW;
        playAgainSpec.h = btnH;

        if (isOwner) {
            playAgainSpec.text = Strings::get(StringId::PLAY_AGAIN);
            playAgainSpec.bgColor = UITheme::BTN_SUCCESS_BG;
            playAgainSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
            playAgainSpec.textColor = UITheme::TEXT_WHITE;
            playAgainSpec.fontSize = 0.108f;
        } else {
            playAgainSpec.text = Strings::get(StringId::WAITING_LEADER);
            playAgainSpec.bgColor = ColorRGBA(0.14f, 0.18f, 0.25f, 0.92f);
            playAgainSpec.borderColor = UITheme::CARD_BORDER_CYAN;
            playAgainSpec.textColor = ColorRGBA(0.80f, 0.90f, 1.0f, 1.0f);
            playAgainSpec.fontSize = 0.092f;
            playAgainSpec.isClickable = false;
        }
        UIButton::render(shader, ortho, fontRenderer, playAgainSpec);
    }

    static void renderLeaveConfirmModal(const Shader& shader, const float* ortho,
                                        const FontRenderer& fontRenderer,
                                        float aspect, bool showModal) {
        if (!showModal) return;

        UIDrawHelpers::drawQuad(shader, ortho, 0.0f, 0.0f, aspect * 2.1f, 2.1f, 0.0f, 0.0f, 0.0f, 0.80f);

        float cx = 0.0f, cy = 0.02f, cardW = 1.70f, cardH = 0.68f;
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::LEAVE_CONFIRM_QUESTION);
        cardSpec.titleFontSize = 0.095f;
        cardSpec.titleOffsetY = 0.18f;
        cardSpec.showUnderline = false;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        float yesX = -0.38f, noX = 0.38f, btnY = cy - 0.14f, btnW = 0.62f, btnH = 0.18f;

        UIButtonSpec yesSpec;
        yesSpec.x = yesX;
        yesSpec.y = btnY;
        yesSpec.w = btnW;
        yesSpec.h = btnH;
        yesSpec.text = Strings::get(StringId::YES_LEAVE);
        yesSpec.bgColor = ColorRGBA(0.85f, 0.25f, 0.20f, 0.95f);
        yesSpec.borderColor = ColorRGBA(0.95f, 0.40f, 0.30f, 0.95f);
        yesSpec.textColor = UITheme::TEXT_WHITE;
        yesSpec.fontSize = 0.095f;
        UIButton::render(shader, ortho, fontRenderer, yesSpec);

        UIButtonSpec noSpec;
        noSpec.x = noX;
        noSpec.y = btnY;
        noSpec.w = btnW;
        noSpec.h = btnH;
        noSpec.text = Strings::get(StringId::CANCEL);
        noSpec.bgColor = UITheme::BTN_SUCCESS_BG;
        noSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
        noSpec.textColor = UITheme::TEXT_WHITE;
        noSpec.fontSize = 0.095f;
        UIButton::render(shader, ortho, fontRenderer, noSpec);
    }

    static void renderReconnectingOverlay(const Shader& shader, const float* ortho,
                                           const FontRenderer& fontRenderer,
                                           float aspect, float reconnectTimer) {
        float banX = 0.0f, banY = 0.0f, banW = 1.65f, banH = 0.35f;
        UIDrawHelpers::drawQuad(shader, ortho, banX, banY, banW, banH, 0.12f, 0.14f, 0.22f, 0.94f);
        UIDrawHelpers::drawQuad(shader, ortho, banX, banY - banH * 0.5f, banW, 0.01f, 1.0f, 0.60f, 0.10f, 0.95f);
        int secsLeft = std::max(0, 15 - static_cast<int>(reconnectTimer));
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, banX, banY,
                                     Strings::get(StringId::RECONNECTING) + " (" + std::to_string(secsLeft) + "s)",
                                     0.105f, banW - 0.20f, 1.0f, 0.85f, 0.20f);
    }

    static void renderMatchPlayersPanel(const Shader& shader, const float* ortho,
                                        const FontRenderer& fontRenderer,
                                        float aspect, const std::string& myTeam,
                                        const std::vector<PlayerInfo>& currentRoomPlayers) {
        const float panelW = 0.72f, rowH = 0.095f, headerH = 0.105f, startY = 0.53f;
        const float leftX = -aspect + panelW * 0.5f + 0.06f;
        const float rightX = aspect - panelW * 0.5f - 0.06f;

        auto drawTeamPanel = [&](const std::string& team, float x, float r, float g, float b) {
            UIDrawHelpers::drawQuad(shader, ortho, x, startY, panelW, 0.025f, r, g, b, 0.95f);

            size_t rowIndex = 0;
            for (const auto& player : currentRoomPlayers) {
                if (player.team != team || rowIndex >= 4) continue;
                float rowY = startY - headerH * 0.5f - rowH * 0.75f - static_cast<float>(rowIndex) * rowH;
                UIDrawHelpers::drawQuad(shader, ortho, x, rowY, panelW, rowH, 0.08f, 0.14f, 0.22f, 0.90f);
                if (player.isOwner) {
                    const float border = 0.006f;
                    UIDrawHelpers::drawQuad(shader, ortho, x, rowY + rowH * 0.5f, panelW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    UIDrawHelpers::drawQuad(shader, ortho, x, rowY - rowH * 0.5f, panelW, border, 1.0f, 0.85f, 0.10f, 1.0f);
                    UIDrawHelpers::drawQuad(shader, ortho, x - panelW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                    UIDrawHelpers::drawQuad(shader, ortho, x + panelW * 0.5f, rowY, border, rowH, 1.0f, 0.85f, 0.10f, 1.0f);
                }
                if (fontRenderer.isLoaded()) {
                    fontRenderer.drawText(shader, ortho, x, rowY, player.name, 0.078f, r, g, b, 1.0f, true);
                }
                rowIndex++;
            }
        };

        const bool localTeamIsRed = myTeam == "RED";
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

    static void renderTopScorePanels(const Shader& shader, const float* ortho,
                                     const FontRenderer& fontRenderer,
                                     float aspect, const std::string& myTeam,
                                     int redCount, int blueCount) {
        float panelY = 0.85f, panelW = 0.52f, panelH = 0.16f;

        const bool localTeamIsRed = myTeam == "RED";
        const int localScore = localTeamIsRed ? redCount : blueCount;
        const int opponentScore = localTeamIsRed ? blueCount : redCount;
        const float localR = localTeamIsRed ? 1.0f : 0.05f;
        const float localG = localTeamIsRed ? 0.22f : 0.55f;
        const float localB = localTeamIsRed ? 0.22f : 1.0f;
        const float opponentR = localTeamIsRed ? 0.05f : 1.0f;
        const float opponentG = localTeamIsRed ? 0.55f : 0.22f;
        const float opponentB = localTeamIsRed ? 1.0f : 0.22f;

        float localX = -aspect + panelW * 0.5f + 0.06f;
        UIDrawHelpers::drawQuad(shader, ortho, localX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        UIDrawHelpers::drawQuad(shader, ortho, localX, panelY - panelH * 0.5f, panelW, 0.01f, localR, localG, localB, 0.95f);
        if (fontRenderer.isLoaded()) {
            fontRenderer.drawText(shader, ortho, localX + 0.04f, panelY, std::to_string(localScore), 0.14f, localR, localG, localB, 1.0f, false);
        }

        float opponentX = aspect - panelW * 0.5f - 0.06f;
        UIDrawHelpers::drawQuad(shader, ortho, opponentX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        UIDrawHelpers::drawQuad(shader, ortho, opponentX, panelY - panelH * 0.5f, panelW, 0.01f, opponentR, opponentG, opponentB, 0.95f);
        if (fontRenderer.isLoaded()) {
            fontRenderer.drawText(shader, ortho, opponentX + 0.04f, panelY, std::to_string(opponentScore), 0.14f, opponentR, opponentG, opponentB, 1.0f, false);
        }
    }

    static void renderCountdown(const Shader& shader, const float* ortho,
                                const FontRenderer& fontRenderer, float countdownTimer) {
        int sec = static_cast<int>(countdownTimer);
        float fraction = countdownTimer - static_cast<float>(sec);

        float scale = 1.0f + std::sin(fraction * 3.14159265f) * 0.35f;
        float alpha = 1.0f - std::pow(fraction, 2.5f);

        if (!fontRenderer.isLoaded()) return;

        if (sec == 0) {
            fontRenderer.drawText(shader, ortho, 0.0f, 0.0f, "3", scale * 0.4f, 1.0f, 0.9f, 0.2f, alpha, true);
        } else if (sec == 1) {
            fontRenderer.drawText(shader, ortho, 0.0f, 0.0f, "2", scale * 0.4f, 1.0f, 0.6f, 0.2f, alpha, true);
        } else if (sec == 2) {
            fontRenderer.drawText(shader, ortho, 0.0f, 0.0f, "1", scale * 0.4f, 0.2f, 1.0f, 0.4f, alpha, true);
        } else if (sec == 3) {
            fontRenderer.drawText(shader, ortho, 0.0f, 0.0f, Strings::get(StringId::COUNTDOWN_GO), scale * 0.35f, 0.1f, 0.9f, 1.0f, alpha, true);
        }
    }
};

#endif // TOUCHPARTY_UIINGAMEOVERLAY_H
