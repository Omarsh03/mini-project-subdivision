#include "app.hpp"

void AppState::recomputeIfDirty() {
    if (!dirty)
        return;
    chaikinResult = subdiv::refine(chaikin, polygon, iterations);
    fourPointResult = subdiv::refine(fourPoint, polygon, iterations);
    dirty = false;
}
