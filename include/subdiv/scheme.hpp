#pragma once

#include <cmath>
#include <utility>
#include <vector>

#include "polygon.hpp"
#include "vec2.hpp"

namespace subdiv {

// A subdivision scheme is one refinement rule: given the points of a
// polygon, produce the points of the refined polygon. Iteration, level
// bookkeeping and divergence protection live in refine(), so concrete
// schemes (Chaikin, four-point) only implement the mask itself.
class SubdivisionScheme {
public:
    virtual ~SubdivisionScheme() = default;
    virtual std::vector<Vec2> step(const std::vector<Vec2>& pts, bool closed) const = 0;
};

// Every level of an iterated refinement, retained for the step-by-step
// convergence view. levels[0] is the control polygon itself.
struct RefineResult {
    std::vector<std::vector<Vec2>> levels;
    bool diverged = false;
};

// Coordinates beyond this bound count as divergence. World space is ~[-1,1]^2,
// so anything near this bound is unrecoverably off-screen anyway.
inline constexpr float kDivergenceBound = 1.0e6f;

// Total polyline length. For a convergent scheme this approaches the limit
// curve's arc length (per-level ratio -> 1); sustained growth per level is
// the practical tell that the weights left the convergent regime, long
// before coordinates blow up in absolute terms.
inline float perimeter(const std::vector<Vec2>& pts, bool closed) {
    if (pts.size() < 2)
        return 0.0f;
    float length = 0.0f;
    const size_t segments = closed ? pts.size() : pts.size() - 1;
    for (size_t i = 0; i < segments; ++i) {
        const Vec2 d = pts[(i + 1) % pts.size()] - pts[i];
        length += std::sqrt(d.x * d.x + d.y * d.y);
    }
    return length;
}

inline bool withinBounds(const std::vector<Vec2>& pts) {
    for (Vec2 v : pts) {
        if (!isFinite(v) || std::abs(v.x) > kDivergenceBound || std::abs(v.y) > kDivergenceBound)
            return false;
    }
    return true;
}

// Apply `scheme` to `poly` `iterations` times, keeping every intermediate
// level. If a level escapes kDivergenceBound (possible under user-chosen
// weights), it is discarded and refinement stops with diverged = true, so
// callers can always safely render every retained level.
inline RefineResult refine(const SubdivisionScheme& scheme, const Polygon& poly, int iterations) {
    RefineResult result;
    result.levels.push_back(poly.pts);
    for (int i = 0; i < iterations; ++i) {
        std::vector<Vec2> next = scheme.step(result.levels.back(), poly.closed);
        if (!withinBounds(next)) {
            result.diverged = true;
            break;
        }
        result.levels.push_back(std::move(next));
    }
    return result;
}

} // namespace subdiv
