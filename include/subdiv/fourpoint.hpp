#pragma once

#include <vector>

#include "scheme.hpp"
#include "vec2.hpp"

namespace subdiv {

// Four-point interpolating subdivision (Dyn–Levin–Gregory). All old points
// are kept; between each consecutive pair a new point is inserted from the
// four surrounding points:
//
//   M_i = (1/2 + w) * (P_i + P_{i+1}) - w * (P_{i-1} + P_{i+2})
//
// Because old points never move, the limit curve interpolates the control
// polygon — the defining contrast with approximating schemes like Chaikin.
// The canonical tension w = 1/16 reproduces cubic polynomials and gives a
// C^1 limit curve; the scheme stays C^1 only for 0 < w < (sqrt(5)-1)/8
// ~= 0.19. w = 0 degenerates to midpoint insertion (the polygon itself is
// the limit), and larger w turns the curve fractal, then divergent.
class FourPointScheme : public SubdivisionScheme {
public:
    float w = 0.0625f; // 1/16

    explicit FourPointScheme(float tension = 0.0625f) : w(tension) {}

    std::vector<Vec2> step(const std::vector<Vec2>& pts, bool closed) const override {
        const size_t n = pts.size();
        if (n < 3)
            return pts;

        // Open polylines lack neighbors at the boundary; clamp indices so
        // the missing neighbor duplicates the endpoint.
        const auto at = [&](long i) -> Vec2 {
            if (closed)
                return pts[static_cast<size_t>(((i % static_cast<long>(n)) + static_cast<long>(n)) % static_cast<long>(n))];
            if (i < 0)
                return pts.front();
            if (i >= static_cast<long>(n))
                return pts.back();
            return pts[static_cast<size_t>(i)];
        };

        std::vector<Vec2> out;
        out.reserve(2 * n);
        const size_t edges = closed ? n : n - 1;
        for (size_t i = 0; i < edges; ++i) {
            const long li = static_cast<long>(i);
            out.push_back(pts[i]);
            const Vec2 mid = (0.5f + w) * (at(li) + at(li + 1)) - w * (at(li - 1) + at(li + 2));
            out.push_back(mid);
        }
        if (!closed)
            out.push_back(pts.back());
        return out;
    }
};

} // namespace subdiv
