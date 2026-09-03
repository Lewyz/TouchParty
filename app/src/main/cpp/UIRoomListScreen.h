#ifndef TOUCHPARTY_UIROOMLISTSCREEN_H
#define TOUCHPARTY_UIROOMLISTSCREEN_H

#include <string>
#include <vector>
#include <algorithm>
#include <GLES3/gl3.h>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "GameUIStructs.h"
#include "Strings.h"
#include "UIScrollContainer.h"

class UIRoomListScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect, float screenWidth, float screenHeight,
                       const FontRenderer& fontRenderer,
                       size_t filterTypeIndex, const std::string& searchQuery,
                       const UIScrollContainer& scrollContainer,
                       const std::vector<ServerRoomEntry>& filteredRooms) {
        float cx = 0.0f, cy = -0.02f;
        float cardW = std::min(2.70f, aspect * 1.85f), cardH = 1.52f;

        // Container card with Title "SERVER BROWSER" / "BUSCADOR DE SALAS"
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::SERVER_BROWSER);
        cardSpec.titleColor = ColorRGBA(0.20f, 0.90f, 1.0f, 1.0f);
        cardSpec.underlineColor = ColorRGBA(0.20f, 0.90f, 1.0f, 0.85f);
        cardSpec.titleFontSize = 0.125f;
        cardSpec.titleOffsetY = 0.06f; // Positioning title nicely near card top
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        // 1. Toolbar at top of panel: Search Bar (left) + Clear [X] + Filter Selector [ALL, PUBLIC, PRIVATE] (right)
        const float toolY = 0.48f, toolH = 0.12f;
        const float searchW = cardW * 0.44f;
        const float searchX = cx - (cardW * 0.5f) + (searchW * 0.5f) + 0.08f;

        const float clearW = 0.12f;
        const float clearX = searchX + (searchW * 0.5f) + (clearW * 0.5f) + 0.015f;

        const float filterW = cardW * 0.28f;
        const float filterX = cx + (cardW * 0.5f) - (filterW * 0.5f) - 0.08f;

        // Search Input Field Box
        std::string searchDisplay = Strings::get(StringId::SEARCH_LABEL) + " | " + (searchQuery.empty() ? "_________" : searchQuery);
        UIButtonSpec searchSpec;
        searchSpec.x = searchX;
        searchSpec.y = toolY;
        searchSpec.w = searchW;
        searchSpec.h = toolH;
        searchSpec.text = searchDisplay;
        searchSpec.bgColor = ColorRGBA(0.10f, 0.14f, 0.22f, 0.95f);
        searchSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        searchSpec.textColor = ColorRGBA(0.90f, 0.95f, 1.00f, 1.00f);
        searchSpec.fontSize = 0.086f;
        searchSpec.isClickable = false;
        UIButton::render(shader, ortho, fontRenderer, searchSpec);

        // Clear Search Button [X]
        UIButtonSpec clearSpec;
        clearSpec.x = clearX;
        clearSpec.y = toolY;
        clearSpec.w = clearW;
        clearSpec.h = toolH;
        clearSpec.text = "[X]";
        clearSpec.bgColor = ColorRGBA(0.35f, 0.08f, 0.08f, 0.95f);
        clearSpec.borderColor = ColorRGBA(0.95f, 0.20f, 0.20f, 1.0f);
        clearSpec.textColor = ColorRGBA(1.0f, 0.40f, 0.40f, 1.0f);
        clearSpec.fontSize = 0.088f;
        UIButton::render(shader, ortho, fontRenderer, clearSpec);

        // Filter selector button [ALL, PUBLIC, PRIVATE]
        std::vector<std::string> filterNames = {
            Strings::get(StringId::FILTER_ALL),
            Strings::get(StringId::FILTER_PUBLIC),
            Strings::get(StringId::FILTER_PRIVATE)
        };

        UIButtonSpec filterSpec;
        filterSpec.x = filterX;
        filterSpec.y = toolY;
        filterSpec.w = filterW;
        filterSpec.h = toolH;
        filterSpec.text = filterNames[filterTypeIndex % 3];
        filterSpec.bgColor = ColorRGBA(0.14f, 0.20f, 0.32f, 0.95f);
        filterSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        filterSpec.textColor = ColorRGBA(0.20f, 0.90f, 1.00f, 1.00f);
        filterSpec.fontSize = 0.090f;
        UIButton::render(shader, ortho, fontRenderer, filterSpec);

        // 2. Sub-header Title: LOBBIES / SALAS (Centered in panel with clean vertical margin)
        float subHeaderX = cx;
        float subHeaderY = 0.26f;
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, subHeaderX, subHeaderY,
                                     Strings::get(StringId::LOBBIES_HEADER), 0.110f, 1.20f,
                                     0.20f, 0.90f, 1.0f);

        // 3. List Box Container & Scissored Items
        const float boxX = cx - 0.02f;
        const float boxY = -0.12f;
        const float boxW = cardW - 0.32f;
        const float boxH = 0.60f;

        // Container Background Box
        UIDrawHelpers::drawQuad(shader, ortho, boxX, boxY, boxW, boxH, 0.08f, 0.11f, 0.18f, 0.95f);
        UIDrawHelpers::drawQuad(shader, ortho, boxX, boxY + boxH * 0.5f, boxW, 0.008f, 0.20f, 0.60f, 0.90f, 0.80f);
        UIDrawHelpers::drawQuad(shader, ortho, boxX, boxY - boxH * 0.5f, boxW, 0.008f, 0.20f, 0.60f, 0.90f, 0.80f);

        if (filteredRooms.empty()) {
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, boxX, boxY + 0.04f,
                                         Strings::get(StringId::EMPTY_ROOMS_LINE1), 0.090f, boxW - 0.10f,
                                         1.0f, 0.45f, 0.35f);
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, boxX, boxY - 0.06f,
                                         Strings::get(StringId::EMPTY_ROOMS_LINE2), 0.082f, boxW - 0.10f,
                                         0.60f, 0.80f, 1.0f);
        } else {
            // Setup Scissor Clipping
            float leftNorm = (boxX - boxW * 0.5f + aspect) / (2.0f * aspect);
            float bottomNorm = (boxY - boxH * 0.5f + 1.0f) / 2.0f;
            float widthNorm = boxW / (2.0f * aspect);
            float heightNorm = boxH / 2.0f;

            auto scissorX = static_cast<GLint>(std::max(0.0f, leftNorm * screenWidth));
            auto scissorY = static_cast<GLint>(std::max(0.0f, bottomNorm * screenHeight));
            auto scissorW = static_cast<GLint>(widthNorm * screenWidth);
            auto scissorH = static_cast<GLint>(heightNorm * screenHeight);

            glEnable(GL_SCISSOR_TEST);
            glScissor(scissorX, scissorY, scissorW, scissorH);

            const float itemH = 0.13f;
            const float rowStep = 0.15f;
            const float scrollOffset = scrollContainer.getScrollOffset();

            const float infoW = boxW * 0.68f;
            const float infoX = boxX - (boxW * 0.5f) + (infoW * 0.5f) + 0.02f;
            const float actionW = boxW * 0.26f;
            const float actionX = boxX + (boxW * 0.5f) - (actionW * 0.5f) - 0.02f;

            for (size_t i = 0; i < filteredRooms.size(); ++i) {
                const auto& room = filteredRooms[i];
                float rowY = (boxY + boxH * 0.5f - itemH * 0.5f - 0.015f) - (static_cast<float>(i) * rowStep) + scrollOffset;

                // Skip if completely outside rendering window
                if (rowY + itemH * 0.5f < boxY - boxH * 0.5f - 0.20f || rowY - itemH * 0.5f > boxY + boxH * 0.5f + 0.20f) {
                    continue;
                }

                // Room Name & Player Count Label
                std::string roomLabel = room.name + " (" + std::to_string(room.playerCount) + "/" + std::to_string(room.maxPlayers) + ")";
                UIButtonSpec roomInfoSpec;
                roomInfoSpec.x = infoX;
                roomInfoSpec.y = rowY;
                roomInfoSpec.w = infoW;
                roomInfoSpec.h = itemH;
                roomInfoSpec.text = roomLabel;
                roomInfoSpec.bgColor = ColorRGBA(0.12f, 0.16f, 0.24f, 0.95f);
                roomInfoSpec.borderColor = room.isPrivate ? UITheme::CARD_BORDER_GOLD : UITheme::CARD_BORDER_CYAN;
                roomInfoSpec.textColor = room.isPrivate ? UITheme::TEXT_TITLE_GOLD : UITheme::TEXT_WHITE;
                roomInfoSpec.fontSize = 0.088f;
                roomInfoSpec.isClickable = false;
                UIButton::render(shader, ortho, fontRenderer, roomInfoSpec);

                // Right Action: FULL (Red Badge) OR PIN/JOIN (Green Button)
                bool isFull = (room.playerCount >= room.maxPlayers);
                if (isFull) {
                    UIButtonSpec fullSpec;
                    fullSpec.x = actionX;
                    fullSpec.y = rowY;
                    fullSpec.w = actionW;
                    fullSpec.h = itemH;
                    fullSpec.text = Strings::get(StringId::FULL_BADGE);
                    fullSpec.bgColor = ColorRGBA(0.35f, 0.08f, 0.08f, 0.95f);
                    fullSpec.borderColor = ColorRGBA(0.95f, 0.20f, 0.20f, 1.0f);
                    fullSpec.textColor = ColorRGBA(1.0f, 0.30f, 0.30f, 1.0f);
                    fullSpec.fontSize = 0.088f;
                    fullSpec.isClickable = false;
                    UIButton::render(shader, ortho, fontRenderer, fullSpec);
                } else {
                    std::string btnText = room.isPrivate ? Strings::get(StringId::PIN_LABEL_WORD) : Strings::get(StringId::JOIN_ROOM);
                    UIButtonSpec joinSpec;
                    joinSpec.x = actionX;
                    joinSpec.y = rowY;
                    joinSpec.w = actionW;
                    joinSpec.h = itemH;
                    joinSpec.text = btnText;
                    joinSpec.bgColor = UITheme::BTN_SUCCESS_BG;
                    joinSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
                    joinSpec.textColor = UITheme::TEXT_WHITE;
                    joinSpec.fontSize = 0.088f;
                    UIButton::render(shader, ortho, fontRenderer, joinSpec);
                }
            }

            glDisable(GL_SCISSOR_TEST);
        }

        // 4. Render Vertical Scrollbar
        float trackX = boxX + (boxW * 0.5f) + 0.035f;
        float trackY = boxY;
        float trackW = 0.032f;
        float trackH = boxH;
        scrollContainer.renderScrollbar(shader, ortho, trackX, trackY, trackW, trackH);

        // 5. REFRESH Button
        float refX = 0.0f, refY = -0.52f, refW = 0.85f, refH = 0.15f;
        UIButtonSpec refSpec;
        refSpec.x = refX;
        refSpec.y = refY;
        refSpec.w = refW;
        refSpec.h = refH;
        refSpec.text = Strings::get(StringId::REFRESH);
        refSpec.bgColor = UITheme::BTN_PRIMARY_BG;
        refSpec.borderColor = UITheme::BTN_PRIMARY_BORDER;
        refSpec.textColor = UITheme::TEXT_WHITE;
        refSpec.fontSize = 0.100f;
        UIButton::render(shader, ortho, fontRenderer, refSpec);

        // 6. VOLVER Button
        UIDrawHelpers::renderBackButton(shader, ortho, fontRenderer, aspect);
    }
};

#endif // TOUCHPARTY_UIROOMLISTSCREEN_H
