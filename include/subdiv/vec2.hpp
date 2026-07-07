#pragma once

#include <cmath>

namespace subdiv {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(float s, Vec2 v) { return {s * v.x, s * v.y}; }
inline Vec2 operator*(Vec2 v, float s) { return s * v; }

// Affine combination (1-t)*a + t*b — the primitive every subdivision mask
// is built from. t outside [0,1] extrapolates, which is exactly what the
// "wrong weights" experiments rely on.
inline Vec2 lerp(Vec2 a, Vec2 b, float t) { return (1.0f - t) * a + t * b; }

inline bool isFinite(Vec2 v) { return std::isfinite(v.x) && std::isfinite(v.y); }

} // namespace subdiv
