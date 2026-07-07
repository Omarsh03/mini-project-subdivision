#pragma once

#include <vector>

#include "scheme.hpp"
#include "vec2.hpp"

namespace subdiv {

// Chaikin corner cutting, generalized to independent cut depths measured
// from each end of the edge (A, B):
//
//   Q = (1-t1)*A + t1*B      (cut t1 from A)
//   R = t2*A + (1-t2)*B      (cut t2 from B)
//
// The classic scheme is the symmetric case t1 = t2 = t; the canonical ratio
// t = 1/4 yields the 1/4 : 3/4 rule, whose limit curve is the quadratic
// B-spline of the control polygon. Both parameters are left fully
// user-adjustable: t = 1/2 collapses the cuts to the edge midpoint,
// t > 1/2 makes them cross over (self-intersecting polygons), and unequal
// t1/t2 breaks the mask's symmetry while every point remains an affine —
// indeed convex — combination, so the polygon still can't explode.
class ChaikinScheme : public SubdivisionScheme {
public:
    float t1 = 0.25f;
    float t2 = 0.25f;

    explicit ChaikinScheme(float cutRatio = 0.25f) : t1(cutRatio), t2(cutRatio) {}
    ChaikinScheme(float cutFromA, float cutFromB) : t1(cutFromA), t2(cutFromB) {}

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
            out.push_back(lerp(a, b, t1));
            out.push_back(lerp(a, b, 1.0f - t2));
        }
        if (!closed)
            out.push_back(pts.back());
        return out;
    }
};

} // namespace subdiv
