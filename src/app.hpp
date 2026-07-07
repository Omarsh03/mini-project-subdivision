#pragma once

#include "subdiv/chaikin.hpp"
#include "subdiv/fourpoint.hpp"
#include "subdiv/polygon.hpp"
#include "subdiv/scheme.hpp"
#include "subdiv/trimesh.hpp"

// Canonical parameter values, shared by the UI (markers, reset buttons)
// and the initial state.
namespace canonical {
inline constexpr float kChaikinRatio = 0.25f;    // the classic 1/4 : 3/4 rule
inline constexpr float kFourPointTension = 0.0625f; // 1/16, cubic precision
// The four-point scheme's limit curve is C^1 only for 0 < w < (sqrt(5)-1)/8.
inline constexpr float kFourPointC1Bound = 0.1545085f;
} // namespace canonical

enum class ViewMode { Curves2D, Loop3D };

// Render-only options; these never invalidate the refinement caches.
struct DisplayOptions {
    bool showControlPolygon = true;
    bool showHandles = true;
    bool showIntermediateLevels = true; // ghosted convergence view
};

// The whole application model: one control polygon, two parameterized
// schemes, and the cached refinement of both. UI and input mutate the
// parameters and set `dirty`; recomputeIfDirty() refreshes the caches
// once per frame at most.
struct AppState {
    subdiv::Polygon polygon = subdiv::presets::star();
    subdiv::ChaikinScheme chaikin{canonical::kChaikinRatio};
    subdiv::FourPointScheme fourPoint{canonical::kFourPointTension};
    int iterations = 4;
    DisplayOptions display;

    // Symmetric Chaikin (t1 == t2) by default; unlinking exposes both cuts.
    bool linkChaikinCuts = true;

    // 3D bonus: Loop subdivision of a cube, shown in a separate view mode.
    ViewMode mode = ViewMode::Curves2D;
    int loopIterations = 2;
    subdiv::TriMesh loopMesh;

    // Vertex-drag state: index into polygon.pts, and which viewport
    // (0 = left, 1 = right) the drag started in — its transform maps the
    // mouse back to world space.
    int draggedVertex = -1;
    int draggedViewport = 0;

    std::uint32_t nextRandomSeed = 1; // bumps so each Random click differs

    bool dirty = true;
    subdiv::RefineResult chaikinResult;
    subdiv::RefineResult fourPointResult;

    void recomputeIfDirty();
};
