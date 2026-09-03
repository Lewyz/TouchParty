#ifndef TOUCHPARTY_UILANGUAGESCREEN_H
#define TOUCHPARTY_UILANGUAGESCREEN_H

#include <string>
#include <memory>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "TextureAsset.h"
#include "Strings.h"

class UILanguageScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer,
                       const std::shared_ptr<TextureAsset>& flagEs,
                       const std::shared_ptr<TextureAsset>& flagUs) {
        // Dark full-screen overlay backdrop
        UIDrawHelpers::drawQuad(shader, ortho, 0.0f, 0.0f, aspect * 2.1f, 2.1f, 0.0f, 0.0f, 0.0f, 0.80f);

        // Panel card
        float cx = 0.0f, cy = 0.0f;
        float cardW = 1.98f, cardH = 1.0f;
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::LANGUAGE_SELECT);
        cardSpec.titleFontSize = 0.11f;
        cardSpec.titleOffsetY = 0.38f;
        cardSpec.showUnderline = false;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        float flagY = 0.14f, flagW = 0.62f, flagH = 0.36f;
        float esX = -0.36f, usX = 0.36f;
        const bool isEs = Strings::getLanguage() == Language::SPANISH;
        const float border = 0.014f;

        // Spanish flag box
        UIDrawHelpers::drawQuad(shader, ortho, esX, flagY, flagW + border * 2.0f, flagH + border * 2.0f,
                                fitsBorderColor(isEs));
        UIDrawHelpers::drawIcon(shader, ortho, flagEs, esX, flagY, flagW, flagH);
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, esX, flagY - flagH * 0.5f - 0.06f,
                                     Strings::get(StringId::LANGUAGE_SPANISH), 0.080f, flagW + border * 2.0f, 1.0f, 0.95f, 0.80f);

        // English flag box
        UIDrawHelpers::drawQuad(shader, ortho, usX, flagY, flagW + border * 2.0f, flagH + border * 2.0f,
                                fitsBorderColor(!isEs));
        UIDrawHelpers::drawIcon(shader, ortho, flagUs, usX, flagY, flagW, flagH);
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, usX, flagY - flagH * 0.5f - 0.06f,
                                     Strings::get(StringId::LANGUAGE_ENGLISH), 0.080f, flagW + border * 2.0f, 0.90f, 0.95f, 1.0f);

        // Back / Close hint
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, cy - 0.42f,
                                     Strings::get(StringId::BACK), 0.088f, cardW * 0.5f, 0.60f, 0.70f, 0.80f);
    }

private:
    static ColorRGBA fitsBorderColor(bool active) {
        return active ? ColorRGBA(0.20f, 0.90f, 0.40f, 1.0f) : ColorRGBA(0.32f, 0.35f, 0.42f, 1.0f);
    }
};

#endif // TOUCHPARTY_UILANGUAGESCREEN_H
