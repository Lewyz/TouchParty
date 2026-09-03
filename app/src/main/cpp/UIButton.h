#ifndef TOUCHPARTY_UIBUTTON_H
#define TOUCHPARTY_UIBUTTON_H

#include <string>
#include <cmath>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "MatrixMath.h"

struct UIButtonSpec {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.60f;
    float h = 0.16f;
    std::string text;
    ColorRGBA bgColor = UITheme::CARD_BG;
    ColorRGBA borderColor = UITheme::CARD_BORDER_CYAN;
    ColorRGBA textColor = UITheme::TEXT_WHITE;
    float borderThickness = 0.012f;
    float fontSize = 0.100f;
    bool isClickable = true;
};

class UIButton {
public:
    static bool contains(float normX, float normY, float x, float y, float w, float h, float padding = 0.04f) {
        return (std::abs(normX - x) <= (w * 0.5f + padding)) &&
               (std::abs(normY - y) <= (h * 0.5f + padding));
    }

    static bool contains(float normX, float normY, const UIButtonSpec& spec, float padding = 0.04f) {
        if (!spec.isClickable) return false;
        return contains(normX, normY, spec.x, spec.y, spec.w, spec.h, padding);
    }

    static void render(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                       const UIButtonSpec& spec) {
        // 1. Draw main button background quad
        drawQuadHelper(shader, ortho, spec.x, spec.y, spec.w, spec.h, spec.bgColor);

        // 2. Draw bottom accent border line if thickness > 0
        if (spec.borderThickness > 0.0f) {
            float bt = spec.borderThickness;
            drawQuadHelper(shader, ortho, spec.x, spec.y - spec.h * 0.5f, spec.w, bt, spec.borderColor);
        }

        // 3. Draw text label optically centered vertically and horizontally inside button
        if (!spec.text.empty() && fontRenderer.isLoaded()) {
            float fit = spec.fontSize;
            float maxTextW = spec.w - 0.14f;
            if (maxTextW > 0.0f) {
                float measuredW = fontRenderer.getTextWidth(spec.text, spec.fontSize);
                if (measuredW > maxTextW && measuredW > 0.0f) {
                    fit = spec.fontSize * (maxTextW / measuredW);
                }
            }
            float textY = spec.y - fit * 0.12f;
            fontRenderer.drawText(shader, ortho, spec.x, textY, spec.text, fit,
                                 spec.textColor.r, spec.textColor.g, spec.textColor.b, spec.textColor.a, true);
        }
    }

private:
    static void drawQuadHelper(const Shader& shader, const float* ortho, float x, float y, float w, float h, const ColorRGBA& c) {
        float model[16];
        MatrixMath::identity(model);
        MatrixMath::translate(model, x, y, 0.0f);
        MatrixMath::scale(model, w * 0.5f, h * 0.5f, 1.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, ortho, model);
        shader.setProjectionMatrix(mvp);
        shader.setColor(c.r, c.g, c.b, c.a);

        static const Vertex quadVertices[] = {
            Vertex(Vector3{-1.0f,  1.0f, 0.0f}, Vector2{0, 0}),
            Vertex(Vector3{ 1.0f,  1.0f, 0.0f}, Vector2{1, 0}),
            Vertex(Vector3{ 1.0f, -1.0f, 0.0f}, Vector2{1, 1}),
            Vertex(Vector3{-1.0f, -1.0f, 0.0f}, Vector2{0, 1})
        };
        static const uint16_t quadIndices[] = { 0, 1, 2, 0, 2, 3 };

        shader.drawIndexed(quadVertices, sizeof(Vertex), 0, sizeof(Vector3), quadIndices, 6);
    }
};

#endif // TOUCHPARTY_UIBUTTON_H
