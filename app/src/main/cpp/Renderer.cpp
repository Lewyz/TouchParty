#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>

#include "AndroidOut.h"
#include "Shader.h"
#include "Utility.h"
#include "MatrixMath.h"

static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;

void main() {
    fragUV = inUV;
    gl_Position = uProjection * vec4(inPosition, 1.0);
}
)vertex";

static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform bool uUseTexture;

out vec4 outColor;

void main() {
    if (uUseTexture) {
        outColor = texture(uTexture, fragUV) * uColor;
    } else {
        outColor = uColor;
    }
}
)fragment";

Renderer::~Renderer() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

static void computeCameraMatrices(float width, float height, float* proj, float* view, float* viewProj) {
    float aspect = width / height;
    float fov = 44.0f;
    if (aspect < 1.6f) {
        fov = 44.0f * (1.6f / aspect);
    }

    MatrixMath::perspective(proj, degToRad(fov), aspect, 0.1f, 100.0f);
    MatrixMath::lookAt(view, Vec3(0.0f, 11.5f, 10.5f), Vec3(0.0f, -0.4f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    MatrixMath::multiply(viewProj, proj, view);
}

void Renderer::render() {
    updateRenderArea();

    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime_).count();
    lastTime_ = now;
    if (dt > 0.1f) dt = 0.1f;

    gameUI_.update(dt);
    cubeGrid_.update(dt);
    particleSystem_.update(dt);

    float proj[16], view[16], viewProj[16];
    computeCameraMatrices(float(width_), float(height_), proj, view, viewProj);

    // Dark tech background
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Render 3D Platform & Cubes with Depth Test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (shader_) {
        shader_->activate();

        // Draw 3D platform & cubes
        cubeGrid_.render(*shader_, viewProj);

        // Draw 3D particle waves (with alpha blending)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        particleSystem_.render(*shader_, viewProj);

        // 2. Render 2D UI Overlay
        glDisable(GL_DEPTH_TEST);
        gameUI_.render(*shader_, float(width_), float(height_), cubeGrid_.getRedCount(), cubeGrid_.getBlueCount());

        shader_->deactivate();
    }

    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
}

void Renderer::initRenderer() {
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    width_ = -1;
    height_ = -1;

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection", "uColor", "uUseTexture"));
    assert(shader_);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
        shaderNeedsNewProjectionMatrix_ = true;
    }
}

void Renderer::handleInput() {
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) return;

    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            // 1. Try handling touch on 2D Game UI (Play button)
            if (gameUI_.handleTouch(x, y, float(width_), float(height_))) {
                // UI consumed touch
            } else if (gameUI_.getState() == GameState::PLAYING) {
                // 2. Unproject screen touch to 3D ray for Cube Picking
                float proj[16], view[16], viewProj[16], invViewProj[16];
                computeCameraMatrices(float(width_), float(height_), proj, view, viewProj);

                if (MatrixMath::invert(invViewProj, viewProj)) {
                    Vec3 rayOrigin, rayDir;
                    MatrixMath::unproject(x, y, float(width_), float(height_), invViewProj, rayOrigin, rayDir);

                    int pickedCube = cubeGrid_.pickCube(rayOrigin, rayDir);
                    if (pickedCube != -1) {
                        Vec3 cubePos;
                        CubeState newState;
                        if (cubeGrid_.tapCube(pickedCube, cubePos, newState)) {
                            if (newState == CUBE_STATE_BLUE) {
                                particleSystem_.spawnWave(cubePos, 0.05f, 0.55f, 1.0f);
                            } else if (newState == CUBE_STATE_RED) {
                                particleSystem_.spawnWave(cubePos, 1.0f, 0.22f, 0.22f);
                            } else {
                                particleSystem_.spawnWave(cubePos, 0.9f, 0.9f, 0.9f);
                            }
                        }
                    }
                }
            }
        }
    }

    android_app_clear_motion_events(inputBuffer);
    android_app_clear_key_events(inputBuffer);
}