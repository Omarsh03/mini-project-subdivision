#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <utility>
#include <vector>

#include "vec3.hpp"

namespace subdiv {

// Minimal indexed triangle mesh — just enough for the Loop-subdivision
// bonus. Connectivity (edge adjacency, vertex valence) is derived on the
// fly in loopSubdivide(); at bonus-demo mesh sizes that costs microseconds
// and keeps the structure trivial.
struct TriMesh {
    std::vector<Vec3> verts;
    std::vector<std::array<int, 3>> tris;
};

// Axis-aligned cube, each face split into two triangles. Note the corner
// vertices are all extraordinary for Loop subdivision (valence != 6) —
// per the project scope they get no special treatment beyond the standard
// valence-dependent weights, which is exactly what makes the demo honest.
inline TriMesh cube(float halfSize = 0.7f) {
    const float s = halfSize;
    TriMesh m;
    m.verts = {{-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
               {-s, -s, s},  {s, -s, s},  {s, s, s},  {-s, s, s}};
    m.tris = {{0, 2, 1}, {0, 3, 2},   // back  (z = -s)
              {4, 5, 6}, {4, 6, 7},   // front (z = +s)
              {0, 1, 5}, {0, 5, 4},   // bottom
              {3, 7, 6}, {3, 6, 2},   // top
              {0, 4, 7}, {0, 7, 3},   // left
              {1, 2, 6}, {1, 6, 5}};  // right
    return m;
}

// One round of Loop subdivision (approximating, for triangle meshes):
//
//   edge point (interior):  3/8·(A + B) + 1/8·(C + D)
//     where C, D are the apexes of the two triangles sharing edge (A, B)
//   old vertex of valence n: (1 − n·β)·V + β·Σ neighbors,
//     with Warren's weights  β = 3/16 (n = 3),  β = 3/(8n) otherwise.
//
// Every triangle splits into four. Boundary edges (only one adjacent
// triangle) fall back to the midpoint rule; the cube has none.
inline TriMesh loopSubdivide(const TriMesh& in) {
    struct EdgeInfo {
        std::array<int, 2> opposite{-1, -1}; // apex vertices of adjacent tris
        int newIndex = -1;                   // index of this edge's new point
    };
    // Note: NOT std::minmax(a, b) — that returns a pair of references to the
    // by-value parameters, which dangle as soon as the lambda returns.
    const auto keyOf = [](int a, int b) {
        return a < b ? std::pair<int, int>(a, b) : std::pair<int, int>(b, a);
    };

    std::map<std::pair<int, int>, EdgeInfo> edges;
    for (const auto& t : in.tris) {
        for (int e = 0; e < 3; ++e) {
            EdgeInfo& info = edges[keyOf(t[e], t[(e + 1) % 3])];
            info.opposite[info.opposite[0] < 0 ? 0 : 1] = t[(e + 2) % 3];
        }
    }

    std::vector<std::vector<int>> neighbors(in.verts.size());
    for (const auto& [key, info] : edges) {
        neighbors[static_cast<size_t>(key.first)].push_back(key.second);
        neighbors[static_cast<size_t>(key.second)].push_back(key.first);
    }

    TriMesh out;
    out.verts.reserve(in.verts.size() + edges.size());
    out.tris.reserve(4 * in.tris.size());

    for (size_t i = 0; i < in.verts.size(); ++i) {
        const auto& nb = neighbors[i];
        const int n = static_cast<int>(nb.size());
        const float beta =
            (n == 3) ? 3.0f / 16.0f : 3.0f / (8.0f * static_cast<float>(n));
        Vec3 sum{};
        for (int j : nb)
            sum = sum + in.verts[static_cast<size_t>(j)];
        out.verts.push_back((1.0f - static_cast<float>(n) * beta) * in.verts[i] +
                            beta * sum);
    }

    for (auto& [key, info] : edges) {
        info.newIndex = static_cast<int>(out.verts.size());
        const Vec3 a = in.verts[static_cast<size_t>(key.first)];
        const Vec3 b = in.verts[static_cast<size_t>(key.second)];
        if (info.opposite[1] >= 0) {
            out.verts.push_back(
                0.375f * (a + b) +
                0.125f * (in.verts[static_cast<size_t>(info.opposite[0])] +
                          in.verts[static_cast<size_t>(info.opposite[1])]));
        } else {
            out.verts.push_back(0.5f * (a + b)); // boundary edge
        }
    }

    for (const auto& t : in.tris) {
        const int e01 = edges[keyOf(t[0], t[1])].newIndex;
        const int e12 = edges[keyOf(t[1], t[2])].newIndex;
        const int e20 = edges[keyOf(t[2], t[0])].newIndex;
        out.tris.push_back({t[0], e01, e20});
        out.tris.push_back({t[1], e12, e01});
        out.tris.push_back({t[2], e20, e12});
        out.tris.push_back({e01, e12, e20});
    }
    return out;
}

} // namespace subdiv
