#include "mesh_view.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

namespace meshview {
namespace {

constexpr float kTilt = 0.45f;    // fixed camera tilt (radians)
constexpr float kCamDist = 4.0f;  // camera distance for the perspective divide

struct Projected {
    SDL_FPoint screen;
    float depth; // camera-space z after rotation+tilt; larger = farther
};

Projected project(subdiv::Vec3 v, float angle, const Viewport& vp) {
    const float cy = std::cos(angle), sy = std::sin(angle);
    const float x = cy * v.x + sy * v.z;
    const float z = -sy * v.x + cy * v.z;
    const float ct = std::cos(kTilt), st = std::sin(kTilt);
    const float y2 = ct * v.y - st * z;
    const float z2 = st * v.y + ct * z;
    const float persp = kCamDist / (kCamDist + z2);
    return {vp.toScreen({x * persp, y2 * persp}), z2};
}

} // namespace

void draw(SDL_Renderer* renderer, const Viewport& vp,
          const subdiv::TriMesh& mesh, float angle, SDL_Color color) {
    std::vector<Projected> proj;
    proj.reserve(mesh.verts.size());
    for (subdiv::Vec3 v : mesh.verts)
        proj.push_back(project(v, angle, vp));

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    std::set<std::pair<int, int>> drawn; // shared edges only once
    for (const auto& t : mesh.tris) {
        for (int e = 0; e < 3; ++e) {
            const std::pair<int, int> key = std::minmax(t[e], t[(e + 1) % 3]);
            if (!drawn.insert(key).second)
                continue;
            const Projected& a = proj[static_cast<size_t>(key.first)];
            const Projected& b = proj[static_cast<size_t>(key.second)];
            const float depth = 0.5f * (a.depth + b.depth);
            const float alpha =
                std::clamp(170.0f - 110.0f * depth, 45.0f, 250.0f);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b,
                                   static_cast<Uint8>(alpha));
            SDL_RenderDrawLineF(renderer, a.screen.x, a.screen.y,
                                b.screen.x, b.screen.y);
        }
    }
}

} // namespace meshview
