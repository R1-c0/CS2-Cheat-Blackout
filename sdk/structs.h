#pragma once

#include <cmath>

struct Vector2 {
    float x, y;

    Vector2() : x(0.f), y(0.f) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct Vector3 {
    float x, y, z;

    Vector3() : x(0.f), y(0.f), z(0.f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

    bool IsValid() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + y * y); }

    Vector3 Normalized() const {
        float len = Length();
        if (len < 0.001f) return Vector3();
        return *this / len;
    }
};

struct view_matrix_t {
    float m[4][4];

    float* operator[](int col) { return m[col]; }
    const float* operator[](int col) const { return m[col]; }
};

struct bone_matrix_t {
    float data[3][4];
};
