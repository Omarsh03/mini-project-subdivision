#include "app.hpp"

void AppState::recomputeIfDirty() {
    if (!dirty)
        return;
    chaikinResult = subdiv::refine(chaikin, polygon, iterations);
    fourPointResult = subdiv::refine(fourPoint, polygon, iterations);
    loopMesh = subdiv::cube();
    for (int i = 0; i < loopIterations; ++i)
        loopMesh = subdiv::loopSubdivide(loopMesh);
    dirty = false;
}
