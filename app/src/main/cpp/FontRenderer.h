#ifndef TOUCHPARTY_FONT_RENDERER_H
#define TOUCHPARTY_FONT_RENDERER_H

#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "Shader.h"
#include "Model.h"

class FontRenderer {
public:
    FontRenderer();
    ~FontRenderer();

    // Load font file from Android AssetManager
    bool loadFont(AAssetManager* assetManager, const std::string& assetPath, float fontPixelHeight = 64.0f);

    // Measure string width in orthographic coordinates
    float getTextWidth(const std::string& text, float fontSize) const;

    // Draw text string with OpenGL shader
    void drawText(const Shader& shader, const float* ortho, float cx, float cy,
                  const std::string& text, float fontSize,
                  float r, float g, float b, float a = 1.0f, bool centered = true) const;

    bool isLoaded() const { return isLoaded_; }

private:
    struct GlyphInfo {
        float x0, y0, x1, y1; // Quad bounds in pixels relative to baseline
        float s0, t0, s1, t1; // UV bounds in texture atlas
        float xAdvance;        // Horizontal advance in pixels
    };

    GLuint textureID_;
    int atlasWidth_;
    int atlasHeight_;
    float fontPixelHeight_;
    bool isLoaded_;

    std::unordered_map<uint32_t, GlyphInfo> glyphMap_;

    const GlyphInfo* getGlyph(uint32_t cp) const;
    static uint32_t getFallbackCodepoint(uint32_t cp);
    static std::vector<uint32_t> decodeUTF8(const std::string& text);
};

#endif // TOUCHPARTY_FONT_RENDERER_H
