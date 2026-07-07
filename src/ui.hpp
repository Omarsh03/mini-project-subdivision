#pragma once

#include "app.hpp"

namespace ui {

// Builds the ImGui control panel; mutates `state` and sets state.dirty
// whenever a parameter changes.
void drawPanel(AppState& state);

} // namespace ui
