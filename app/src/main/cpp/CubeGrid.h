#ifndef ANDROIDGLINVESTIGATIONS_CUBEGRID_H
#define ANDROIDGLINVESTIGATIONS_CUBEGRID_H

#include <vector>
#include <cmath>
#include <algorithm>
#include "MatrixMath.h"
#include "Shader.h"
#include "Model.h"

enum CubeState {
    CUBE_STATE_WHITE = 0,
    CUBE_STATE_BLUE = 1,
    CUBE_STATE_RED = 2
};

struct CubeData {
    int row;
    int col;
    Vec3 basePos;
    CubeState state;
    bool isObstacle;
    float jumpTime; // 0.0 to 1.0
    float yOffset;
    float rotAngle;

    CubeData()
        : row(0), col(0), basePos(0, 0, 0), state(CUBE_STATE_WHITE), isObstacle(false),
          jumpTime(0.0f), yOffset(0.0f), rotAngle(0.0f) {}
};

class CubeGrid {
public:
    static constexpr int ROWS = 10;
    static constexpr int COLS = 12;
    static constexpr float CUBE_SIZE = 0.72f;
    static constexpr float GRID_SPACING = 0.90f;

    CubeGrid() : boardRotationY_(0.0f), blueCount_(0), redCount_(0) {
        initGeometry();
        initPlatformGeometry();
        initGrid();
    }

    void reset() {
        blueCount_ = 0;
        redCount_ = 0;
        boardRotationY_ = 0.0f;
        for (auto& cube : cubes_) {
            cube.state = CUBE_STATE_WHITE;
            cube.jumpTime = 0.0f;
            cube.yOffset = 0.0f;
            cube.rotAngle = 0.0f;
        }
    }

    void recalculateCounts() {
        blueCount_ = 0;
        redCount_ = 0;
        for (const auto& cube : cubes_) {
            if (cube.state == CUBE_STATE_BLUE) blueCount_++;
            else if (cube.state == CUBE_STATE_RED) redCount_++;
        }
    }

    void setCubeState(int col, int row, CubeState newState) {
        if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return;
        int index = row * COLS + col;
        if (index >= 0 && index < static_cast<int>(cubes_.size())) {
            auto& cube = cubes_[index];
            if (cube.state != newState) {
                cube.state = newState;
                cube.jumpTime = 0.001f;
                recalculateCounts();
            }
        }
    }

    void setBoardRotationY(float angle) { boardRotationY_ = angle; }
    float getBoardRotationY() const { return boardRotationY_; }

    void update(float deltaTime) {
        for (auto& cube : cubes_) {
            if (cube.jumpTime > 0.0f) {
                cube.jumpTime += deltaTime * 3.0f;
                if (cube.jumpTime >= 1.0f) {
                    cube.jumpTime = 0.0f;
                    cube.yOffset = 0.0f;
                    cube.rotAngle = 0.0f;
                } else {
                    cube.yOffset = std::sin(cube.jumpTime * PI) * 0.75f;
                    cube.rotAngle = cube.jumpTime * 360.0f;
                }
            }
        }
    }

    int pickCube(const Vec3& rayOrigin, const Vec3& rayDir) const {
        float negRad = degToRad(-boardRotationY_);
        float cosA = std::cos(negRad);
        float sinA = std::sin(negRad);

        Vec3 localRayOrigin(
            rayOrigin.x * cosA + rayOrigin.z * sinA,
            rayOrigin.y,
            -rayOrigin.x * sinA + rayOrigin.z * cosA
        );

        Vec3 localRayDir(
            rayDir.x * cosA + rayDir.z * sinA,
            rayDir.y,
            -rayDir.x * sinA + rayDir.z * cosA
        );

        int bestIndex = -1;
        float minT = 1e9f;
        float hSize = CUBE_SIZE * 0.5f;

        for (int i = 0; i < static_cast<int>(cubes_.size()); ++i) {
            const auto& cube = cubes_[i];
            if (cube.isObstacle) continue;

            Vec3 pos = cube.basePos;
            pos.y += cube.yOffset;

            Vec3 minB(pos.x - hSize, pos.y - hSize, pos.z - hSize);
            Vec3 maxB(pos.x + hSize, pos.y + hSize, pos.z + hSize);

            float t = 0.0f;
            if (MatrixMath::rayAABBIntersection(localRayOrigin, localRayDir, minB, maxB, t)) {
                if (t < minT) {
                    minT = t;
                    bestIndex = i;
                }
            }
        }
        return bestIndex;
    }

    bool tapCube(int index, Vec3& outPos, CubeState& outState, bool isPlayerOne = true) {
        if (index < 0 || index >= static_cast<int>(cubes_.size())) return false;

        auto& cube = cubes_[index];
        if (cube.isObstacle) return false;

        CubeState targetState = isPlayerOne ? CUBE_STATE_BLUE : CUBE_STATE_RED;
        cube.state = targetState;
        recalculateCounts();

        cube.jumpTime = 0.001f;

        // Return local unrotated board position so particles rotate along with the board
        outPos = cube.basePos;
        outPos.y += cube.yOffset;

        outState = cube.state;
        return true;
    }

    void render(const Shader& shader, const float* viewProjMatrix) const {
        shader.setUseTexture(false);

        // 1. Render Base Platform Floor
        renderPlatform(shader, viewProjMatrix);

        // 2. Render Cubes
        for (const auto& cube : cubes_) {
            float model[16];
            MatrixMath::identity(model);

            if (boardRotationY_ > 0.0f) {
                MatrixMath::rotateY(model, degToRad(boardRotationY_));
            }

            Vec3 currentPos = cube.basePos;
            currentPos.y += cube.yOffset;

            MatrixMath::translate(model, currentPos.x, currentPos.y, currentPos.z);
            if (cube.rotAngle > 0.0f) {
                MatrixMath::rotateY(model, degToRad(cube.rotAngle));
            }

            float mvp[16];
            MatrixMath::multiply(mvp, viewProjMatrix, model);
            shader.setProjectionMatrix(mvp);

            float r = 0.95f, g = 0.96f, b = 0.98f;
            if (cube.isObstacle) {
                r = 0.22f; g = 0.24f; b = 0.28f;
            } else if (cube.state == CUBE_STATE_BLUE) {
                r = 0.05f; g = 0.55f; b = 1.0f;
            } else if (cube.state == CUBE_STATE_RED) {
                r = 1.0f; g = 0.22f; b = 0.22f;
            }

            for (size_t f = 0; f < 6; ++f) {
                float shade = faceShades_[f];
                shader.setColor(r * shade, g * shade, b * shade, 1.0f);
                shader.drawIndexed(
                    vertices_.data() + f * 4,
                    sizeof(Vertex),
                    0,
                    sizeof(Vector3),
                    indices_.data() + f * 6,
                    6
                );
            }
        }
    }

    int getBlueCount() const { return blueCount_; }
    int getRedCount() const { return redCount_; }

private:
    void renderPlatform(const Shader& shader, const float* viewProjMatrix) const {
        float model[16];
        MatrixMath::identity(model);

        if (boardRotationY_ > 0.0f) {
            MatrixMath::rotateY(model, degToRad(boardRotationY_));
        }

        MatrixMath::translate(model, 0.0f, -0.55f, 0.0f);

        float mvp[16];
        MatrixMath::multiply(mvp, viewProjMatrix, model);
        shader.setProjectionMatrix(mvp);

        float r = 0.35f, g = 0.40f, b = 0.46f;
        for (size_t f = 0; f < platformFaceCount_; ++f) {
            float shade = platformFaceShades_[f];
            shader.setColor(r * shade, g * shade, b * shade, 1.0f);
            shader.drawIndexed(
                platformVertices_.data() + f * 4,
                sizeof(Vertex),
                0,
                sizeof(Vector3),
                platformIndices_.data() + f * 6,
                6
            );
        }
    }

    void initGrid() {
        cubes_.clear();
        float startX = -((COLS - 1) * GRID_SPACING) * 0.5f;
        float startZ = -((ROWS - 1) * GRID_SPACING) * 0.5f;

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                CubeData cube;
                cube.row = r;
                cube.col = c;
                cube.basePos = Vec3(startX + c * GRID_SPACING, 0.0f, startZ + r * GRID_SPACING);
                cube.state = CUBE_STATE_WHITE;
                cube.isObstacle = (c == 3 && r == 3) || (c == 8 && r == 3) ||
                                  (c == 3 && r == 6) || (c == 8 && r == 6) ||
                                  (c == 5 && r == 4) || (c == 6 && r == 5);
                cubes_.push_back(cube);
            }
        }
    }

    void initGeometry() {
        float h = CUBE_SIZE * 0.5f;
        vertices_.clear();
        indices_.clear();

        // Top (+Y)
        addCubeFace(Vec3(-h, h,  h), Vec3( h, h,  h), Vec3( h, h, -h), Vec3(-h, h, -h), 1.0f);
        // Bottom (-Y)
        addCubeFace(Vec3(-h, -h, -h), Vec3( h, -h, -h), Vec3( h, -h,  h), Vec3(-h, -h,  h), 0.5f);
        // Front (+Z)
        addCubeFace(Vec3(-h, -h,  h), Vec3( h, -h,  h), Vec3( h,  h,  h), Vec3(-h,  h,  h), 0.9f);
        // Back (-Z)
        addCubeFace(Vec3( h, -h, -h), Vec3(-h, -h, -h), Vec3(-h,  h, -h), Vec3( h,  h, -h), 0.7f);
        // Right (+X)
        addCubeFace(Vec3( h, -h,  h), Vec3( h, -h, -h), Vec3( h,  h, -h), Vec3( h,  h,  h), 0.85f);
        // Left (-X)
        addCubeFace(Vec3(-h, -h, -h), Vec3(-h, -h,  h), Vec3(-h,  h,  h), Vec3(-h,  h, -h), 0.75f);
    }

    void addCubeFace(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, float shade) {
        vertices_.emplace_back(Vector3{p0.x, p0.y, p0.z}, Vector2{0, 0});
        vertices_.emplace_back(Vector3{p1.x, p1.y, p1.z}, Vector2{1, 0});
        vertices_.emplace_back(Vector3{p2.x, p2.y, p2.z}, Vector2{1, 1});
        vertices_.emplace_back(Vector3{p3.x, p3.y, p3.z}, Vector2{0, 1});

        indices_.push_back(0); indices_.push_back(1); indices_.push_back(2);
        indices_.push_back(0); indices_.push_back(2); indices_.push_back(3);

        faceShades_.push_back(shade);
    }

    void initPlatformGeometry() {
        float halfW = ((COLS * GRID_SPACING) * 0.5f) + 0.5f;
        float halfD = ((ROWS * GRID_SPACING) * 0.5f) + 0.5f;
        float topY = 0.0f;
        float botY = -0.35f;

        platformVertices_.clear();
        platformIndices_.clear();
        platformFaceShades_.clear();

        // Top Face
        addPlatformFace(Vec3(-halfW, topY,  halfD), Vec3( halfW, topY,  halfD),
                        Vec3( halfW, topY, -halfD), Vec3(-halfW, topY, -halfD), 0.95f);
        // Front Face
        addPlatformFace(Vec3(-halfW, botY,  halfD), Vec3( halfW, botY,  halfD),
                        Vec3( halfW, topY,  halfD), Vec3(-halfW, topY,  halfD), 0.80f);
        // Right Face
        addPlatformFace(Vec3( halfW, botY,  halfD), Vec3( halfW, botY, -halfD),
                        Vec3( halfW, topY, -halfD), Vec3( halfW, topY,  halfD), 0.75f);
        // Back Face
        addPlatformFace(Vec3( halfW, botY, -halfD), Vec3(-halfW, botY, -halfD),
                        Vec3(-halfW, topY, -halfD), Vec3( halfW, topY, -halfD), 0.65f);
        // Left Face
        addPlatformFace(Vec3(-halfW, botY, -halfD), Vec3(-halfW, botY,  halfD),
                        Vec3(-halfW, topY,  halfD), Vec3(-halfW, topY, -halfD), 0.70f);

        platformFaceCount_ = 5;
    }

    void addPlatformFace(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, float shade) {
        platformVertices_.emplace_back(Vector3{p0.x, p0.y, p0.z}, Vector2{0, 0});
        platformVertices_.emplace_back(Vector3{p1.x, p1.y, p1.z}, Vector2{1, 0});
        platformVertices_.emplace_back(Vector3{p2.x, p2.y, p2.z}, Vector2{1, 1});
        platformVertices_.emplace_back(Vector3{p3.x, p3.y, p3.z}, Vector2{0, 1});

        platformIndices_.push_back(0); platformIndices_.push_back(1); platformIndices_.push_back(2);
        platformIndices_.push_back(0); platformIndices_.push_back(2); platformIndices_.push_back(3);

        platformFaceShades_.push_back(shade);
    }

    float boardRotationY_;
    std::vector<CubeData> cubes_;
    std::vector<Vertex> vertices_;
    std::vector<uint16_t> indices_;
    std::vector<float> faceShades_;

    std::vector<Vertex> platformVertices_;
    std::vector<uint16_t> platformIndices_;
    std::vector<float> platformFaceShades_;
    size_t platformFaceCount_;

    int blueCount_;
    int redCount_;
};

#endif // ANDROIDGLINVESTIGATIONS_CUBEGRID_H