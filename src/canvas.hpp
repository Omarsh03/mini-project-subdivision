#pragma once

#include <vector>

#include <SDL.h>

#include "subdiv/vec2.hpp"
#include "viewport.hpp"

// Low-level drawing of polylines and vertex handles on an SDL_Renderer.
namespace canvas {

void drawPolyline(SDL_Renderer* renderer, const Viewport& vp,
                  const std::vector<subdiv::Vec2>& pts, bool closed,
                  SDL_Color color);

// Small filled squares marking control vertices, sized in pixels.
void drawHandles(SDL_Renderer* renderer, const Viewport& vp,
                 const std::vector<subdiv::Vec2>& pts, SDL_Color color,
                 float sizePx = 7.0f);

} // namespace canvas
