#include "ui.hpp"

#include <cfloat>
#include <cstddef>

#include "imgui.h"

namespace ui {
namespace {

// Slider + canonical-value annotation + reset button for one scheme weight.
// Returns true if the value changed.
bool weightControl(const char* label, float* value, float min, float max,
                   float canonicalValue, const char* canonicalNote) {
    bool changed = ImGui::SliderFloat(label, value, min, max, "%.4f");
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::SmallButton("reset")) {
        *value = canonicalValue;
        changed = true;
    }
    ImGui::PopID();
    ImGui::TextDisabled("%s", canonicalNote);
    return changed;
}

// Sustained perimeter growth above this per-level ratio marks the
// rough/fractal regime; convergent settings decay to 1.0 (measured
// separation: <= 1.006 convergent vs >= 1.05 fractal on the presets).
constexpr float kGrowthWarnRatio = 1.02f;

// Iterations are capped at 8, so a refinement holds at most 9 levels.
constexpr int kMaxLevels = 9;

void schemeStats(const subdiv::RefineResult& result, int requestedIterations, bool closed) {
    const std::size_t levels = result.levels.size();
    ImGui::Text("levels computed: %zu / %d, points at finest: %zu",
                levels - 1, requestedIterations, result.levels.back().size());

    // Live convergence plots (M9): perimeter approaches the limit curve's
    // arc length when converging (flat tail) and grows geometrically in the
    // fractal regime; max edge length must tend to 0 for any convergent
    // scheme. Recomputed per frame — a few thousand float ops at most.
    float perims[kMaxLevels];
    float maxEdges[kMaxLevels];
    const int n = static_cast<int>(levels) < kMaxLevels ? static_cast<int>(levels) : kMaxLevels;
    for (int k = 0; k < n; ++k) {
        const auto& level = result.levels[static_cast<std::size_t>(k)];
        perims[k] = subdiv::perimeter(level, closed);
        maxEdges[k] = subdiv::maxEdgeLength(level, closed);
    }
    ImGui::PlotLines("perimeter / level", perims, n, 0, nullptr,
                     0.0f, FLT_MAX, ImVec2(0, 42));
    ImGui::PlotLines("max edge / level", maxEdges, n, 0, nullptr,
                     0.0f, FLT_MAX, ImVec2(0, 42));

    if (levels >= 2) {
        const float prev = perims[n - 2];
        const float last = perims[n - 1];
        const float ratio = prev > 0.0f ? last / prev : 1.0f;
        ImGui::Text("perimeter: %.3f (x%.3f per level)", last, ratio);
        if (ratio > kGrowthWarnRatio) {
            ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.2f, 1.0f),
                               "perimeter growing - NOT converging (rough/fractal)");
        }
    }
    if (result.diverged) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.3f, 1.0f),
                           "COORDINATES EXPLODED - refinement stopped early");
    }
}

} // namespace

void drawPanel(AppState& state) {
    ImGui::SetNextWindowPos(ImVec2(10, 34), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    if (ImGui::RadioButton("2D curves", state.mode == ViewMode::Curves2D))
        state.mode = ViewMode::Curves2D;
    ImGui::SameLine();
    if (ImGui::RadioButton("3D Loop (bonus)", state.mode == ViewMode::Loop3D))
        state.mode = ViewMode::Loop3D;

    if (state.mode == ViewMode::Loop3D) {
        ImGui::SeparatorText("Loop subdivision of a cube");
        if (ImGui::SliderInt("Loop iterations", &state.loopIterations, 0, 5))
            state.dirty = true;
        ImGui::Text("vertices: %zu, triangles: %zu",
                    state.loopMesh.verts.size(), state.loopMesh.tris.size());
        ImGui::TextDisabled("approximating: the cube rounds toward a smooth blob;\n"
                            "six of eight corners are extraordinary (valence 4),\n"
                            "handled by the standard valence-dependent weights");
        ImGui::Checkbox("show base cube", &state.display.showControlPolygon);
        ImGui::End();
        return;
    }

    if (ImGui::SliderInt("iterations", &state.iterations, 0, 8))
        state.dirty = true;

    ImGui::SeparatorText("Chaikin (approximating)");
    if (weightControl("cut ratio t", &state.chaikin.t, 0.01f, 0.99f,
                      canonical::kChaikinRatio,
                      "canonical t = 0.25 (the 1/4 : 3/4 rule); "
                      "t = 0.5 degenerates, t > 0.5 cuts cross over"))
        state.dirty = true;
    schemeStats(state.chaikinResult, state.iterations, state.polygon.closed);

    ImGui::SeparatorText("Four-point (interpolating)");
    if (weightControl("tension w", &state.fourPoint.w, -0.25f, 0.40f,
                      canonical::kFourPointTension,
                      "canonical w = 1/16 = 0.0625; C1 only for "
                      "0 < w < (sqrt(5)-1)/8 ~= 0.1545"))
        state.dirty = true;
    schemeStats(state.fourPointResult, state.iterations, state.polygon.closed);

    ImGui::SeparatorText("Control polygon");
    if (ImGui::Button("Square")) {
        state.polygon = subdiv::presets::square();
        state.dirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Star")) {
        state.polygon = subdiv::presets::star();
        state.dirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Zig-zag")) {
        state.polygon = subdiv::presets::zigzag();
        state.dirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Random")) {
        state.polygon = subdiv::presets::random(10, state.nextRandomSeed++);
        state.dirty = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Arc")) {
        state.polygon = subdiv::presets::arc();
        state.dirty = true;
    }
    if (ImGui::Checkbox("closed polygon", &state.polygon.closed))
        state.dirty = true;
    ImGui::TextDisabled("drag vertices with the mouse (in either viewport)");

    ImGui::SeparatorText("Display");
    ImGui::Checkbox("control polygon", &state.display.showControlPolygon);
    ImGui::SameLine();
    ImGui::Checkbox("handles", &state.display.showHandles);
    ImGui::Checkbox("intermediate levels (convergence view)",
                    &state.display.showIntermediateLevels);

    ImGui::End();
}

} // namespace ui
