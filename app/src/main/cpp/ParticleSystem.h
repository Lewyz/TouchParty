#ifndef ANDROIDGLINVESTIGATIONS_PARTICLESYSTEM_H
#define ANDROIDGLINVESTIGATIONS_PARTICLESYSTEM_H

#include <vector>
#include <cmath>
#include <algorithm>
#include "MatrixMath.h"
#include "Shader.h"
#include "Model.h"

struct Particle {
    Vec3 pos;
    float r, g, b;
    float currentRadius;
    float maxRadius;
    float age;       // in seconds
    float maxAge;    // total lifetime
    bool active;
};

class ParticleSystem {
public:
    ParticleSystem() {
        initRingGeometry();
    }

    void clear() {
        particles_.clear();
    }

    void spawnWave(const Vec3& center, float r, float g, float b) {
        for (int i = 0; i < 2; ++i) {
            Particle p;
            p.pos = center;
            p.pos.y += 0.05f + static_cast<float>(i) * 0.02f;
            p.r = r;
            p.g = g;
            p.b = b;
            p.currentRadius = 0.3f + static_cast<float>(i) * 0.1f;
            p.maxRadius = 1.6f + static_cast<float>(i) * 0.4f;
            p.age = 0.0f;
            p.maxAge = 0.45f + static_cast<float>(i) * 0.1f;
            p.active = true;
            particles_.push_back(p);
        }
    }

    void update(float deltaTime) {
        for (auto& p : particles_) {
            if (!p.active) continue;
            p.age += deltaTime;
            if (p.age >= p.maxAge) {
                p.active = false;
            } else {
                float t = p.age / p.maxAge;
                p.currentRadius = 0.3f + t * (p.maxRadius - 0.3f);
            }
        }

        particles_.erase(
            std::remove_if(particles_.begin(), particles_.end(),
                           [](const Particle& p) { return !p.active; }),
            particles_.end());
    }

    void render(const Shader& shader, const float* viewProjMatrix, float boardRotationY = 0.0f) const {
        if (particles_.empty()) return;

        shader.setUseTexture(false);

        for (const auto& p : particles_) {
            if (!p.active) continue;

            float alpha = 1.0f - (p.age / p.maxAge);
            if (alpha < 0.0f) alpha = 0.0f;

            float model[16];
            MatrixMath::identity(model);

            if (boardRotationY > 0.0f) {
                MatrixMath::rotateY(model, degToRad(boardRotationY));
            }

            MatrixMath::translate(model, p.pos.x, p.pos.y, p.pos.z);
            MatrixMath::scale(model, p.currentRadius, 1.0f, p.currentRadius);

            float mvp[16];
            MatrixMath::multiply(mvp, viewProjMatrix, model);
            shader.setProjectionMatrix(mvp);
            shader.setColor(p.r, p.g, p.b, alpha * 0.85f);

            shader.drawIndexed(
                vertices_.data(),
                sizeof(Vertex),
                0,
                sizeof(Vector3),
                indices_.data(),
                indices_.size()
            );
        }
    }

private:
    void initRingGeometry() {
        vertices_.clear();
        indices_.clear();

        constexpr int SEGMENTS = 24;
        float innerR = 0.7f;
        float outerR = 1.0f;

        for (int i = 0; i < SEGMENTS; ++i) {
            float angle = (i * 2.0f * PI) / SEGMENTS;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            vertices_.emplace_back(Vector3{innerR * cosA, 0.0f, innerR * sinA}, Vector2{0, 0});
            vertices_.emplace_back(Vector3{outerR * cosA, 0.0f, outerR * sinA}, Vector2{1, 0});

            int next = (i + 1) % SEGMENTS;
            uint16_t i0 = static_cast<uint16_t>(i * 2);
            uint16_t i1 = static_cast<uint16_t>(i * 2 + 1);
            uint16_t i2 = static_cast<uint16_t>(next * 2);
            uint16_t i3 = static_cast<uint16_t>(next * 2 + 1);

            indices_.push_back(i0); indices_.push_back(i1); indices_.push_back(i2);
            indices_.push_back(i1); indices_.push_back(i3); indices_.push_back(i2);
        }
    }

    std::vector<Particle> particles_;
    std::vector<Vertex> vertices_;
    std::vector<uint16_t> indices_;
};

#endif // ANDROIDGLINVESTIGATIONS_PARTICLESYSTEM_H