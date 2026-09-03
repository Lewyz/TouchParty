#ifndef TOUCHPARTY_UIBANNER_H
#define TOUCHPARTY_UIBANNER_H

#include <string>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UIDrawHelpers.h"
#include "Strings.h"

class UIBanner {
public:
    // Renders top server connection status banner
    static void renderServerStatus(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                                   bool isConnected) {
        UIButtonSpec bannerSpec;
        bannerSpec.x = 0.0f;
        bannerSpec.y = 0.91f;
        bannerSpec.w = 1.35f;
        bannerSpec.h = 0.11f;
        bannerSpec.text = isConnected ? Strings::get(StringId::SERVER_CONNECTED) : Strings::get(StringId::SERVER_DISCONNECTED);
        bannerSpec.bgColor = isConnected ? UITheme::SERVER_CONNECTED_BG : UITheme::SERVER_DISCONNECTED_BG;
        bannerSpec.borderColor = isConnected ? UITheme::SERVER_CONNECTED_BORDER : UITheme::SERVER_DISCONNECTED_BORDER;
        bannerSpec.textColor = isConnected ? UITheme::SERVER_CONNECTED_TEXT : UITheme::SERVER_DISCONNECTED_TEXT;
        bannerSpec.fontSize = 0.092f;
        bannerSpec.isClickable = false;

        UIButton::render(shader, ortho, fontRenderer, bannerSpec);
    }

    // Renders top-right registered user nickname badge in bright gold/yellow text
    static void renderNicknameBadge(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                                    float aspect, const std::string& nickname) {
        std::string nameToDraw = nickname.empty() ? "PLAYER" : nickname;

        float badgeW = 0.95f;
        float badgeH = 0.16f;
        float badgeX = aspect - badgeW * 0.5f - 0.04f;
        float badgeY = 0.84f;

        UIButtonSpec badgeSpec;
        badgeSpec.x = badgeX;
        badgeSpec.y = badgeY;
        badgeSpec.w = badgeW;
        badgeSpec.h = badgeH;
        badgeSpec.text = nameToDraw;
        badgeSpec.bgColor = UITheme::CARD_BG;
        badgeSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        badgeSpec.textColor = UITheme::TEXT_TITLE_GOLD;
        badgeSpec.fontSize = 0.092f;
        badgeSpec.isClickable = false;

        UIButton::render(shader, ortho, fontRenderer, badgeSpec);
    }

    // Renders notification banner when a player joins or leaves
    static void renderNotification(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                                   const std::string& notificationText) {
        if (notificationText.empty()) return;
        float banX = 0.0f, banY = 0.70f, banW = 1.45f, banH = 0.12f;
        UIDrawHelpers::drawQuad(shader, ortho, banX, banY, banW, banH, 0.10f, 0.65f, 0.35f, 0.95f);
        UIDrawHelpers::drawQuad(shader, ortho, banX, banY - banH * 0.5f, banW, 0.008f, 0.40f, 0.95f, 0.60f, 0.95f);
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, banX, banY, notificationText, 0.090f, banW - 0.20f, 1.0f, 1.0f, 1.0f);
    }
};

#endif // TOUCHPARTY_UIBANNER_H
