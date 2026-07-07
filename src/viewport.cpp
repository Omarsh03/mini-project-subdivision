#include "viewport.hpp"

#include <algorithm>

float Viewport::scale() const {
    return std::min(screen.w, screen.h) / worldSpan;
}

SDL_FPoint Viewport::toScreen(subdiv::Vec2 world) const {
    const float s = scale();
    const float cx = screen.x + screen.w / 2.0f;
    const float cy = screen.y + screen.h / 2.0f;
    return {cx + world.x * s, cy - world.y * s};
}

subdiv::Vec2 Viewport::toWorld(float screenX, float screenY) const {
    const float s = scale();
    const float cx = screen.x + screen.w / 2.0f;
    const float cy = screen.y + screen.h / 2.0f;
    return {(screenX - cx) / s, -(screenY - cy) / s};
}

bool Viewport::contains(float screenX, float screenY) const {
    return screenX >= screen.x && screenX < screen.x + screen.w &&
           screenY >= screen.y && screenY < screen.y + screen.h;
}
