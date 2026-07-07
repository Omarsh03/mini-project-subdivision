// Assert-based unit tests for the subdivision math core (include/subdiv/).
// Runs via CTest; exits non-zero on any failure.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <utility>
#include <vector>

#include "subdiv/chaikin.hpp"
#include "subdiv/fourpoint.hpp"
#include "subdiv/polygon.hpp"
#include "subdiv/scheme.hpp"
#include "subdiv/trimesh.hpp"
#include "subdiv/vec2.hpp"

using namespace subdiv;

static int g_failures = 0;

#define CHECK(cond)                                                         \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);     \
            ++g_failures;                                                   \
        }                                                                   \
    } while (0)

static bool near(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) < eps; }
static bool near(Vec2 a, Vec2 b, float eps = 1e-5f) { return near(a.x, b.x, eps) && near(a.y, b.y, eps); }

// --- Test doubles ------------------------------------------------------------

// Replaces each point set with the edge midpoints (closed polygons keep
// their point count) — a minimal well-behaved scheme for exercising refine().
struct MidpointScheme : SubdivisionScheme {
    std::vector<Vec2> step(const std::vector<Vec2>& pts, bool /*closed*/) const override {
        std::vector<Vec2> out;
        out.reserve(pts.size());
        for (size_t i = 0; i < pts.size(); ++i)
            out.push_back(lerp(pts[i], pts[(i + 1) % pts.size()], 0.5f));
        return out;
    }
};

// Scales every coordinate by a large factor each step — guaranteed to
// escape kDivergenceBound, for exercising the divergence guard.
struct ExplodingScheme : SubdivisionScheme {
    std::vector<Vec2> step(const std::vector<Vec2>& pts, bool /*closed*/) const override {
        std::vector<Vec2> out;
        out.reserve(pts.size());
        for (Vec2 v : pts)
            out.push_back(1000.0f * v);
        return out;
    }
};

// --- Tests -------------------------------------------------------------------

static void testVec2() {
    CHECK(near(Vec2{1, 2} + Vec2{3, 4}, Vec2{4, 6}));
    CHECK(near(Vec2{5, 7} - Vec2{2, 3}, Vec2{3, 4}));
    CHECK(near(2.0f * Vec2{1, -2}, Vec2{2, -4}));
    CHECK(near(Vec2{1, -2} * 2.0f, Vec2{2, -4}));

    const Vec2 a{0, 0}, b{4, 8};
    CHECK(near(lerp(a, b, 0.0f), a));
    CHECK(near(lerp(a, b, 1.0f), b));
    CHECK(near(lerp(a, b, 0.5f), Vec2{2, 4}));
    CHECK(near(lerp(a, b, 0.25f), Vec2{1, 2}));
    CHECK(near(lerp(a, b, 1.5f), Vec2{6, 12})); // extrapolation must work too

    CHECK(isFinite(Vec2{1, 1}));
    CHECK(!isFinite(Vec2{std::nanf(""), 0}));
    CHECK(!isFinite(Vec2{0, INFINITY}));
}

static void testPresets() {
    const Polygon sq = presets::square();
    CHECK(sq.pts.size() == 4);
    CHECK(sq.closed);

    const Polygon st = presets::star(5);
    CHECK(st.pts.size() == 10);
    CHECK(near(st.pts[0], Vec2{0.0f, -0.9f})); // first spike points up (y grows downward on screen later)

    const Polygon zz = presets::zigzag(6);
    CHECK(zz.pts.size() == 2 * 6 + 1 + 2);
    CHECK(zz.closed);

    // Arc preset: open, endpoints below the x-axis, all points on the circle.
    const Polygon a = presets::arc(7, 0.8f);
    CHECK(a.pts.size() == 7);
    CHECK(!a.closed);
    CHECK(a.pts.front().y < 0.0f && a.pts.back().y < 0.0f);
    for (Vec2 v : a.pts)
        CHECK(near(std::sqrt(v.x * v.x + v.y * v.y), 0.8f));

    // Random preset is deterministic per seed.
    const Polygon r1 = presets::random(10, 42);
    const Polygon r2 = presets::random(10, 42);
    const Polygon r3 = presets::random(10, 43);
    CHECK(r1.pts.size() == 10);
    bool sameSeedEqual = true;
    for (size_t i = 0; i < r1.pts.size(); ++i)
        sameSeedEqual = sameSeedEqual && near(r1.pts[i], r2.pts[i]);
    CHECK(sameSeedEqual);
    bool differentSeedDiffers = false;
    for (size_t i = 0; i < r1.pts.size(); ++i)
        differentSeedDiffers = differentSeedDiffers || !near(r1.pts[i], r3.pts[i]);
    CHECK(differentSeedDiffers);
}

static void testRefine() {
    const Polygon sq = presets::square();
    const MidpointScheme midpoint;

    const RefineResult r0 = refine(midpoint, sq, 0);
    CHECK(r0.levels.size() == 1);
    CHECK(!r0.diverged);

    const RefineResult r3 = refine(midpoint, sq, 3);
    CHECK(r3.levels.size() == 4); // control polygon + 3 refinements
    CHECK(!r3.diverged);
    bool level0IsInput = true;
    for (size_t i = 0; i < sq.pts.size(); ++i)
        level0IsInput = level0IsInput && near(r3.levels[0][i], sq.pts[i]);
    CHECK(level0IsInput);
    // Midpoints of a centered square stay centered and shrink toward it.
    CHECK(near(r3.levels[1][0], Vec2{0.0f, -0.7f}));
}

static void testPerimeter() {
    const Polygon sq = presets::square(); // side 1.4 -> perimeter 5.6
    CHECK(near(perimeter(sq.pts, true), 5.6f));
    CHECK(near(perimeter(sq.pts, false), 4.2f)); // open: closing edge dropped

    // Chaikin at the canonical ratio shortens the polygon toward the limit
    // curve's arc length; the per-level ratio must settle near 1.
    const ChaikinScheme chaikin(0.25f);
    const RefineResult r = refine(chaikin, sq, 6);
    const float p5 = perimeter(r.levels[5], true);
    const float p6 = perimeter(r.levels[6], true);
    CHECK(p6 < 5.6f);
    CHECK(p6 / p5 > 0.99f && p6 / p5 < 1.01f);

    // Max edge length: 1.4 on the square, roughly halving per convergent
    // level (the sup-norm indicator behind the convergence plot).
    CHECK(near(maxEdgeLength(sq.pts, true), 1.4f));
    CHECK(near(maxEdgeLength(sq.pts, false), 1.4f));
    CHECK(maxEdgeLength(r.levels[6], true) < 0.05f);
}

static void testDivergenceGuard() {
    const Polygon sq = presets::square();
    const ExplodingScheme exploding;

    // 1000^3 * 0.7 > 1e6 — must trip the guard before all 8 levels exist.
    const RefineResult r = refine(exploding, sq, 8);
    CHECK(r.diverged);
    CHECK(r.levels.size() < 9);
    for (const auto& level : r.levels)
        CHECK(withinBounds(level)); // every retained level is safe to render
}

static void testChaikin() {
    const Polygon sq = presets::square(); // (-.7,-.7) (.7,-.7) (.7,.7) (-.7,.7)
    const ChaikinScheme chaikin(0.25f);

    // Hand-computed step at the canonical ratio: the first edge
    // (-.7,-.7)->(.7,-.7) must yield cuts at x = -0.35 and x = 0.35.
    const std::vector<Vec2> once = chaikin.step(sq.pts, sq.closed);
    CHECK(once.size() == 8); // point count doubles for closed polygons
    CHECK(near(once[0], Vec2{-0.35f, -0.7f}));
    CHECK(near(once[1], Vec2{0.35f, -0.7f}));
    CHECK(near(once[2], Vec2{0.7f, -0.35f}));
    CHECK(near(once[3], Vec2{0.7f, 0.35f}));

    // Point count doubles per level across an iterated refinement.
    const RefineResult r = refine(chaikin, sq, 3);
    CHECK(!r.diverged);
    CHECK(r.levels[1].size() == 8);
    CHECK(r.levels[2].size() == 16);
    CHECK(r.levels[3].size() == 32);

    // t = 1/2: both cuts collapse to the edge midpoint (degenerate pairs).
    const ChaikinScheme half(0.5f);
    const std::vector<Vec2> mid = half.step(sq.pts, sq.closed);
    CHECK(near(mid[0], mid[1]));

    // Open polylines keep their endpoints untouched.
    Polygon open = presets::square();
    open.closed = false;
    const std::vector<Vec2> openOnce = chaikin.step(open.pts, open.closed);
    CHECK(openOnce.size() == 8); // 2 endpoints + 2 cuts on each of 3 edges
    CHECK(near(openOnce.front(), open.pts.front()));
    CHECK(near(openOnce.back(), open.pts.back()));
}

static void testFourPoint() {
    const Polygon sq = presets::square(); // (-.7,-.7) (.7,-.7) (.7,.7) (-.7,.7)
    const FourPointScheme fourPoint(0.0625f);

    const std::vector<Vec2> once = fourPoint.step(sq.pts, sq.closed);
    CHECK(once.size() == 8); // point count doubles for closed polygons

    // Interpolation property: every original point survives, at even indices.
    for (size_t i = 0; i < sq.pts.size(); ++i)
        CHECK(near(once[2 * i], sq.pts[i]));

    // Hand-computed insertion on the bottom edge at w = 1/16:
    // M = (1/2 + w)(P0 + P1) - w(P3 + P2)
    //   = 0.5625*(0, -1.4) - 0.0625*(0, 1.4) = (0, -0.875)
    // — bulges outward past the edge, as an interpolating curve must.
    CHECK(near(once[1], Vec2{0.0f, -0.875f}));

    // The interpolation property holds across iterated refinement too.
    const RefineResult r = refine(fourPoint, sq, 3);
    CHECK(!r.diverged);
    CHECK(r.levels[3].size() == 32);
    for (size_t i = 0; i < sq.pts.size(); ++i)
        CHECK(near(r.levels[3][8 * i], sq.pts[i])); // spacing doubles per level

    // w = 0 degenerates to plain midpoint insertion.
    const FourPointScheme zero(0.0f);
    const std::vector<Vec2> mids = zero.step(sq.pts, sq.closed);
    CHECK(near(mids[1], Vec2{0.0f, -0.7f}));

    // Open polylines: endpoints kept, count is 2n-1.
    Polygon open = presets::square();
    open.closed = false;
    const std::vector<Vec2> openOnce = fourPoint.step(open.pts, open.closed);
    CHECK(openOnce.size() == 7);
    CHECK(near(openOnce.front(), open.pts.front()));
    CHECK(near(openOnce.back(), open.pts.back()));
}

static size_t edgeCount(const TriMesh& m) {
    std::set<std::pair<int, int>> edges;
    for (const auto& t : m.tris)
        for (int e = 0; e < 3; ++e)
            edges.insert(std::minmax(t[e], t[(e + 1) % 3]));
    return edges.size();
}

static void testLoopSubdivision() {
    const TriMesh base = subdiv::cube(0.7f);
    CHECK(base.verts.size() == 8);
    CHECK(base.tris.size() == 12);
    CHECK(edgeCount(base) == 18); // 12 cube edges + 6 face diagonals
    CHECK(8 - 18 + 12 == 2);      // Euler characteristic of a sphere

    // One round: V' = V + E, F' = 4F, E' = 2E + 3F; Euler must hold.
    const TriMesh once = loopSubdivide(base);
    CHECK(once.verts.size() == 26);
    CHECK(once.tris.size() == 48);
    CHECK(edgeCount(once) == 72);
    CHECK(26 - 72 + 48 == 2);

    const TriMesh twice = loopSubdivide(once);
    CHECK(twice.verts.size() == 98); // 26 + 72
    CHECK(twice.tris.size() == 192);
    CHECK(static_cast<long>(twice.verts.size()) - static_cast<long>(edgeCount(twice)) +
              static_cast<long>(twice.tris.size()) == 2);

    // Loop is approximating: nothing may leave the cube. After one round the
    // face-interior edge points still lie exactly on the (flat) face planes,
    // so the maximum only becomes strictly interior from the second round on.
    const auto maxAbsCoord = [](const TriMesh& m) {
        float maxAbs = 0.0f;
        for (Vec3 v : m.verts)
            maxAbs = std::max({maxAbs, std::fabs(v.x), std::fabs(v.y), std::fabs(v.z)});
        return maxAbs;
    };
    bool allFinite = true;
    for (Vec3 v : twice.verts)
        allFinite = allFinite && subdiv::isFinite(v);
    CHECK(allFinite);
    CHECK(maxAbsCoord(once) <= 0.7f + 1e-6f);
    CHECK(maxAbsCoord(twice) < 0.7f);
}

int main() {
    testVec2();
    testPresets();
    testRefine();
    testPerimeter();
    testDivergenceGuard();
    testChaikin();
    testFourPoint();
    testLoopSubdivision();

    if (g_failures == 0) {
        std::printf("All tests passed.\n");
        return 0;
    }
    std::printf("%d check(s) FAILED.\n", g_failures);
    return 1;
}
