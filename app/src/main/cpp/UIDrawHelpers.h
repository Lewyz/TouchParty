#ifndef TOUCHPARTY_UIDRAWHELPERS_H
#define TOUCHPARTY_UIDRAWHELPERS_H

#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include "Shader.h"
#include "FontRenderer.h"
#include "MatrixMath.h"
#include "TextureAsset.h"
#include "UITheme.h"
#include "UIButton.h"
#include "Strings.h"

class UIDrawHelpers {
public:
    // Draws a colored 2D quad rectangle
    static void drawQuad(const Shader& shader, const float* ortho, float x, float y, float w, float h,
                         float r, float g, float b, float a) {
        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, x, y, 0.0f);
        MatrixMath::scale(model, w * 0.5f, h * 0.5f, 1.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(r, g, b, a);

        static const Vertex quadVertices[] = {
            Vertex(Vector3{-1.0f,  1.0f, 0.0f}, Vector2{0, 0}),
            Vertex(Vector3{ 1.0f,  1.0f, 0.0f}, Vector2{1, 0}),
            Vertex(Vector3{ 1.0f, -1.0f, 0.0f}, Vector2{1, 1}),
            Vertex(Vector3{-1.0f, -1.0f, 0.0f}, Vector2{0, 1})
        };
        static const uint16_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

        shader.drawIndexed(quadVertices, sizeof(Vertex), 0, sizeof(Vector3), quadIndices, 6);
    }

    // Draws a colored 2D quad rectangle using ColorRGBA
    static void drawQuad(const Shader& shader, const float* ortho, float x, float y, float w, float h, const ColorRGBA& c) {
        drawQuad(shader, ortho, x, y, w, h, c.r, c.g, c.b, c.a);
    }

    // Draws an RGBA PNG texture sprite as a 2D quad (flags, gear icon, etc.)
    static void drawIcon(const Shader& shader, const float* ortho,
                         const std::shared_ptr<TextureAsset>& texture,
                         float x, float y, float w, float h) {
        if (!texture) return;
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
        static const Vertex vertices[] = {
            Vertex(Vector3{-1.0f,  1.0f, 0.0f}, Vector2{0.0f, 1.0f}),
            Vertex(Vector3{ 1.0f,  1.0f, 0.0f}, Vector2{1.0f, 1.0f}),
            Vertex(Vector3{ 1.0f, -1.0f, 0.0f}, Vector2{1.0f, 0.0f}),
            Vertex(Vector3{-1.0f, -1.0f, 0.0f}, Vector2{0.0f, 0.0f})
        };
        static const uint16_t indices[] = {0, 1, 2, 0, 2, 3};
        shader.drawIndexed(vertices, sizeof(Vertex), 0, sizeof(Vector3), indices, 6);
        shader.setUseTexture(false);
    }

    // Draws team arrow cursor sprites
    static void drawCursorSprite(const Shader& shader, const float* ortho, float x, float y,
                                 const std::string& team, bool pointsRight,
                                 const std::shared_ptr<TextureAsset>& arrowBlueLeft,
                                 const std::shared_ptr<TextureAsset>& arrowBlueRight,
                                 const std::shared_ptr<TextureAsset>& arrowRedLeft,
                                 const std::shared_ptr<TextureAsset>& arrowRedRight) {
        const std::shared_ptr<TextureAsset>& texture = (team == "BLUE")
                ? (pointsRight ? arrowBlueRight : arrowBlueLeft)
                : (pointsRight ? arrowRedRight : arrowRedLeft);
        if (!texture) return;

        drawIcon(shader, ortho, texture, x, y, 0.18f, 0.125f);
    }

    // Auto-fit font size calculation
    static float fitTextSize(const std::string& text, float size, float maxWidth, float minRatio = 0.72f) {
        if (maxWidth <= 0.0f || text.empty()) return size;
        float w = text.length() > 0 ? size * text.length() * 0.90f : size;
        if (w <= maxWidth) return size;
        float fit = size * (maxWidth / w);
        if (fit < size * minRatio) fit = size * minRatio;
        return fit;
    }

    // Truncates string to fit width
    static std::string truncateToFit(const std::string& text, float maxWidth, float size) {
        if (maxWidth <= 0.0f || text.empty()) return text;
        float fullW = text.empty() ? 0.0f : size * text.length() * 0.90f;
        if (fullW <= maxWidth) return text;
        const std::string ell = "...";
        size_t keep = text.length();
        while (keep > 1) {
            std::string cand = text.substr(0, keep) + ell;
            float candW = size * cand.length() * 0.90f;
            if (candW <= maxWidth) return cand;
            --keep;
        }
        return text.substr(0, 1) + ell;
    }

    // Draws auto-fitted text with truncation fallback
    static void drawTextFitted(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                               float cx, float cy, const std::string& text, float size, float maxWidth,
                               float r, float g, float b, float a = 1.0f) {
        if (text.empty() || !fontRenderer.isLoaded()) return;
        float fit = size;
        if (maxWidth > 0.0f) {
            float w = fontRenderer.getTextWidth(text, size);
            if (w > maxWidth) {
                fit = size * (maxWidth / w);
                const float floor = size * 0.72f;
                if (fit < floor) {
                    std::string shortText = truncateToFit(text, maxWidth, floor);
                    fontRenderer.drawText(shader, ortho, cx, cy, shortText, floor, r, g, b, a, true);
                    return;
                }
            }
        }
        fontRenderer.drawText(shader, ortho, cx, cy, text, fit, r, g, b, a, true);
    }

    // Draws a standard Back button with comfortable left margin
    static void renderBackButton(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer, float aspect) {
        UIButtonSpec backSpec;
        backSpec.x = -aspect + 0.28f;
        backSpec.y = -0.85f;
        backSpec.w = 0.36f;
        backSpec.h = 0.12f;
        backSpec.text = Strings::get(StringId::BACK);
        backSpec.bgColor = UITheme::CARD_BG;
        backSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        backSpec.textColor = UITheme::TEXT_WHITE;
        backSpec.fontSize = 0.075f;
        UIButton::render(shader, ortho, fontRenderer, backSpec);
    }

    // Checks click inside Back button
    static bool isBackButtonClicked(float normX, float normY, float aspect) {
        UIButtonSpec backSpec;
        backSpec.x = -aspect + 0.28f;
        backSpec.y = -0.85f;
        backSpec.w = 0.36f;
        backSpec.h = 0.12f;
        return UIButton::contains(normX, normY, backSpec);
    }
};

#endif // TOUCHPARTY_UIDRAWHELPERS_H
