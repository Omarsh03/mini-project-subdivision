#pragma once

#include <SDL.h>

#include "subdiv/trimesh.hpp"
#include "viewport.hpp"

namespace meshview {

// Draws a triangle mesh as a wireframe: rotation about the Y axis by
// `angle` (plus a fixed viewing tilt), simple perspective projection into
// the viewport's world space, and depth-cued line alpha (nearer = brighter).
void draw(SDL_Renderer* renderer, const Viewport& vp,
          const subdiv::TriMesh& mesh, float angle, SDL_Color color);

} // namespace meshview
