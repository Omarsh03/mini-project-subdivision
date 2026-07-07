#pragma once

#include <SDL.h>

#include "app.hpp"
#include "viewport.hpp"

namespace input {

// Vertex dragging in either viewport. The caller filters out events that
// ImGui wants (WantCaptureMouse) before calling this.
void handleEvent(AppState& state, const SDL_Event& event,
                 const Viewport& left, const Viewport& right);

} // namespace input
