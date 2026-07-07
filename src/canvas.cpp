#include "canvas.hpp"

namespace canvas {

void drawPolyline(SDL_Renderer* renderer, const Viewport& vp,
                  const std::vector<subdiv::Vec2>& pts, bool closed,
                  SDL_Color color) {
    if (pts.size() < 2)
        return;
    std::vector<SDL_FPoint> screen;
    screen.reserve(pts.size() + 1);
    for (subdiv::Vec2 p : pts)
        screen.push_back(vp.toScreen(p));
    if (closed)
        screen.push_back(screen.front());
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLinesF(renderer, screen.data(), static_cast<int>(screen.size()));
}

void drawHandles(SDL_Renderer* renderer, const Viewport& vp,
                 const std::vector<subdiv::Vec2>& pts, SDL_Color color,
                 float sizePx) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (subdiv::Vec2 p : pts) {
        const SDL_FPoint c = vp.toScreen(p);
        const SDL_FRect r{c.x - sizePx / 2.0f, c.y - sizePx / 2.0f, sizePx, sizePx};
        SDL_RenderFillRectF(renderer, &r);
    }
}

} // namespace canvas
