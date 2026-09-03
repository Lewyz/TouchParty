#ifndef TOUCHPARTY_UITHEME_H
#define TOUCHPARTY_UITHEME_H

enum class UIThemeMode {
    LIGHT,
    DARK
};

struct ColorRGBA {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    ColorRGBA() = default;
    constexpr ColorRGBA(float r_, float g_, float b_, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}
};

class UITheme {
public:
    static UIThemeMode currentMode;

    // Badge / NickName Chip Colors
    static constexpr ColorRGBA BADGE_BG_LIGHT{1.0f, 1.0f, 1.0f, 1.0f};           // ⚪ White chip
    static constexpr ColorRGBA BADGE_TEXT_LIGHT{0.05f, 0.08f, 0.12f, 1.0f};       // ⚫ High-contrast dark text
    static constexpr ColorRGBA BADGE_ACCENT_LIGHT{0.20f, 0.70f, 1.0f, 1.0f};     // 🔵 Cyan accent line

    static constexpr ColorRGBA BADGE_BG_DARK{0.08f, 0.12f, 0.20f, 0.85f};         // ⬛ Semi-transparent dark/black chip
    static constexpr ColorRGBA BADGE_TEXT_DARK{1.00f, 0.90f, 0.20f, 1.00f};       // 🟡 Bright gold yellow text
    static constexpr ColorRGBA BADGE_ACCENT_DARK{0.10f, 0.70f, 1.00f, 0.95f};      // 🔵 Bright cyan accent line

    // Lobby Team Row Colors
    static constexpr ColorRGBA TEAM_ROW_BG{0.10f, 0.16f, 0.25f, 0.98f};          // 🔵 Dark blue row
    static constexpr ColorRGBA TEAM_ROW_TEXT{1.0f, 1.0f, 1.0f, 1.0f};            // ⚪ White text
    static constexpr ColorRGBA OWNER_BORDER{1.0f, 0.85f, 0.10f, 1.0f};           // 🟡 Gold border

    // Container Cards
    static constexpr ColorRGBA CARD_BG{0.07f, 0.10f, 0.17f, 0.96f};              // 🔵 Dark slate card
    static constexpr ColorRGBA CARD_BORDER_CYAN{0.10f, 0.70f, 1.0f, 0.95f};       // 🔵 Cyan border
    static constexpr ColorRGBA CARD_BORDER_GOLD{0.95f, 0.70f, 0.10f, 0.85f};       // 🟡 Gold border

    // Button Colors
    static constexpr ColorRGBA BTN_SUCCESS_BG{0.10f, 0.55f, 0.40f, 0.95f};       // 🟢 Emerald Green
    static constexpr ColorRGBA BTN_SUCCESS_BORDER{0.40f, 0.95f, 0.70f, 0.95f};   // 🟢 Bright Green Border
    static constexpr ColorRGBA BTN_PRIMARY_BG{0.10f, 0.42f, 0.90f, 0.95f};       // 🔵 Royal Blue
    static constexpr ColorRGBA BTN_PRIMARY_BORDER{0.50f, 0.85f, 1.00f, 0.95f};   // 🔵 Bright Blue Border
    static constexpr ColorRGBA BTN_TEST_BG{0.85f, 0.45f, 0.10f, 0.92f};          // 🟠 Orange Test Button
    static constexpr ColorRGBA BTN_TEST_BORDER{1.00f, 0.70f, 0.20f, 0.95f};      // 🟠 Gold Border

    // Server Status Banner Colors
    static constexpr ColorRGBA SERVER_CONNECTED_BG{0.08f, 0.22f, 0.12f, 0.92f};
    static constexpr ColorRGBA SERVER_CONNECTED_BORDER{0.20f, 0.90f, 0.35f, 0.95f};
    static constexpr ColorRGBA SERVER_CONNECTED_TEXT{0.30f, 1.00f, 0.45f, 1.00f};

    static constexpr ColorRGBA SERVER_DISCONNECTED_BG{0.24f, 0.08f, 0.10f, 0.92f};
    static constexpr ColorRGBA SERVER_DISCONNECTED_BORDER{0.95f, 0.25f, 0.25f, 0.95f};
    static constexpr ColorRGBA SERVER_DISCONNECTED_TEXT{1.00f, 0.40f, 0.40f, 1.00f};

    // Text Colors
    static constexpr ColorRGBA TEXT_TITLE_GOLD{1.0f, 0.90f, 0.20f, 1.0f};        // 🟡 Gold title
    static constexpr ColorRGBA TEXT_WHITE{1.0f, 1.0f, 1.0f, 1.0f};               // ⚪ White text
    static constexpr ColorRGBA TEXT_MUTED_CYAN{0.60f, 0.80f, 1.0f, 1.0f};         // 🔵 Muted cyan text

    // Helper to get active Badge Background
    static ColorRGBA getBadgeBg() {
        return currentMode == UIThemeMode::LIGHT ? BADGE_BG_LIGHT : BADGE_BG_DARK;
    }

    // Helper to get active Badge Text Color
    static ColorRGBA getBadgeText() {
        return currentMode == UIThemeMode::LIGHT ? BADGE_TEXT_LIGHT : BADGE_TEXT_DARK;
    }

    // Helper to get active Badge Accent Line
    static ColorRGBA getBadgeAccent() {
        return currentMode == UIThemeMode::LIGHT ? BADGE_ACCENT_LIGHT : BADGE_ACCENT_DARK;
    }
};

inline UIThemeMode UITheme::currentMode = UIThemeMode::DARK;

#endif // TOUCHPARTY_UITHEME_H
