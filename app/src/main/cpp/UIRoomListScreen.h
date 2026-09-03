#ifndef TOUCHPARTY_UIROOMLISTSCREEN_H
#define TOUCHPARTY_UIROOMLISTSCREEN_H

#include <string>
#include <vector>
#include <algorithm>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "GameUIStructs.h"
#include "Strings.h"

class UIRoomListScreen {
public:
    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer,
                       size_t filterTypeIndex, int roomListScrollOffset,
                       const std::vector<ServerRoomEntry>& filteredRooms) {
        float cx = 0.0f, cy = -0.02f;
        float cardW = std::min(2.65f, aspect * 1.85f), cardH = 1.45f;

        // Container card
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::AVAILABLE_ROOMS_TITLE);
        cardSpec.titleColor = ColorRGBA(0.20f, 0.90f, 1.0f, 1.0f);
        cardSpec.underlineColor = ColorRGBA(0.20f, 0.90f, 1.0f, 0.85f);
        cardSpec.titleFontSize = 0.14f;
        cardSpec.titleOffsetY = 0.54f;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        // Toolbar: Search / Filter (left) + scroll controls (right)
        const float toolY = 0.28f, toolH = 0.14f;
        const float filterX = -0.45f, filterW = 1.15f;
        const float scrollX = 0.75f, btnW = 0.32f, btnH = 0.13f;
        const float upY = 0.28f, downY = 0.12f;

        std::vector<std::string> filterNames = {
            Strings::get(StringId::SEARCH_PREFIX) + Strings::get(StringId::FILTER_ALL),
            Strings::get(StringId::SEARCH_PREFIX) + Strings::get(StringId::FILTER_PUBLIC),
            Strings::get(StringId::SEARCH_PREFIX) + Strings::get(StringId::FILTER_PRIVATE)
        };

        // Filter selector button
        UIButtonSpec filterSpec;
        filterSpec.x = filterX;
        filterSpec.y = toolY;
        filterSpec.w = filterW;
        filterSpec.h = toolH;
        filterSpec.text = filterNames[filterTypeIndex % 3];
        filterSpec.bgColor = ColorRGBA(0.12f, 0.16f, 0.24f, 0.95f);
        filterSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        filterSpec.textColor = ColorRGBA(0.80f, 0.90f, 1.00f, 1.00f);
        filterSpec.fontSize = 0.098f;
        UIButton::render(shader, ortho, fontRenderer, filterSpec);

        // Scroll Up / Down
        UIButtonSpec upSpec;
        upSpec.x = scrollX;
        upSpec.y = upY;
        upSpec.w = btnW;
        upSpec.h = btnH;
        upSpec.text = "UP ^";
        upSpec.bgColor = ColorRGBA(0.14f, 0.22f, 0.35f, 0.95f);
        upSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        upSpec.textColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
        upSpec.fontSize = 0.090f;
        UIButton::render(shader, ortho, fontRenderer, upSpec);

        UIButtonSpec downSpec;
        downSpec.x = scrollX;
        downSpec.y = downY;
        downSpec.w = btnW;
        downSpec.h = btnH;
        downSpec.text = "DWN v";
        downSpec.bgColor = ColorRGBA(0.14f, 0.22f, 0.35f, 0.95f);
        downSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        downSpec.textColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
        downSpec.fontSize = 0.090f;
        UIButton::render(shader, ortho, fontRenderer, downSpec);

        // Content area
        const float contentH = 0.40f;

        if (filteredRooms.empty()) {
            float emptyY = -0.16f;
            float emptyW = cardW - 0.28f;
            UIDrawHelpers::drawQuad(shader, ortho, 0.0f, emptyY, emptyW, contentH, 0.12f, 0.15f, 0.22f, 0.94f);
            UIDrawHelpers::drawQuad(shader, ortho, 0.0f, emptyY + contentH * 0.5f, emptyW, 0.010f, 0.40f, 0.60f, 0.90f, 0.70f);
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, 0.0f, emptyY + 0.08f, Strings::get(StringId::EMPTY_ROOMS_LINE1), 0.096f, emptyW - 0.20f, 1.0f, 0.45f, 0.35f);
            UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, 0.0f, emptyY - 0.06f, Strings::get(StringId::EMPTY_ROOMS_LINE2), 0.088f, emptyW - 0.20f, 0.60f, 0.80f, 1.0f);
        } else {
            float startY = 0.05f;
            size_t maxVisible = 3;
            size_t startIndex = std::min(static_cast<size_t>(roomListScrollOffset), filteredRooms.size() - 1);
            const float infoX = -0.22f, infoW = 1.35f, infoH = 0.15f;
            const float joinX = 0.65f, joinW = 0.38f, joinH = 0.15f;

            for (size_t i = 0; i < maxVisible && (startIndex + i) < filteredRooms.size(); ++i) {
                const auto& room = filteredRooms[startIndex + i];
                float rowY = startY - static_cast<float>(i) * 0.17f;

                std::string label = room.name + " (" + std::to_string(room.playerCount) + "/" + std::to_string(room.maxPlayers) + ")";
                UIButtonSpec roomInfoSpec;
                roomInfoSpec.x = infoX;
                roomInfoSpec.y = rowY;
                roomInfoSpec.w = infoW;
                roomInfoSpec.h = infoH;
                roomInfoSpec.text = label;
                roomInfoSpec.bgColor = ColorRGBA(0.12f, 0.16f, 0.24f, 0.95f);
                roomInfoSpec.borderColor = room.isPrivate ? UITheme::CARD_BORDER_GOLD : UITheme::CARD_BORDER_CYAN;
                roomInfoSpec.textColor = room.isPrivate ? UITheme::TEXT_TITLE_GOLD : UITheme::TEXT_WHITE;
                roomInfoSpec.fontSize = 0.088f;
                roomInfoSpec.isClickable = false;
                UIButton::render(shader, ortho, fontRenderer, roomInfoSpec);

                std::string btnText = room.isPrivate ? Strings::get(StringId::PIN_LABEL_WORD) : Strings::get(StringId::JOIN_ROOM);
                UIButtonSpec joinSpec;
                joinSpec.x = joinX;
                joinSpec.y = rowY;
                joinSpec.w = joinW;
                joinSpec.h = joinH;
                joinSpec.text = btnText;
                joinSpec.bgColor = UITheme::BTN_SUCCESS_BG;
                joinSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
                joinSpec.textColor = UITheme::TEXT_WHITE;
                joinSpec.fontSize = 0.088f;
                UIButton::render(shader, ortho, fontRenderer, joinSpec);
            }
        }

        // REFRESH Button
        float refX = 0.0f, refY = -0.50f, refW = 0.85f, refH = 0.16f;
        UIButtonSpec refSpec;
        refSpec.x = refX;
        refSpec.y = refY;
        refSpec.w = refW;
        refSpec.h = refH;
        refSpec.text = Strings::get(StringId::REFRESH);
        refSpec.bgColor = UITheme::BTN_PRIMARY_BG;
        refSpec.borderColor = UITheme::BTN_PRIMARY_BORDER;
        refSpec.textColor = UITheme::TEXT_WHITE;
        refSpec.fontSize = 0.105f;
        UIButton::render(shader, ortho, fontRenderer, refSpec);

        // VOLVER Button
        UIDrawHelpers::renderBackButton(shader, ortho, fontRenderer, aspect);
    }
};

#endif // TOUCHPARTY_UIROOMLISTSCREEN_H
