#ifndef TOUCHPARTY_UISETUPSCREEN_H
#define TOUCHPARTY_UISETUPSCREEN_H

#include <string>
#include <memory>
#include <cmath>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "TextureAsset.h"
#include "AudioEngine.h"
#include "Strings.h"

class UISetupScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer, const std::string& userNickname,
                       const std::shared_ptr<TextureAsset>& flagEs,
                       const std::shared_ptr<TextureAsset>& flagUs) {
        float cx = 0.0f, cy = -0.03f;
        float cardW = 1.88f, cardH = 0.88f;

        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::WELCOME_SETUP_TITLE);
        cardSpec.titleFontSize = 0.115f;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        float flagY = 0.18f, flagW = 0.55f, flagH = 0.34f;
        float esX = -0.35f, usX = 0.35f;

        UIDrawHelpers::drawIcon(shader, ortho, flagEs, esX, flagY, flagW, flagH);
        UIDrawHelpers::drawIcon(shader, ortho, flagUs, usX, flagY, flagW, flagH);

        bool isEs = Strings::getLanguage() == Language::SPANISH;
        float activeX = isEs ? esX : usX;
        UIDrawHelpers::drawQuad(shader, ortho, activeX, flagY + flagH * 0.5f, flagW + 0.04f, 0.015f, UITheme::CARD_BORDER_GOLD);
        UIDrawHelpers::drawQuad(shader, ortho, activeX, flagY - flagH * 0.5f, flagW + 0.04f, 0.015f, UITheme::CARD_BORDER_GOLD);

        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, esX, flagY - flagH * 0.5f - 0.08f, "ESPAÑOL", 0.085f, flagW, 1.0f, 1.0f, 1.0f);
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, usX, flagY - flagH * 0.5f - 0.08f, "ENGLISH", 0.085f, flagW, 1.0f, 1.0f, 1.0f);

        float nickY = -0.22f, nickW = 1.1f, nickH = 0.14f;
        std::string nickLabel = userNickname.empty() ? Strings::get(StringId::PROMPT_ENTER_NICKNAME) : userNickname;

        UIButtonSpec nickSpec;
        nickSpec.x = cx;
        nickSpec.y = nickY;
        nickSpec.w = nickW;
        nickSpec.h = nickH;
        nickSpec.text = nickLabel;
        nickSpec.bgColor = UITheme::CARD_BG;
        nickSpec.borderColor = userNickname.empty() ? UITheme::CARD_BORDER_GOLD : UITheme::CARD_BORDER_CYAN;
        nickSpec.textColor = UITheme::TEXT_WHITE;
        nickSpec.fontSize = 0.09f;
        UIButton::render(shader, ortho, fontRenderer, nickSpec);

        float contY = -0.52f, contW = 1.1f, contH = 0.15f;
        UIButtonSpec contSpec;
        contSpec.x = cx;
        contSpec.y = contY;
        contSpec.w = contW;
        contSpec.h = contH;
        contSpec.text = Strings::get(StringId::SAVE_CONTINUE);
        contSpec.bgColor = UITheme::BTN_SUCCESS_BG;
        contSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
        contSpec.textColor = UITheme::TEXT_WHITE;
        contSpec.fontSize = 0.095f;
        UIButton::render(shader, ortho, fontRenderer, contSpec);
    }
};

#endif // TOUCHPARTY_UISETUPSCREEN_H
