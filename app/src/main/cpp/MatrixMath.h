#ifndef ANDROIDGLINVESTIGATIONS_MATRIXMATH_H
#define ANDROIDGLINVESTIGATIONS_MATRIXMATH_H

#include <cmath>
#include <cstring>
#include <algorithm>

constexpr float PI = 3.14159265358979323846f;

inline float degToRad(float deg) {
    return deg * (PI / 180.0f);
}

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float len = length();
        if (len > 0.00001f) return Vec3(x / len, y / len, z / len);
        return Vec3(0, 0, 0);
    }
};

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

class MatrixMath {
public:
    static inline void identity(float* m) {
        std::memset(m, 0, sizeof(float) * 16);
        m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
    }

    static inline void multiply(float* out, const float* a, const float* b) {
        float res[16];
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                res[col * 4 + row] =
                    a[0 * 4 + row] * b[col * 4 + 0] +
                    a[1 * 4 + row] * b[col * 4 + 1] +
                    a[2 * 4 + row] * b[col * 4 + 2] +
                    a[3 * 4 + row] * b[col * 4 + 3];
            }
        }
        std::memcpy(out, res, sizeof(float) * 16);
    }

    static inline void translate(float* m, float tx, float ty, float tz) {
        float t[16];
        identity(t);
        t[12] = tx; t[13] = ty; t[14] = tz;
        multiply(m, m, t);
    }

    static inline void scale(float* m, float sx, float sy, float sz) {
        float s[16];
        identity(s);
        s[0] = sx; s[5] = sy; s[10] = sz;
        multiply(m, m, s);
    }

    static inline void rotateY(float* m, float angleRad) {
        float r[16];
        identity(r);
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        r[0] = c;   r[2] = -s;
        r[8] = s;   r[10] = c;
        multiply(m, m, r);
    }

    static inline void rotateX(float* m, float angleRad) {
        float r[16];
        identity(r);
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        r[5] = c;   r[6] = s;
        r[9] = -s;  r[10] = c;
        multiply(m, m, r);
    }

    static inline void rotateZ(float* m, float angleRad) {
        float r[16];
        identity(r);
        float c = std::cos(angleRad);
        float s = std::sin(angleRad);
        r[0] = c;   r[1] = s;
        r[4] = -s;  r[5] = c;
        multiply(m, m, r);
    }

    static inline void perspective(float* m, float fovYRad, float aspect, float zNear, float zFar) {
        std::memset(m, 0, sizeof(float) * 16);
        float tanHalfFovy = std::tan(fovYRad / 2.0f);
        m[0] = 1.0f / (aspect * tanHalfFovy);
        m[5] = 1.0f / tanHalfFovy;
        m[10] = -(zFar + zNear) / (zFar - zNear);
        m[11] = -1.0f;
        m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
    }

    static inline void orthographic(float* m, float left, float right, float bottom, float top, float zNear, float zFar) {
        std::memset(m, 0, sizeof(float) * 16);
        m[0] = 2.0f / (right - left);
        m[5] = 2.0f / (top - bottom);
        m[10] = -2.0f / (zFar - zNear);
        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -(zFar + zNear) / (zFar - zNear);
        m[15] = 1.0f;
    }

    static inline void lookAt(float* m, Vec3 eye, Vec3 target, Vec3 up) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = cross(f, up).normalized();
        Vec3 u = cross(s, f);

        float r[16];
        identity(r);
        r[0] = s.x;  r[4] = s.y;  r[8] = s.z;
        r[1] = u.x;  r[5] = u.y;  r[9] = u.z;
        r[2] = -f.x; r[6] = -f.y; r[10] = -f.z;
        r[12] = -dot(s, eye);
        r[13] = -dot(u, eye);
        r[14] = dot(f, eye);

        std::memcpy(m, r, sizeof(float) * 16);
    }

    static inline bool invert(float* invOut, const float* m) {
        float inv[16], det;
        inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] + m[9]  * m[7]  * m[14] +m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];
        inv[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] - m[8]  * m[7]  * m[14] -m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];
        inv[8] = m[4]  * m[9]  * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5]  * m[15] + m[8]  * m[7]  * m[13] +m[12] * m[5]  * m[11] - m[12] * m[7]  * m[9];
        inv[12] = -m[4]  * m[9]  * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5]  * m[14] - m[8]  * m[6]  * m[13] -m[12] * m[5]  * m[10] + m[12] * m[6]  * m[9];
        inv[1] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2]  * m[15] - m[9]  * m[3]  * m[14] -m[13] * m[2]  * m[11] + m[13] * m[3]  * m[10];
        inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2]  * m[15] + m[8]  * m[3]  * m[14] +m[12] * m[2]  * m[11] - m[12] * m[3]  * m[10];
        inv[9] = -m[0]  * m[9]  * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1]  * m[15] - m[8]  * m[3]  * m[13] -m[12] * m[1]  * m[11] + m[12] * m[3]  * m[9];
        inv[13] = m[0]  * m[9]  * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1]  * m[14] + m[8]  * m[2]  * m[13] +m[12] * m[1]  * m[10] - m[12] * m[2]  * m[9];
        inv[2] = m[1]  * m[6]  * m[15] - m[1]  * m[7]  * m[14] - m[5]  * m[2]  * m[15] + m[5]  * m[3]  * m[14] +m[13] * m[2]  * m[7]  - m[13] * m[3]  * m[6];
        inv[6] = -m[0]  * m[6]  * m[15] + m[0]  * m[7]  * m[14] + m[4]  * m[2]  * m[15] - m[4]  * m[3]  * m[14] -m[12] * m[2]  * m[7]  + m[12] * m[3]  * m[6];
        inv[10] = m[0]  * m[5]  * m[15] - m[0]  * m[7]  * m[13] - m[4]  * m[1]  * m[15] + m[4]  * m[3]  * m[13] +m[12] * m[1]  * m[7]  - m[12] * m[3]  * m[5];
        inv[14] = -m[0]  * m[5]  * m[14] + m[0]  * m[6]  * m[13] + m[4]  * m[1]  * m[14] - m[4]  * m[2]  * m[13] -m[12] * m[1]  * m[6]  + m[12] * m[2]  * m[5];
        inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        if (det == 0.0f) return false;

        float invDet = 1.0f / det;
        for (int i = 0; i < 16; i++) {
            invOut[i] = inv[i] * invDet;
        }
        return true;
    }

    static inline void transformVec4(float* outVec, const float* m, const float* v) {
        for (int i = 0; i < 4; ++i) {
            outVec[i] = m[0 * 4 + i] * v[0] +
                        m[1 * 4 + i] * v[1] +
                        m[2 * 4 + i] * v[2] +
                        m[3 * 4 + i] * v[3];
        }
    }

    static inline void unproject(float screenX, float screenY, float screenW, float screenH,
                                const float* invViewProj, Vec3& rayOrigin, Vec3& rayDir) {
        // Convert screen coordinates to Normalized Device Coordinates (NDC)
        float ndcX = (2.0f * screenX) / screenW - 1.0f;
        float ndcY = 1.0f - (2.0f * screenY) / screenH; // Invert Y for OpenGL

        float nearPoint[4] = { ndcX, ndcY, -1.0f, 1.0f };
        float farPoint[4]  = { ndcX, ndcY,  1.0f, 1.0f };

        float nearResult[4], farResult[4];
        transformVec4(nearResult, invViewProj, nearPoint);
        transformVec4(farResult, invViewProj, farPoint);

        if (nearResult[3] != 0.0f) {
            nearResult[0] /= nearResult[3];
            nearResult[1] /= nearResult[3];
            nearResult[2] /= nearResult[3];
        }
        if (farResult[3] != 0.0f) {
            farResult[0] /= farResult[3];
            farResult[1] /= farResult[3];
            farResult[2] /= farResult[3];
        }

        rayOrigin = Vec3(nearResult[0], nearResult[1], nearResult[2]);
        Vec3 endPoint(farResult[0], farResult[1], farResult[2]);
        rayDir = (endPoint - rayOrigin).normalized();
    }

    static inline bool rayAABBIntersection(const Vec3& rayOrigin, const Vec3& rayDir,
                                           const Vec3& minBounds, const Vec3& maxBounds, float& tOut) {
        float tmin = -1e9f;
        float tmax = 1e9f;

        float dir[3] = { rayDir.x, rayDir.y, rayDir.z };
        float orig[3] = { rayOrigin.x, rayOrigin.y, rayOrigin.z };
        float bmin[3] = { minBounds.x, minBounds.y, minBounds.z };
        float bmax[3] = { maxBounds.x, maxBounds.y, maxBounds.z };

        for (int i = 0; i < 3; ++i) {
            if (std::abs(dir[i]) < 0.00001f) {
                if (orig[i] < bmin[i] || orig[i] > bmax[i]) return false;
            } else {
                float invD = 1.0f / dir[i];
                float t1 = (bmin[i] - orig[i]) * invD;
                float t2 = (bmax[i] - orig[i]) * invD;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) return false;
            }
        }
        if (tmax < 0.0f) return false;
        tOut = (tmin >= 0.0f) ? tmin : tmax;
        return true;
    }
};

#endif // ANDROIDGLINVESTIGATIONS_MATRIXMATH_H