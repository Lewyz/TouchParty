#ifndef ANDROIDGLINVESTIGATIONS_GAMEUI_H
#define ANDROIDGLINVESTIGATIONS_GAMEUI_H

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include "MatrixMath.h"
#include "Shader.h"
#include "Model.h"
#include "AudioEngine.h"

enum class GameState {
    MENU,
    COUNTDOWN,
    PLAYING,
    MATCH_OVER
};

enum class TouchAction {
    NONE,
    PLAY,
    RESET
};

class GameUI {
public:
    GameUI() : state_(GameState::MENU), countdownTimer_(0.0f), matchTimer_(30.0f), uiCubeRot_(0.0f), lastCountdownSec_(-1) {
        initQuadGeometry();
        initMiniCubeGeometry();
    }

    GameState getState() const { return state_; }
    void setState(GameState state) { state_ = state; }

    float getMatchTimer() const { return matchTimer_; }
    void setMatchTimer(float t) { matchTimer_ = t; }

    void startCountdown(AudioEngine* audioEngine = nullptr) {
        state_ = GameState::COUNTDOWN;
        countdownTimer_ = 0.0f;
        matchTimer_ = 30.0f;
        lastCountdownSec_ = -1;
        if (audioEngine) audioEngine->playCountdownBeep();
    }

    void update(float deltaTime, AudioEngine* audioEngine = nullptr) {
        uiCubeRot_ += deltaTime * 50.0f;
        if (uiCubeRot_ >= 360.0f) uiCubeRot_ -= 360.0f;

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
                matchTimer_ = 30.0f;
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

    TouchAction handleTouch(float screenX, float screenY, float width, float height, AudioEngine* audioEngine = nullptr) {
        float aspect = width / height;
        float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
        float normY = 1.0f - (2.0f * screenY) / height;

        // 1. Check Reset Button (Top-Left below Red score panel)
        float resetX = -aspect + 0.28f;
        float resetY = 0.65f;
        float resetW = 0.38f;
        float resetH = 0.14f;

        if (std::abs(normX - resetX) <= resetW * 0.5f + 0.03f &&
            std::abs(normY - resetY) <= resetH * 0.5f + 0.03f) {
            matchTimer_ = 30.0f;
            return TouchAction::RESET;
        }

        // 2. Check Play Button (Only active in MENU state)
        if (state_ == GameState::MENU) {
            float dx = normX - 0.0f;
            float dy = normY - (-0.1f);
            if (dx * dx + dy * dy <= 0.25f * 0.25f) {
                startCountdown(audioEngine);
                return TouchAction::PLAY;
            }
        } else if (state_ == GameState::MATCH_OVER) {
            // Check "PLAY AGAIN" Button in center card
            float playAgainX = 0.0f;
            float playAgainY = -0.20f;
            float playAgainW = 0.55f;
            float playAgainH = 0.16f;

            if (std::abs(normX - playAgainX) <= playAgainW * 0.5f + 0.04f &&
                std::abs(normY - playAgainY) <= playAgainH * 0.5f + 0.04f) {
                startCountdown(audioEngine);
                return TouchAction::RESET;
            }
        }

        return TouchAction::NONE;
    }

    void render(const Shader& shader, float width, float height, int redCount, int blueCount) const {
        float aspect = width / height;
        float ortho[16];
        MatrixMath::orthographic(ortho, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

        shader.setUseTexture(false);

        // 1. Top Side Panels, Reset Button & Center Match Timer Display
        renderTopScorePanels(shader, ortho, aspect, redCount, blueCount);
        renderMatchTimerDisplay(shader, ortho);

        // 2. Render State Specific UI
        if (state_ == GameState::MENU) {
            renderPlayButton(shader, ortho);
        } else if (state_ == GameState::COUNTDOWN) {
            renderCountdown(shader, ortho);
        } else if (state_ == GameState::MATCH_OVER) {
            renderWinnerOverlay(shader, ortho, redCount, blueCount);
        }
    }

private:
    void renderMatchTimerDisplay(const Shader& shader, const float* ortho) const {
        float timerX = 0.0f;
        float timerY = 0.85f;
        float timerW = 0.44f;
        float timerH = 0.16f;

        // Container panel
        drawQuad(shader, ortho, timerX, timerY, timerW, timerH, 0.10f, 0.13f, 0.20f, 0.92f);

        // Pulse color if time is low
        float r = 0.1f, g = 0.9f, b = 1.0f;
        if (matchTimer_ <= 10.0f) {
            r = 1.0f; g = 0.3f; b = 0.2f;
        }

        drawQuad(shader, ortho, timerX, timerY - timerH * 0.5f, timerW, 0.01f, r, g, b, 0.95f);

        // Render "00:XX"
        int totalSec = std::max(0, static_cast<int>(std::ceil(matchTimer_)));
        int mins = totalSec / 60;
        int secs = totalSec % 60;

        std::string timeStr = (mins < 10 ? "0" : "") + std::to_string(mins) + ":" + (secs < 10 ? "0" : "") + std::to_string(secs);

        float charSpacing = 0.065f;
        float startX = timerX - charSpacing * 2.1f;
        for (char c : timeStr) {
            if (c == ':') {
                // Draw colon dots
                drawQuad(shader, ortho, startX, timerY + 0.025f, 0.015f, 0.015f, r, g, b, 1.0f);
                drawQuad(shader, ortho, startX, timerY - 0.025f, 0.015f, 0.015f, r, g, b, 1.0f);
            } else {
                int digit = c - '0';
                drawSingleDigit(shader, ortho, startX, timerY, digit, 0.075f, r, g, b, 1.0f);
            }
            startX += charSpacing;
        }
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
            drawTextREDWINS(shader, ortho, cx, textY, 0.08f, 1.0f, 0.25f, 0.25f, 1.0f);
        } else if (blueCount > redCount) {
            drawTextBLUEWINS(shader, ortho, cx, textY, 0.08f, 0.1f, 0.65f, 1.0f, 1.0f);
        } else {
            drawTextDRAW(shader, ortho, cx, textY, 0.085f, 1.0f, 0.9f, 0.2f, 1.0f);
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
        drawTextAGAIN(shader, ortho, btnX, btnY, 0.075f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    void drawTextREDWINS(const Shader& shader, const float* ortho, float cx, float cy, float size, float r, float g, float b, float a) const {
        float sp = size * 0.65f;
        float x = cx - sp * 3.5f;

        drawCharR(shader, ortho, x + sp * 0.0f, cy, size, r, g, b, a);
        drawCharE(shader, ortho, x + sp * 1.0f, cy, size, r, g, b, a);
        drawCharD(shader, ortho, x + sp * 2.0f, cy, size, r, g, b, a);

        // Space
        x += sp * 0.8f;
        drawCharW(shader, ortho, x + sp * 3.0f, cy, size, r, g, b, a);
        drawCharI(shader, ortho, x + sp * 4.0f, cy, size, r, g, b, a);
        drawCharN(shader, ortho, x + sp * 5.0f, cy, size, r, g, b, a);
        drawCharS(shader, ortho, x + sp * 6.0f, cy, size, r, g, b, a);
    }

    void drawTextBLUEWINS(const Shader& shader, const float* ortho, float cx, float cy, float size, float r, float g, float b, float a) const {
        float sp = size * 0.65f;
        float x = cx - sp * 4.0f;

        drawCharB(shader, ortho, x + sp * 0.0f, cy, size, r, g, b, a);
        drawCharL(shader, ortho, x + sp * 1.0f, cy, size, r, g, b, a);
        drawCharU(shader, ortho, x + sp * 2.0f, cy, size, r, g, b, a);
        drawCharE(shader, ortho, x + sp * 3.0f, cy, size, r, g, b, a);

        // Space
        x += sp * 0.8f;
        drawCharW(shader, ortho, x + sp * 4.0f, cy, size, r, g, b, a);
        drawCharI(shader, ortho, x + sp * 5.0f, cy, size, r, g, b, a);
        drawCharN(shader, ortho, x + sp * 6.0f, cy, size, r, g, b, a);
        drawCharS(shader, ortho, x + sp * 7.0f, cy, size, r, g, b, a);
    }

    void drawTextDRAW(const Shader& shader, const float* ortho, float cx, float cy, float size, float r, float g, float b, float a) const {
        float sp = size * 0.65f;
        float x = cx - sp * 1.5f;

        drawCharD(shader, ortho, x + sp * 0.0f, cy, size, r, g, b, a);
        drawCharR(shader, ortho, x + sp * 1.0f, cy, size, r, g, b, a);
        drawCharA(shader, ortho, x + sp * 2.0f, cy, size, r, g, b, a);
        drawCharW(shader, ortho, x + sp * 3.0f, cy, size, r, g, b, a);
    }

    void drawTextAGAIN(const Shader& shader, const float* ortho, float cx, float cy, float size, float r, float g, float b, float a) const {
        float sp = size * 0.65f;
        float x = cx - sp * 2.0f;

        drawCharA(shader, ortho, x + sp * 0.0f, cy, size, r, g, b, a);
        drawCharG(shader, ortho, x + sp * 1.0f, cy, size, r, g, b, a);
        drawCharA(shader, ortho, x + sp * 2.0f, cy, size, r, g, b, a);
        drawCharI(shader, ortho, x + sp * 3.0f, cy, size, r, g, b, a);
        drawCharN(shader, ortho, x + sp * 4.0f, cy, size, r, g, b, a);
    }

    void drawCharB(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y + h * 0.5f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y - h * 0.5f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharD(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharA(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
    }

    void drawCharL(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharU(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharW(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h * 0.25f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharI(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x, y, t, h * 2.0f, r, g, b, a);
    }

    void drawCharN(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y + h * 0.25f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y, t, h * 2.0f, r, g, b, a);
    }

    void drawCharG(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y - h * 0.25f, t, h * 0.5f, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.25f, y, w * 0.5f, t, r, g, b, a);
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
        drawTextRESET(shader, ortho, resetX, resetY, 0.075f, 0.9f, 0.95f, 1.0f, 1.0f);

        // Top-Right Panel (BLUE Score)
        float blueX = aspect - panelW * 0.5f - 0.06f;
        drawQuad(shader, ortho, blueX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, blueX, panelY - panelH * 0.5f, panelW, 0.01f, 0.05f, 0.55f, 1.0f, 0.95f);
        draw3DMiniCube(shader, ortho, blueX - panelW * 0.30f, panelY, 0.05f, 0.55f, 1.0f);
        drawDigits(shader, ortho, blueX + 0.04f, panelY, blueCount, 0.25f, 0.68f, 1.0f);
    }

    void drawTextRESET(const Shader& shader, const float* ortho, float cx, float cy, float size,
                       float r, float g, float b, float a) const {
        float spacing = size * 0.65f;
        float startX = cx - spacing * 2.0f;

        drawCharR(shader, ortho, startX + spacing * 0.0f, cy, size, r, g, b, a);
        drawCharE(shader, ortho, startX + spacing * 1.0f, cy, size, r, g, b, a);
        drawCharS(shader, ortho, startX + spacing * 2.0f, cy, size, r, g, b, a);
        drawCharE(shader, ortho, startX + spacing * 3.0f, cy, size, r, g, b, a);
        drawCharT(shader, ortho, startX + spacing * 4.0f, cy, size, r, g, b, a);
    }

    void drawCharR(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y + h * 0.5f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.25f, y - h * 0.5f, t, h, r, g, b, a);
    }

    void drawCharE(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x - w * 0.5f, y, t, h * 2.0f, r, g, b, a);
        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharS(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x, y + h, w, t, r, g, b, a);
        drawQuad(shader, ortho, x - w * 0.5f, y + h * 0.5f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x, y, w, t, r, g, b, a);
        drawQuad(shader, ortho, x + w * 0.5f, y - h * 0.5f, t, h, r, g, b, a);
        drawQuad(shader, ortho, x, y - h, w, t, r, g, b, a);
    }

    void drawCharT(const Shader& shader, const float* ortho, float x, float y, float size, float r, float g, float b, float a) const {
        float w = size * 0.45f;
        float h = size * 0.45f;
        float t = size * 0.10f;

        drawQuad(shader, ortho, x, y + h, w * 1.2f, t, r, g, b, a);
        drawQuad(shader, ortho, x, y, t, h * 2.0f, r, g, b, a);
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

    void renderPlayButton(const Shader& shader, const float* ortho) const {
        float btnX = 0.0f;
        float btnY = -0.1f;
        float btnRadius = 0.24f;

        drawCircle(shader, ortho, btnX, btnY, btnRadius + 0.04f, 0.1f, 0.6f, 1.0f, 0.4f);
        drawCircle(shader, ortho, btnX, btnY, btnRadius, 0.08f, 0.45f, 0.9f, 0.95f);
        drawPlayTriangle(shader, ortho, btnX + 0.02f, btnY, 0.10f, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    void renderCountdown(const Shader& shader, const float* ortho) const {
        int sec = static_cast<int>(countdownTimer_);
        float fraction = countdownTimer_ - static_cast<float>(sec);

        float scale = 1.0f + std::sin(fraction * PI) * 0.35f;
        float alpha = 1.0f - std::pow(fraction, 2.5f);

        if (sec == 0) {
            drawSingleDigit(shader, ortho, 0.0f, 0.0f, 3, scale * 0.4f, 1.0f, 0.9f, 0.2f, alpha);
        } else if (sec == 1) {
            drawSingleDigit(shader, ortho, 0.0f, 0.0f, 2, scale * 0.4f, 1.0f, 0.6f, 0.2f, alpha);
        } else if (sec == 2) {
            drawSingleDigit(shader, ortho, 0.0f, 0.0f, 1, scale * 0.4f, 0.2f, 1.0f, 0.4f, alpha);
        } else if (sec == 3) {
            drawTextGO(shader, ortho, 0.0f, 0.0f, scale * 0.35f, 0.1f, 0.9f, 1.0f, alpha);
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

    void drawCircle(const Shader& shader, const float* ortho, float cx, float cy, float radius,
                    float r, float g, float b, float a) const {
        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, cx, cy, 0.0f);
        MatrixMath::scale(model, radius, radius, 1.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(r, g, b, a);

        shader.drawIndexed(
            circleVertices_.data(),
            sizeof(Vertex),
            0,
            sizeof(Vector3),
            circleIndices_.data(),
            circleIndices_.size()
        );
    }

    void drawPlayTriangle(const Shader& shader, const float* ortho, float cx, float cy, float size,
                          float r, float g, float b, float a) const {
        std::vector<Vertex> triVertices = {
            Vertex(Vector3{-size * 0.6f,  size * 0.8f, 0.0f}, Vector2{0, 0}),
            Vertex(Vector3{ size * 0.8f,  0.0f,        0.0f}, Vector2{1, 0}),
            Vertex(Vector3{-size * 0.6f, -size * 0.8f, 0.0f}, Vector2{0, 1})
        };
        std::vector<uint16_t> triIndices = { 0, 1, 2 };

        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, cx, cy, 0.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(r, g, b, a);

        shader.drawIndexed(
            triVertices.data(),
            sizeof(Vertex),
            0,
            sizeof(Vector3),
            triIndices.data(),
            triIndices.size()
        );
    }

    void drawDigits(const Shader& shader, const float* ortho, float startX, float startY, int number,
                    float r, float g, float b) const {
        std::string s = std::to_string(number);
        float charSpacing = 0.07f;
        float x = startX;

        for (char c : s) {
            int digit = c - '0';
            if (digit >= 0 && digit <= 9) {
                drawSingleDigit(shader, ortho, x, startY, digit, 0.08f, r, g, b, 1.0f);
            }
            x += charSpacing;
        }
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
        float t = size * 0.08f;
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

    void drawTextGO(const Shader& shader, const float* ortho, float x, float y, float size,
                    float r, float g, float b, float a) const {
        float w = size * 0.6f;
        float h = size * 0.8f;
        float t = size * 0.12f;

        float gx = x - size * 0.5f;
        drawQuad(shader, ortho, gx, y + h * 0.5f, w, t, r, g, b, a);
        drawQuad(shader, ortho, gx - w * 0.5f, y, t, h, r, g, b, a);
        drawQuad(shader, ortho, gx, y - h * 0.5f, w, t, r, g, b, a);
        drawQuad(shader, ortho, gx + w * 0.5f, y - h * 0.25f, t, h * 0.5f, r, g, b, a);
        drawQuad(shader, ortho, gx + w * 0.25f, y, w * 0.5f, t, r, g, b, a);

        float ox = x + size * 0.5f;
        drawQuad(shader, ortho, ox, y + h * 0.5f, w, t, r, g, b, a);
        drawQuad(shader, ortho, ox - w * 0.5f, y, t, h, r, g, b, a);
        drawQuad(shader, ortho, ox + w * 0.5f, y, t, h, r, g, b, a);
        drawQuad(shader, ortho, ox, y - h * 0.5f, w, t, r, g, b, a);
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
            float angle = (i * 2.0f * PI) / SEGMENTS;
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
    std::vector<Vertex> quadVertices_;
    std::vector<uint16_t> quadIndices_;
    std::vector<Vertex> circleVertices_;
    std::vector<uint16_t> circleIndices_;

    std::vector<Vertex> miniCubeVertices_;
    std::vector<uint16_t> miniCubeIndices_;
    std::vector<float> miniCubeShades_;
};

#endif // ANDROIDGLINVESTIGATIONS_GAMEUI_H