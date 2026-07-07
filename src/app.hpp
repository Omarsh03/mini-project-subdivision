#pragma once

#include "subdiv/chaikin.hpp"
#include "subdiv/fourpoint.hpp"
#include "subdiv/polygon.hpp"
#include "subdiv/scheme.hpp"

// Canonical parameter values, shared by the UI (markers, reset buttons)
// and the initial state.
namespace canonical {
inline constexpr float kChaikinRatio = 0.25f;    // the classic 1/4 : 3/4 rule
inline constexpr float kFourPointTension = 0.0625f; // 1/16, cubic precision
// The four-point scheme's limit curve is C^1 only for 0 < w < (sqrt(5)-1)/8.
inline constexpr float kFourPointC1Bound = 0.1545085f;
} // namespace canonical

// The whole application model: one control polygon, two parameterized
// schemes, and the cached refinement of both. UI and input mutate the
// parameters and set `dirty`; recomputeIfDirty() refreshes the caches
// once per frame at most.
struct AppState {
    subdiv::Polygon polygon = subdiv::presets::star();
    subdiv::ChaikinScheme chaikin{canonical::kChaikinRatio};
    subdiv::FourPointScheme fourPoint{canonical::kFourPointTension};
    int iterations = 4;

    bool dirty = true;
    subdiv::RefineResult chaikinResult;
    subdiv::RefineResult fourPointResult;

    void recomputeIfDirty();
};
