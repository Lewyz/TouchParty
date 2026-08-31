#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "FontRenderer.h"
#include "AndroidOut.h"
#include "MatrixMath.h"

FontRenderer::FontRenderer()
    : textureID_(0), atlasWidth_(1024), atlasHeight_(1024), fontPixelHeight_(64.0f), isLoaded_(false) {}

FontRenderer::~FontRenderer() {
    if (textureID_ != 0) {
        glDeleteTextures(1, &textureID_);
        textureID_ = 0;
    }
}

uint32_t FontRenderer::getFallbackCodepoint(uint32_t cp) {
    switch (cp) {
        // Uppercase Spanish Accents
        case 0x00C1: return 'A'; // Á
        case 0x00C9: return 'E'; // É
        case 0x00CD: return 'I'; // Í
        case 0x00D3: return 'O'; // Ó
        case 0x00DA: // Ú
        case 0x00DC: return 'U'; // Ü
        case 0x00D1: return 'N'; // Ñ

        // Lowercase Spanish Accents
        case 0x00E1: return 'a'; // á
        case 0x00E9: return 'e'; // é
        case 0x00ED: return 'i'; // í
        case 0x00F3: return 'o'; // ó
        case 0x00FA: // ú
        case 0x00FC: return 'u'; // ü
        case 0x00F1: return 'n'; // ñ

        default: break;
    }
    // Lowercase to Uppercase fallback
    if (cp >= 'a' && cp <= 'z') {
        return 'A' + (cp - 'a');
    }
    return 0;
}

const FontRenderer::GlyphInfo* FontRenderer::getGlyph(uint32_t cp) const {
    auto it = glyphMap_.find(cp);
    if (it != glyphMap_.end()) {
        return &it->second;
    }
    uint32_t fb = getFallbackCodepoint(cp);
    while (fb != 0) {
        auto itFb = glyphMap_.find(fb);
        if (itFb != glyphMap_.end()) {
            return &itFb->second;
        }
        fb = getFallbackCodepoint(fb);
    }
    auto itQuestion = glyphMap_.find(static_cast<uint32_t>('?'));
    if (itQuestion != glyphMap_.end()) {
        return &itQuestion->second;
    }
    return nullptr;
}

std::vector<uint32_t> FontRenderer::decodeUTF8(const std::string& text) {
    std::vector<uint32_t> codepoints;
    size_t i = 0;
    while (i < text.length()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c < 0x80) {
            codepoints.push_back(c);
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 < text.length()) {
                uint32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3F);
                codepoints.push_back(cp);
                i += 2;
            } else {
                break;
            }
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 < text.length()) {
                uint32_t cp = ((c & 0x0F) << 12) |
                              ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) |
                              (static_cast<unsigned char>(text[i + 2]) & 0x3F);
                codepoints.push_back(cp);
                i += 3;
            } else {
                break;
            }
        } else {
            i += 1;
        }
    }
    return codepoints;
}

bool FontRenderer::loadFont(AAssetManager* assetManager, const std::string& assetPath, float fontPixelHeight) {
    if (!assetManager) {
        aout << "AssetManager is null in loadFont!" << std::endl;
        return false;
    }

    fontPixelHeight_ = fontPixelHeight;

    AAsset* fontAsset = AAssetManager_open(assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
    if (!fontAsset) {
        aout << "Failed to open font asset: " << assetPath << std::endl;
        return false;
    }

    off_t fontFileSize = AAsset_getLength(fontAsset);
    std::vector<uint8_t> fontBuffer(fontFileSize);
    int readBytes = AAsset_read(fontAsset, fontBuffer.data(), fontFileSize);
    AAsset_close(fontAsset);

    if (readBytes != fontFileSize) {
        aout << "Failed to read full font file: " << assetPath << std::endl;
        return false;
    }

    stbtt_fontinfo fontInfo;
    bool hasFontInfo = (stbtt_InitFont(&fontInfo, fontBuffer.data(), 0) != 0);

    stbtt_pack_context packContext;
    std::vector<uint8_t> alphaPixels(atlasWidth_ * atlasHeight_, 0);

    if (!stbtt_PackBegin(&packContext, alphaPixels.data(), atlasWidth_, atlasHeight_, 0, 1, nullptr)) {
        aout << "stbtt_PackBegin failed!" << std::endl;
        return false;
    }

    // Pack ASCII (32 to 126) and Latin-1 Supplement (160 to 255)
    stbtt_packedchar asciiChars[95];
    stbtt_packedchar latinChars[96];
    stbtt_pack_range ranges[2];

    ranges[0].font_size = fontPixelHeight_;
    ranges[0].first_unicode_codepoint_in_range = 32;
    ranges[0].array_of_unicode_codepoints = nullptr;
    ranges[0].num_chars = 95;
    ranges[0].chardata_for_range = asciiChars;

    ranges[1].font_size = fontPixelHeight_;
    ranges[1].first_unicode_codepoint_in_range = 160;
    ranges[1].array_of_unicode_codepoints = nullptr;
    ranges[1].num_chars = 96;
    ranges[1].chardata_for_range = latinChars;

    stbtt_PackFontRanges(&packContext, fontBuffer.data(), 0, ranges, 2);
    stbtt_PackEnd(&packContext);

    // Convert alpha pixels to RGBA (white color with alpha channel)
    std::vector<uint8_t> rgbaPixels(atlasWidth_ * atlasHeight_ * 4);
    for (size_t i = 0; i < alphaPixels.size(); ++i) {
        uint8_t alpha = alphaPixels[i];
        rgbaPixels[i * 4 + 0] = 255;
        rgbaPixels[i * 4 + 1] = 255;
        rgbaPixels[i * 4 + 2] = 255;
        rgbaPixels[i * 4 + 3] = alpha;
    }

    glGenTextures(1, &textureID_);
    glBindTexture(GL_TEXTURE_2D, textureID_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        atlasWidth_, atlasHeight_, 0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        rgbaPixels.data()
    );

    glyphMap_.clear();
    for (int i = 0; i < 95; ++i) {
        uint32_t cp = 32 + i;
        const auto& pc = asciiChars[i];
        GlyphInfo g;
        g.x0 = pc.xoff;
        g.y0 = pc.yoff;
        g.x1 = pc.xoff2;
        g.y1 = pc.yoff2;
        g.s0 = static_cast<float>(pc.x0) / atlasWidth_;
        g.t0 = static_cast<float>(pc.y0) / atlasHeight_;
        g.s1 = static_cast<float>(pc.x1) / atlasWidth_;
        g.t1 = static_cast<float>(pc.y1) / atlasHeight_;
        g.xAdvance = pc.xadvance;
        glyphMap_[cp] = g;
    }

    for (int i = 0; i < 96; ++i) {
        uint32_t cp = 160 + i;
        const auto& pc = latinChars[i];
        GlyphInfo g;
        g.x0 = pc.xoff;
        g.y0 = pc.yoff;
        g.x1 = pc.xoff2;
        g.y1 = pc.yoff2;
        g.s0 = static_cast<float>(pc.x0) / atlasWidth_;
        g.t0 = static_cast<float>(pc.y0) / atlasHeight_;
        g.s1 = static_cast<float>(pc.x1) / atlasWidth_;
        g.t1 = static_cast<float>(pc.y1) / atlasHeight_;
        g.xAdvance = pc.xadvance;
        glyphMap_[cp] = g;
    }

    auto isGlyphValid = [&](uint32_t cp) -> bool {
        if (hasFontInfo && stbtt_FindGlyphIndex(&fontInfo, static_cast<int>(cp)) == 0) {
            return false;
        }
        auto it = glyphMap_.find(cp);
        if (it == glyphMap_.end()) return false;
        if (cp == 32) return true;
        const auto& g = it->second;
        if (g.x0 == g.x1 && g.y0 == g.y1 && g.xAdvance == 0.0f) {
            return false;
        }
        return true;
    };

    for (auto& pair : glyphMap_) {
        uint32_t cp = pair.first;
        if (!isGlyphValid(cp)) {
            uint32_t fb = getFallbackCodepoint(cp);
            while (fb != 0 && !isGlyphValid(fb)) {
                fb = getFallbackCodepoint(fb);
            }
            if (fb != 0 && isGlyphValid(fb)) {
                pair.second = glyphMap_[fb];
            } else if (cp == 0x00A1 || cp == 0x00BF) {
                pair.second = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            }
        }
    }

    isLoaded_ = true;
    aout << "Font " << assetPath << " loaded successfully with stb_truetype!" << std::endl;
    return true;
}

float FontRenderer::getTextWidth(const std::string& text, float fontSize) const {
    if (!isLoaded_) return static_cast<float>(text.length()) * fontSize * 0.6f;

    std::vector<uint32_t> cps = decodeUTF8(text);
    float scale = fontSize / fontPixelHeight_;
    float totalWidth = 0.0f;

    for (uint32_t cp : cps) {
        const GlyphInfo* g = getGlyph(cp);
        if (g != nullptr) {
            totalWidth += g->xAdvance * scale;
        } else {
            totalWidth += (fontPixelHeight_ * 0.5f) * scale;
        }
    }
    return totalWidth;
}

void FontRenderer::drawText(const Shader& shader, const float* ortho, float cx, float cy,
                           const std::string& text, float fontSize,
                           float r, float g, float b, float a, bool centered) const {
    if (!isLoaded_ || text.empty() || textureID_ == 0) return;

    std::vector<uint32_t> cps = decodeUTF8(text);
    float scale = fontSize / fontPixelHeight_;
    float totalWidth = getTextWidth(text, fontSize);

    float currentX = centered ? (cx - totalWidth * 0.5f) : cx;
    float baselineY = cy;

    shader.setUseTexture(true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID_);

    static const uint16_t quadIndices[6] = { 0, 1, 2, 0, 2, 3 };

    for (uint32_t cp : cps) {
        const GlyphInfo* gPtr = getGlyph(cp);
        if (gPtr != nullptr) {
            const GlyphInfo& glyph = *gPtr;

            float x0 = currentX + glyph.x0 * scale;
            float y0 = baselineY - glyph.y0 * scale;
            float x1 = currentX + glyph.x1 * scale;
            float y1 = baselineY - glyph.y1 * scale;

            float dx = scale * 1.8f;

            Vertex v[4] = {
                Vertex(Vector3{x0, y0, 0.0f}, Vector2{glyph.s0, glyph.t0}),
                Vertex(Vector3{x1, y0, 0.0f}, Vector2{glyph.s1, glyph.t0}),
                Vertex(Vector3{x1, y1, 0.0f}, Vector2{glyph.s1, glyph.t1}),
                Vertex(Vector3{x0, y1, 0.0f}, Vector2{glyph.s0, glyph.t1})
            };

            Vertex vBold[4] = {
                Vertex(Vector3{x0 + dx, y0, 0.0f}, Vector2{glyph.s0, glyph.t0}),
                Vertex(Vector3{x1 + dx, y0, 0.0f}, Vector2{glyph.s1, glyph.t0}),
                Vertex(Vector3{x1 + dx, y1, 0.0f}, Vector2{glyph.s1, glyph.t1}),
                Vertex(Vector3{x0 + dx, y1, 0.0f}, Vector2{glyph.s0, glyph.t1})
            };

            float model[16];
            MatrixMath::identity(model);

            float mvp[16];
            MatrixMath::multiply(mvp, ortho, model);
            shader.setProjectionMatrix(mvp);
            shader.setColor(r, g, b, a);

            shader.drawIndexed(v, sizeof(Vertex), 0, sizeof(Vector3), quadIndices, 6);
            shader.drawIndexed(vBold, sizeof(Vertex), 0, sizeof(Vector3), quadIndices, 6);

            currentX += glyph.xAdvance * scale;
        }
    }

    shader.setUseTexture(false);
}
