#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>
#include <chrono>

#include "Shader.h"
#include "CubeGrid.h"
#include "ParticleSystem.h"
#include "GameUI.h"
#include "AudioEngine.h"
#include "TextureAsset.h"

struct android_app;

class Renderer {
public:
    inline Renderer(android_app *pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true),
            multiTouchGesture_(false),
            lastTime_(std::chrono::high_resolution_clock::now()) {
        initRenderer();
    }

    virtual ~Renderer();

    void handleInput();
    void render();

    GameUI& getUI() { return gameUI_; }
    CubeGrid& getCubeGrid() { return cubeGrid_; }
    android_app* getApp() const { return app_; }

private:
    void initRenderer();
    void updateRenderArea();
    void renderBackground();

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;
    bool multiTouchGesture_;

    std::unique_ptr<Shader> shader_;
    std::shared_ptr<TextureAsset> bgTexture_;
    CubeGrid cubeGrid_;
    ParticleSystem particleSystem_;
    GameUI gameUI_;
    AudioEngine audioEngine_;

    std::chrono::high_resolution_clock::time_point lastTime_;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H
