#ifndef TOUCHPARTY_UICARD_H
#define TOUCHPARTY_UICARD_H

#include <string>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIDrawHelpers.h"

struct UICardSpec {
    float x = 0.0f;
    float y = 0.0f;
    float w = 1.80f;
    float h = 1.00f;
    std::string title;
    ColorRGBA bgColor = UITheme::CARD_BG;
    ColorRGBA borderColor = UITheme::CARD_BORDER_CYAN;
    ColorRGBA titleColor = UITheme::TEXT_TITLE_GOLD;
    ColorRGBA underlineColor = UITheme::CARD_BORDER_GOLD;
    float borderThickness = 0.012f;
    float titleFontSize = 0.115f;
    float titleOffsetY = 0.0f;
    bool showUnderline = true;
};

class UICard {
public:
    static void render(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                       const UICardSpec& spec) {
        // 1. Draw main container background quad
        UIDrawHelpers::drawQuad(shader, ortho, spec.x, spec.y, spec.w, spec.h, spec.bgColor);

        // 2. Draw outer border frame
        if (spec.borderThickness > 0.0f) {
            float bt = spec.borderThickness;
            // Top border
            UIDrawHelpers::drawQuad(shader, ortho, spec.x, spec.y + spec.h * 0.5f, spec.w, bt, spec.borderColor);
            // Bottom border
            UIDrawHelpers::drawQuad(shader, ortho, spec.x, spec.y - spec.h * 0.5f, spec.w, bt, spec.borderColor);
            // Left border
            UIDrawHelpers::drawQuad(shader, ortho, spec.x - spec.w * 0.5f, spec.y, bt, spec.h, spec.borderColor);
            // Right border
            UIDrawHelpers::drawQuad(shader, ortho, spec.x + spec.w * 0.5f, spec.y, bt, spec.h, spec.borderColor);
        }

        // 3. Draw Title Text inside Header
        if (!spec.title.empty() && fontRenderer.isLoaded()) {
            float titleY = spec.y + spec.h * 0.5f - 0.12f + spec.titleOffsetY;
            float maxTextW = spec.w - 0.16f;
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, spec.x, titleY,
                                         spec.title, spec.titleFontSize, maxTextW,
                                         spec.titleColor.r, spec.titleColor.g, spec.titleColor.b, spec.titleColor.a);

            // 4. Draw Header Underline
            if (spec.showUnderline) {
                float lineY = titleY - 0.08f;
                float lineW = spec.w * 0.45f;
                UIDrawHelpers::drawQuad(shader, ortho, spec.x, lineY, lineW, 0.008f, spec.underlineColor);
            }
        }
    }
};

#endif // TOUCHPARTY_UICARD_H
