#ifndef TOUCHPARTY_UIWELCOMESCREEN_H
#define TOUCHPARTY_UIWELCOMESCREEN_H

#include <string>
#include <memory>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "TextureAsset.h"
#include "Strings.h"

class UIWelcomeScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer,
                       const std::shared_ptr<TextureAsset>& gearTexture) {
        float cx = 0.0f, cy = -0.03f;
        float cardW = 1.88f, cardH = 0.88f;

        // Container card with game title
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::TITLE_GAME);
        cardSpec.titleFontSize = 0.115f;
        cardSpec.titleOffsetY = 0.34f;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        // Upper-Left Corner Test Button using UIButton
        float testX = -aspect + 0.38f, testY = 0.85f, testW = 0.58f, testH = 0.13f;
        UIButtonSpec testBtnSpec;
        testBtnSpec.x = testX;
        testBtnSpec.y = testY;
        testBtnSpec.w = testW;
        testBtnSpec.h = testH;
        testBtnSpec.text = Strings::get(StringId::TEST_1V1);
        testBtnSpec.bgColor = UITheme::BTN_TEST_BG;
        testBtnSpec.borderColor = UITheme::BTN_TEST_BORDER;
        testBtnSpec.textColor = UITheme::TEXT_WHITE;
        testBtnSpec.fontSize = 0.088f;
        UIButton::render(shader, ortho, fontRenderer, testBtnSpec);

        // Gear (language settings) icon directly below the TEST button
        float gearX = -aspect + 0.16f, gearY = 0.68f, gearW = 0.16f, gearH = 0.16f;
        UIDrawHelpers::drawIcon(shader, ortho, gearTexture, gearX, gearY, gearW, gearH);

        // Navigation Buttons Side by Side (Left: SEARCH ROOMS, Right: CREATE ROOM) using UIButton
        float btnY = -0.12f, btnW = 0.78f, btnH = 0.22f;

        // SEARCH ROOMS (Left - Green Button)
        float leftX = -0.42f;
        UIButtonSpec searchBtn;
        searchBtn.x = leftX;
        searchBtn.y = btnY;
        searchBtn.w = btnW;
        searchBtn.h = btnH;
        searchBtn.text = Strings::get(StringId::SEARCH_ROOMS);
        searchBtn.bgColor = UITheme::BTN_SUCCESS_BG;
        searchBtn.borderColor = UITheme::BTN_SUCCESS_BORDER;
        searchBtn.textColor = UITheme::TEXT_WHITE;
        searchBtn.fontSize = 0.105f;
        UIButton::render(shader, ortho, fontRenderer, searchBtn);

        // CREATE ROOM (Right - Blue Button)
        float rightX = 0.42f;
        UIButtonSpec createBtn;
        createBtn.x = rightX;
        createBtn.y = btnY;
        createBtn.w = btnW;
        createBtn.h = btnH;
        createBtn.text = Strings::get(StringId::CREATE_ROOM);
        createBtn.bgColor = UITheme::BTN_PRIMARY_BG;
        createBtn.borderColor = UITheme::BTN_PRIMARY_BORDER;
        createBtn.textColor = UITheme::TEXT_WHITE;
        createBtn.fontSize = 0.105f;
        UIButton::render(shader, ortho, fontRenderer, createBtn);
    }
};

#endif // TOUCHPARTY_UIWELCOMESCREEN_H
