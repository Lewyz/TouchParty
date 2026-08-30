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
    PLAYING
};

class GameUI {
public:
    GameUI() : state_(GameState::MENU), countdownTimer_(0.0f), uiCubeRot_(0.0f), lastCountdownSec_(-1) {
        initQuadGeometry();
        initMiniCubeGeometry();
    }

    GameState getState() const { return state_; }
    void setState(GameState state) { state_ = state; }

    void startCountdown(AudioEngine* audioEngine = nullptr) {
        state_ = GameState::COUNTDOWN;
        countdownTimer_ = 0.0f;
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
            }
        }
    }

    bool handleTouch(float screenX, float screenY, float width, float height, AudioEngine* audioEngine = nullptr) {
        if (state_ == GameState::MENU) {
            float aspect = width / height;
            float normX = ((2.0f * screenX) / width - 1.0f) * aspect;
            float normY = 1.0f - (2.0f * screenY) / height;

            float dx = normX - 0.0f;
            float dy = normY - (-0.1f);
            if (dx * dx + dy * dy <= 0.25f * 0.25f) {
                startCountdown(audioEngine);
                return true;
            }
        }
        return false;
    }

    void render(const Shader& shader, float width, float height, int redCount, int blueCount) const {
        float aspect = width / height;
        float ortho[16];
        MatrixMath::orthographic(ortho, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

        shader.setUseTexture(false);

        // 1. Top Side Panels with 3D Mini-Cubes
        renderTopScorePanels(shader, ortho, aspect, redCount, blueCount);

        // 2. Render State Specific UI
        if (state_ == GameState::MENU) {
            renderPlayButton(shader, ortho);
        } else if (state_ == GameState::COUNTDOWN) {
            renderCountdown(shader, ortho);
        }
    }

private:
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

        // Top-Right Panel (BLUE Score)
        float blueX = aspect - panelW * 0.5f - 0.06f;
        drawQuad(shader, ortho, blueX, panelY, panelW, panelH, 0.12f, 0.15f, 0.22f, 0.92f);
        drawQuad(shader, ortho, blueX, panelY - panelH * 0.5f, panelW, 0.01f, 0.05f, 0.55f, 1.0f, 0.95f);
        draw3DMiniCube(shader, ortho, blueX - panelW * 0.30f, panelY, 0.05f, 0.55f, 1.0f);
        drawDigits(shader, ortho, blueX + 0.04f, panelY, blueCount, 0.25f, 0.68f, 1.0f);
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