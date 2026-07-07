#pragma once

#include <vector>

#include "scheme.hpp"
#include "vec2.hpp"

namespace subdiv {

// Chaikin corner cutting. Each edge (A, B) is replaced by two points that
// "cut" the corners at its ends:
//
//   Q = (1-t)*A + t*B      (near A)
//   R = t*A + (1-t)*B      (near B)
//
// The canonical ratio t = 1/4 yields the classic 1/4 : 3/4 rule, whose limit
// curve is the quadratic B-spline of the control polygon. t is left fully
// user-adjustable: t = 1/2 collapses both cuts to the edge midpoint, and
// t > 1/2 makes the cuts cross over, producing self-intersecting polygons —
// the first "wrong weights" experiment.
class ChaikinScheme : public SubdivisionScheme {
public:
    float t = 0.25f;

    explicit ChaikinScheme(float cutRatio = 0.25f) : t(cutRatio) {}

    std::vector<Vec2> step(const std::vector<Vec2>& pts, bool closed) const override {
        const size_t n = pts.size();
        if (n < 3)
            return pts;

        std::vector<Vec2> out;
        out.reserve(2 * n + 2);
        // Open polylines keep their endpoints and only cut interior corners.
        if (!closed)
            out.push_back(pts.front());
        const size_t edges = closed ? n : n - 1;
        for (size_t i = 0; i < edges; ++i) {
            const Vec2 a = pts[i];
            const Vec2 b = pts[(i + 1) % n];
            out.push_back(lerp(a, b, t));
            out.push_back(lerp(a, b, 1.0f - t));
        }
        if (!closed)
            out.push_back(pts.back());
        return out;
    }
};

} // namespace subdiv
