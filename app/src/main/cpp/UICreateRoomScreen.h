#ifndef TOUCHPARTY_UICREATEROOMSCREEN_H
#define TOUCHPARTY_UICREATEROOMSCREEN_H

#include <string>
#include <cctype>
#include "Shader.h"
#include "FontRenderer.h"
#include "UITheme.h"
#include "UIButton.h"
#include "UICard.h"
#include "UIDrawHelpers.h"
#include "Strings.h"

class UICreateRoomScreen {
public:
    static std::string sanitizeToUppercase(const std::string& str) {
        std::string out;
        out.reserve(str.size());
        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = static_cast<unsigned char>(str[i]);
            if (c < 0x80) {
                char ch = static_cast<char>(c);
                if (ch >= 'a' && ch <= 'z') {
                    out += static_cast<char>(ch - 'a' + 'A');
                } else {
                    out += ch;
                }
                i += 1;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
                unsigned char c2 = static_cast<unsigned char>(str[i + 1]);
                uint32_t cp = ((c & 0x1F) << 6) | (c2 & 0x3F);
                switch (cp) {
                    case 0x00E1: case 0x00E0: case 0x00E4: case 0x00E2:
                    case 0x00C1: case 0x00C0: case 0x00C4: case 0x00C2:
                        out += 'A'; break;
                    case 0x00E9: case 0x00E8: case 0x00EB: case 0x00EA:
                    case 0x00C9: case 0x00C8: case 0x00CB: case 0x00CA:
                        out += 'E'; break;
                    case 0x00ED: case 0x00EC: case 0x00EF: case 0x00EE:
                    case 0x00CD: case 0x00CC: case 0x00CF: case 0x00CE:
                        out += 'I'; break;
                    case 0x00F3: case 0x00F2: case 0x00F6: case 0x00F4:
                    case 0x00D3: case 0x00D2: case 0x00D6: case 0x00D4:
                        out += 'O'; break;
                    case 0x00FA: case 0x00F9: case 0x00FC: case 0x00FB:
                    case 0x00DA: case 0x00D9: case 0x00DC: case 0x00DB:
                        out += 'U'; break;
                    case 0x00F1: case 0x00D1:
                        out += "\xC3\x91"; break; // Ñ
                    default:
                        out += static_cast<char>(c);
                        out += static_cast<char>(c2);
                        break;
                }
                i += 2;
            } else {
                out += static_cast<char>(c);
                i += 1;
            }
        }
        return out;
    }

    static void render(const Shader& shader, const float* ortho, float aspect,
                       const FontRenderer& fontRenderer,
                       const std::string& roomName, bool isPrivateRoom, const std::string& roomPin,
                       const std::string& userNickname) {
        float cx = 0.0f, cy = 0.00f;
        float cardW = 2.05f, cardH = 1.60f;
        const float margin = 0.14f;
        const float innerW = cardW - margin * 2.0f;
        const float fieldH = 0.17f;

        // 1. Container card with header title centered inside top margin
        UICardSpec cardSpec;
        cardSpec.x = cx;
        cardSpec.y = cy;
        cardSpec.w = cardW;
        cardSpec.h = cardH;
        cardSpec.title = Strings::get(StringId::CREATE_NEW_ROOM_TITLE);
        cardSpec.titleFontSize = 0.125f;
        cardSpec.titleOffsetY = -0.03f;
        UICard::render(shader, ortho, fontRenderer, cardSpec);

        float boxX = 0.0f, boxW = innerW;
        float nameY = 0.38f;
        float togY = 0.18f;
        float pinY = -0.02f;

        // 2. Room Name String (Uppercase, no prefix, normalized accents)
        std::string cleanNick = sanitizeToUppercase(userNickname);
        std::string displayName = sanitizeToUppercase(roomName);

        if (displayName.empty() || displayName == "LOBBY" || displayName == "LOBB") {
            displayName = cleanNick.empty() ? "PLAYER_LOBBY" : (cleanNick + "_LOBBY");
        }

        // Draw Room Name String starting from left side (fontSize = 0.092f, maxTextW = 1.35f)
        float nameTextX = -0.28f;
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, nameTextX, nameY,
                                     displayName, 0.092f, 1.35f, 1.0f, 1.0f, 1.0f);

        // Draw EDIT button on right side
        std::string editBtnLabel = Strings::getLanguage() == Language::SPANISH ? "EDITAR" : "EDIT";
        UIButtonSpec editBtnSpec;
        editBtnSpec.x = 0.58f;
        editBtnSpec.y = nameY;
        editBtnSpec.w = 0.38f;
        editBtnSpec.h = 0.14f;
        editBtnSpec.text = editBtnLabel;
        editBtnSpec.bgColor = UITheme::CARD_BG;
        editBtnSpec.borderColor = UITheme::CARD_BORDER_CYAN;
        editBtnSpec.textColor = UITheme::TEXT_TITLE_GOLD;
        editBtnSpec.fontSize = 0.088f;
        editBtnSpec.isClickable = false;
        UIButton::render(shader, ortho, fontRenderer, editBtnSpec);

        // 3. Privacy Toggle Button (TYPE: PUBLIC / PRIVATE)
        float r = isPrivateRoom ? 0.62f : 0.10f;
        float g = isPrivateRoom ? 0.38f : 0.52f;
        float b = isPrivateRoom ? 0.10f : 0.85f;
        std::string typeStr = Strings::get(StringId::TYPE_LABEL) +
            (isPrivateRoom ? Strings::get(StringId::PRIVATE_LABEL) : Strings::get(StringId::PUBLIC_LABEL));
        UIButtonSpec togSpec;
        togSpec.x = boxX;
        togSpec.y = togY;
        togSpec.w = boxW;
        togSpec.h = fieldH;
        togSpec.text = typeStr;
        togSpec.bgColor = ColorRGBA(r, g, b, 0.97f);
        togSpec.borderColor = UITheme::CARD_BORDER_GOLD;
        togSpec.textColor = UITheme::TEXT_WHITE;
        togSpec.fontSize = 0.102f;
        togSpec.isClickable = false;
        UIButton::render(shader, ortho, fontRenderer, togSpec);

        // 4. Password / PIN Box (ONLY SHOWN IF PRIVATE ROOM!)
        if (isPrivateRoom) {
            std::string pinText = roomPin.empty() ? "" : Strings::get(StringId::PIN_LABEL) + roomPin;
            renderInputField(shader, ortho, fontRenderer, boxX, pinY, boxW, fieldH,
                             pinText, Strings::get(StringId::PROMPT_ROOM_PIN));
        }

        // 5. Rules & Info Label
        float infoY = isPrivateRoom ? -0.22f : -0.02f;
        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, cx, infoY,
                                     Strings::get(StringId::MAX_PLAYERS_NOTE), 0.082f, innerW, 0.60f, 0.80f, 1.0f);

        // 6. CREATE ROOM Button
        float createX = 0.0f, createY = -0.54f, createW = innerW, createH = 0.18f;
        UIButtonSpec createSpec;
        createSpec.x = createX;
        createSpec.y = createY;
        createSpec.w = createW;
        createSpec.h = createH;
        createSpec.text = Strings::get(StringId::CREATE_ROOM);
        createSpec.bgColor = UITheme::BTN_SUCCESS_BG;
        createSpec.borderColor = UITheme::BTN_SUCCESS_BORDER;
        createSpec.textColor = UITheme::TEXT_WHITE;
        createSpec.fontSize = 0.118f;
        UIButton::render(shader, ortho, fontRenderer, createSpec);

        // 7. VOLVER / BACK Button
        UIDrawHelpers::renderBackButton(shader, ortho, fontRenderer, aspect);
    }

private:
    static void renderInputField(const Shader& shader, const float* ortho, const FontRenderer& fontRenderer,
                                 float x, float y, float w, float h,
                                 const std::string& text, const std::string& placeholder) {
        float bd = 0.012f;
        ColorRGBA bg = ColorRGBA(0.14f, 0.20f, 0.32f, 0.98f);
        ColorRGBA border = UITheme::CARD_BORDER_CYAN;

        UIDrawHelpers::drawQuad(shader, ortho, x, y, w, h, bg);

        UIDrawHelpers::drawQuad(shader, ortho, x, y + h * 0.5f, w, bd, border);
        UIDrawHelpers::drawQuad(shader, ortho, x, y - h * 0.5f, w, bd, border);
        UIDrawHelpers::drawQuad(shader, ortho, x - w * 0.5f, y, bd, h, border);
        UIDrawHelpers::drawQuad(shader, ortho, x + w * 0.5f, y, bd, h, border);

        std::string display = text.empty() ? placeholder : text;
        ColorRGBA textColor = text.empty() ? ColorRGBA(0.65f, 0.82f, 1.0f, 0.90f) : UITheme::TEXT_WHITE;

        UIDrawHelpers::drawTextFitted(shader, ortho, fontRenderer, x, y, display, 0.105f, w - 0.20f,
                                     textColor.r, textColor.g, textColor.b, textColor.a);
    }
};

#endif // TOUCHPARTY_UICREATEROOMSCREEN_H
