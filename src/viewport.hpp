#pragma once

#include <SDL.h>

#include "subdiv/vec2.hpp"

// Maps world space (origin-centered, y up) into a pixel rectangle, y down,
// preserving aspect ratio. worldSpan world units fit the rectangle's
// smaller dimension, so presets in [-1,1]^2 stay comfortably visible.
struct Viewport {
    SDL_FRect screen{0.0f, 0.0f, 0.0f, 0.0f};
    float worldSpan = 2.4f;

    float scale() const;
    SDL_FPoint toScreen(subdiv::Vec2 world) const;
    subdiv::Vec2 toWorld(float screenX, float screenY) const;
    bool contains(float screenX, float screenY) const;
};
