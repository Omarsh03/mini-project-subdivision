#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "vec2.hpp"

namespace subdiv {

// Control polygon in world space. Presets fit roughly inside [-1, 1]^2,
// which the viewport maps to the visible canvas.
struct Polygon {
    std::vector<Vec2> pts;
    bool closed = true;
};

namespace presets {

inline constexpr float kPi = 3.14159265358979323846f;

inline Polygon square() {
    return {{{-0.7f, -0.7f}, {0.7f, -0.7f}, {0.7f, 0.7f}, {-0.7f, 0.7f}}, true};
}

inline Polygon star(int spikes = 5, float outerRadius = 0.9f, float innerRadius = 0.35f) {
    Polygon p;
    p.pts.reserve(static_cast<size_t>(2 * spikes));
    for (int i = 0; i < 2 * spikes; ++i) {
        const float angle = kPi * static_cast<float>(i) / static_cast<float>(spikes)
                          - kPi / 2.0f; // first spike points up
        const float r = (i % 2 == 0) ? outerRadius : innerRadius;
        p.pts.push_back({r * std::cos(angle), r * std::sin(angle)});
    }
    return p;
}

// Jagged saw-tooth ring: alternating teeth along the top, flat bottom.
// Deliberately full of sharp corners for corner-cutting to chew on.
inline Polygon zigzag(int teeth = 6) {
    Polygon p;
    const float left = -0.9f, right = 0.9f;
    const int topPoints = 2 * teeth + 1;
    for (int i = 0; i < topPoints; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(topPoints - 1);
        const float x = left + t * (right - left);
        const float y = (i % 2 == 0) ? 0.1f : 0.6f;
        p.pts.push_back({x, y});
    }
    p.pts.push_back({right, -0.6f});
    p.pts.push_back({left, -0.6f});
    return p;
}

// Open circular arc — the open-polyline showcase: both schemes must keep
// its endpoints fixed while smoothing the interior.
inline Polygon arc(int n = 7, float radius = 0.8f) {
    Polygon p;
    p.closed = false;
    p.pts.reserve(static_cast<size_t>(n));
    // 216-degree sweep over the top, endpoints hanging below the x-axis.
    const float start = 1.1f * kPi;
    const float end = -0.1f * kPi;
    for (int i = 0; i < n; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(n - 1);
        const float angle = start + t * (end - start);
        p.pts.push_back({radius * std::cos(angle), radius * std::sin(angle)});
    }
    return p;
}

// Star-shaped random polygon: evenly spaced angles with jittered radii, so
// the outline never self-intersects. Seeded for reproducibility.
inline Polygon random(int n = 10, std::uint32_t seed = 1) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> radius(0.3f, 0.9f);
    Polygon p;
    p.pts.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        const float angle = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(n);
        const float r = radius(rng);
        p.pts.push_back({r * std::cos(angle), r * std::sin(angle)});
    }
    return p;
}

} // namespace presets
} // namespace subdiv
