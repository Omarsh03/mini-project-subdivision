#include "ui.hpp"

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

void schemeStats(const subdiv::RefineResult& result, int requestedIterations) {
    const std::size_t levels = result.levels.size();
    ImGui::Text("levels computed: %zu / %d, points at finest: %zu",
                levels - 1, requestedIterations, result.levels.back().size());
    if (result.diverged) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.3f, 1.0f),
                           "DIVERGING — refinement stopped early");
    }
}

} // namespace

void drawPanel(AppState& state) {
    ImGui::SetNextWindowPos(ImVec2(10, 34), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    if (ImGui::SliderInt("iterations", &state.iterations, 0, 8))
        state.dirty = true;

    ImGui::SeparatorText("Chaikin (approximating)");
    if (weightControl("cut ratio t", &state.chaikin.t, 0.01f, 0.99f,
                      canonical::kChaikinRatio,
                      "canonical t = 0.25 (the 1/4 : 3/4 rule); "
                      "t = 0.5 degenerates, t > 0.5 cuts cross over"))
        state.dirty = true;
    schemeStats(state.chaikinResult, state.iterations);

    ImGui::SeparatorText("Four-point (interpolating)");
    if (weightControl("tension w", &state.fourPoint.w, -0.25f, 0.40f,
                      canonical::kFourPointTension,
                      "canonical w = 1/16 = 0.0625; C1 only for "
                      "0 < w < (sqrt(5)-1)/8 ~= 0.1545"))
        state.dirty = true;
    schemeStats(state.fourPointResult, state.iterations);

    ImGui::End();
}

} // namespace ui
