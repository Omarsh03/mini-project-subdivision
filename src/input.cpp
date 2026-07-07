#include "input.hpp"

namespace input {
namespace {

constexpr float kHitRadiusPx = 10.0f;

int hitVertex(const AppState& state, const Viewport& vp, float mx, float my) {
    for (size_t i = 0; i < state.polygon.pts.size(); ++i) {
        const SDL_FPoint p = vp.toScreen(state.polygon.pts[i]);
        const float dx = p.x - mx, dy = p.y - my;
        if (dx * dx + dy * dy <= kHitRadiusPx * kHitRadiusPx)
            return static_cast<int>(i);
    }
    return -1;
}

} // namespace

void handleEvent(AppState& state, const SDL_Event& event,
                 const Viewport& left, const Viewport& right) {
    const Viewport* viewports[2] = {&left, &right};

    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button != SDL_BUTTON_LEFT)
            break;
        const float mx = static_cast<float>(event.button.x);
        const float my = static_cast<float>(event.button.y);
        for (int v = 0; v < 2; ++v) {
            if (!viewports[v]->contains(mx, my))
                continue;
            const int hit = hitVertex(state, *viewports[v], mx, my);
            if (hit >= 0) {
                state.draggedVertex = hit;
                state.draggedViewport = v;
            }
            break; // viewports don't overlap; no need to test the other
        }
        break;
    }
    case SDL_MOUSEMOTION: {
        if (state.draggedVertex < 0 ||
            state.draggedVertex >= static_cast<int>(state.polygon.pts.size()))
            break;
        const Viewport& vp = *viewports[state.draggedViewport];
        state.polygon.pts[static_cast<size_t>(state.draggedVertex)] =
            vp.toWorld(static_cast<float>(event.motion.x),
                       static_cast<float>(event.motion.y));
        state.dirty = true;
        break;
    }
    case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT)
            state.draggedVertex = -1;
        break;
    default:
        break;
    }
}

} // namespace input
