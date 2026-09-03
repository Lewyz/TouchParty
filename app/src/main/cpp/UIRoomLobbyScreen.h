#ifndef TOUCHPARTY_UIROOMLOBBYSCREEN_H
#define TOUCHPARTY_UIROOMLOBBYSCREEN_H

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "TextureAsset.h"
#include "GameUIStructs.h"
#include "Strings.h"

class UIRoomLobbyScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer,
                       const std::string& roomName, bool isPrivateRoom,
                       int connectedPlayerCount, bool isOwner, bool roomMatchActive,
                       const std::string& userNickname,
                       const std::vector<PlayerInfo>& currentRoomPlayers,
                       const std::shared_ptr<TextureAsset>& arrowBlueLeft,
                       const std::shared_ptr<TextureAsset>& arrowBlueRight,
                       const std::shared_ptr<TextureAsset>& arrowRedLeft,
                       const std::shared_ptr<TextureAsset>& arrowRedRight) {
        float cx = 0.0f, cy = -0.03f;
        float cardW = std::min(2.65f, aspect * 1.85f), cardH = 1.25f;

        // Header above container card
        float headerY = cy + cardH * 0.5f + 0.10f;
        std::string privTag = "[" + (isPrivateRoom ? Strings::get(StringId::PRIVATE_LABEL) : Strings::get(StringId::PUBLIC_LABEL)) + "]";
        std::string fullHeader = roomName + " " + privTag;
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, headerY, fullHeader, 0.095f, cardW, 1.0f, 0.90f, 0.20f);

        // Container card
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.showUnderline = false;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        // TEAM COLUMNS
        float leftX = -cardW * 0.25f;
        float rightX = cardW * 0.25f;
        float teamBoxW = std::min(1.15f, cardW * 0.44f);
        float teamHeaderY = 0.32f;
        float rowStartY = 0.22f;
        float rowH = 0.12f;

        auto renderTeamColumn = [&](const std::string& team, float x, float r, float g, float b) {
            UIDrawHelpers::drawQuad(shader, ortho, x, teamHeaderY, teamBoxW, 0.025f, r, g, b, 0.95f);

            size_t rowIndex = 0;
            for (const auto& player : currentRoomPlayers) {
                if (player.team != team || rowIndex >= 4) continue;
                float rowY = rowStartY - static_cast<float>(rowIndex) * 0.14f;

                ColorRGBA rowBg = UITheme::TEAM_ROW_BG;
                ColorRGBA ownerBorder = UITheme::OWNER_BORDER;

                UIDrawHelpers::drawQuad(shader, ortho, x, rowY, teamBoxW, rowH, rowBg.r, rowBg.g, rowBg.b, rowBg.a);

                const float border = 0.009f;
                float br = player.isOwner ? ownerBorder.r : r;
                float bg_col = player.isOwner ? ownerBorder.g : g;
                float bb = player.isOwner ? ownerBorder.b : b;

                UIDrawHelpers::drawQuad(shader, ortho, x, rowY + rowH * 0.5f, teamBoxW, border, br, bg_col, bb, 1.0f);
                UIDrawHelpers::drawQuad(shader, ortho, x, rowY - rowH * 0.5f, teamBoxW, border, br, bg_col, bb, 1.0f);
                UIDrawHelpers::drawQuad(shader, ortho, x - teamBoxW * 0.5f, rowY, border, rowH, br, bg_col, bb, 1.0f);
                UIDrawHelpers::drawQuad(shader, ortho, x + teamBoxW * 0.5f, rowY, border, rowH, br, bg_col, bb, 1.0f);

                std::string displayName = player.name;
                if (displayName.empty() && player.isLocal) displayName = userNickname;
                if (displayName.empty()) displayName = "PLAYER";
                UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, x, rowY, displayName, 0.098f, teamBoxW - 0.20f, 1.0f, 1.0f, 1.0f, 1.0f);

                if (player.isLocal && !roomMatchActive) {
                    const float arrowX = team == "BLUE" ? x + teamBoxW * 0.5f - 0.10f : x - teamBoxW * 0.5f + 0.10f;
                    const float arrowY = rowY - 0.005f;
                    UIDrawHelpers::drawCursorSprite(shader, ortho, arrowX, arrowY, team, team == "BLUE",
                                                    arrowBlueLeft, arrowBlueRight, arrowRedLeft, arrowRedRight);
                }
                rowIndex++;
            }
        };

        renderTeamColumn("BLUE", leftX, 0.20f, 0.75f, 1.0f);
        renderTeamColumn("RED", rightX, 1.0f, 0.30f, 0.30f);

        // Player count bar
        float countY = -0.32f;
        UIButtonSpec countSpec;
        countSpec.x = 0.0f;
        countSpec.y = countY;
        countSpec.w = 1.40f;
        countSpec.h = 0.14f;
        countSpec.text = Strings::get(StringId::PLAYERS_COUNT) + ": " + std::to_string(connectedPlayerCount) + "/8";
        countSpec.bgColor = ColorRGBA(0.12f, 0.16f, 0.24f, 0.95f);
        countSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        countSpec.textColor = ColorRGBA(0.20f, 0.90f, 1.0f, 1.0f);
        countSpec.fontSize = 0.105f;
        countSpec.isClickable = false;
        UIButton::render(shader, ortho, fontRenderer, countSpec);

        // Start match / waiting status button
        float startY = -0.54f, startW = 1.15f, startH = 0.18f;
        const float requiredW = 1.65f;
        UIButtonSpec startSpec;
        startSpec.x = 0.0f;
        startSpec.y = startY;
        startSpec.w = startW;
        startSpec.h = startH;

        if (roomMatchActive) {
            startSpec.text = Strings::get(StringId::MATCH_IN_PROGRESS);
            startSpec.bgColor = ColorRGBA(0.12f, 0.18f, 0.28f, 0.92f);
            startSpec.borderColor = UITheme::CARD_BORDER_CYAN;
            startSpec.textColor = ColorRGBA(1.0f, 0.75f, 0.20f, 1.0f);
            startSpec.fontSize = 0.100f;
        } else if (isOwner && connectedPlayerCount >= 2) {
            startSpec.text = Strings::get(StringId::LOBBY_START_MATCH) + " (" + std::to_string(connectedPlayerCount) + "/8)";
            startSpec.bgColor = UITheme::BTN_SUCCESS_BG;
            startSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
            startSpec.textColor = UITheme::TEXT_WHITE;
            startSpec.fontSize = 0.105f;
        } else if (isOwner) {
            startSpec.w = requiredW;
            startSpec.text = Strings::get(StringId::LOBBY_REQUIRED_PLAYERS);
            startSpec.bgColor = ColorRGBA(0.12f, 0.18f, 0.28f, 0.95f);
            startSpec.borderColor = UITheme::CARD_BORDER_CYAN;
            startSpec.textColor = ColorRGBA(1.0f, 0.85f, 0.30f, 1.0f);
            startSpec.fontSize = 0.100f;
        } else {
            startSpec.text = Strings::get(StringId::WAITING_CREATOR);
            startSpec.bgColor = ColorRGBA(0.12f, 0.18f, 0.28f, 0.92f);
            startSpec.borderColor = UITheme::CARD_BORDER_CYAN;
            startSpec.textColor = ColorRGBA(0.90f, 0.95f, 1.0f, 1.0f);
            startSpec.fontSize = 0.096f;
        }
        UIButton::render(shader, ortho, fontRenderer, startSpec);

        // SALIR DE SALA Button
        UIDrawHelpers::renderBackButton(shader, ortho, fontRenderer, aspect);
    }
};

#endif // TOUCHPARTY_UIROOMLOBBYSCREEN_H
