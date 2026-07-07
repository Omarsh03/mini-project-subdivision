// Entry point: SDL2 window + Dear ImGui context and the main loop.
// Renders Chaikin (left) and four-point (right) subdivision of a shared
// control polygon side by side, with a live parameter panel.

#include <SDL.h>
#include <cstdio>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "app.hpp"
#include "canvas.hpp"
#include "input.hpp"
#include "ui.hpp"
#include "viewport.hpp"

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Subdivision Visualizer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 760,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    AppState state;

    const SDL_Color kControlColor{140, 140, 150, 255};
    const SDL_Color kHandleColor{220, 220, 230, 255};
    const SDL_Color kChaikinColor{86, 156, 255, 255};
    const SDL_Color kFourPointColor{255, 170, 66, 255};
    const SDL_Color kDividerColor{70, 74, 84, 255};

    // Viewports persist across frames so event handling can hit-test against
    // last frame's layout (at worst one frame stale after a resize).
    Viewport left, right;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            const bool isMouse = event.type == SDL_MOUSEBUTTONDOWN ||
                                 event.type == SDL_MOUSEBUTTONUP ||
                                 event.type == SDL_MOUSEMOTION;
            if (isMouse && !ImGui::GetIO().WantCaptureMouse)
                input::handleEvent(state, event, left, right);
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        state.recomputeIfDirty(); // panel reads stats from the caches
        ui::drawPanel(state);
        state.recomputeIfDirty(); // apply this frame's slider changes

        int outW = 0, outH = 0;
        SDL_GetRendererOutputSize(renderer, &outW, &outH);
        const float w = static_cast<float>(outW), h = static_cast<float>(outH);

        left.screen = {0.0f, 0.0f, w / 2.0f, h};
        right.screen = {w / 2.0f, 0.0f, w / 2.0f, h};

        // Viewport labels, drawn behind ImGui windows but over the canvas.
        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        bg->AddText(ImVec2(left.screen.x + 12.0f, 10.0f),
                    IM_COL32(kChaikinColor.r, kChaikinColor.g, kChaikinColor.b, 255),
                    "Chaikin (approximating)");
        bg->AddText(ImVec2(right.screen.x + 12.0f, 10.0f),
                    IM_COL32(kFourPointColor.r, kFourPointColor.g, kFourPointColor.b, 255),
                    "Four-point (interpolating)");

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 24, 26, 32, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, kDividerColor.r, kDividerColor.g, kDividerColor.b, 255);
        SDL_RenderDrawLineF(renderer, w / 2.0f, 0.0f, w / 2.0f, h);

        // One scheme view per viewport: control polygon, ghosted intermediate
        // levels fading in toward the finest curve, then handles on top.
        const auto drawSchemeView = [&](const Viewport& vp,
                                        const subdiv::RefineResult& result,
                                        SDL_Color color) {
            const subdiv::Polygon& polygon = state.polygon;
            if (state.display.showControlPolygon)
                canvas::drawPolyline(renderer, vp, polygon.pts, polygon.closed, kControlColor);
            const size_t last = result.levels.size() - 1;
            if (state.display.showIntermediateLevels && last > 1) {
                for (size_t k = 1; k < last; ++k) {
                    SDL_Color ghost = color;
                    ghost.a = static_cast<Uint8>(
                        40.0f + 120.0f * static_cast<float>(k) / static_cast<float>(last));
                    canvas::drawPolyline(renderer, vp, result.levels[k], polygon.closed, ghost);
                }
            }
            canvas::drawPolyline(renderer, vp, result.levels.back(), polygon.closed, color);
            if (state.display.showHandles)
                canvas::drawHandles(renderer, vp, polygon.pts, kHandleColor);
        };

        drawSchemeView(left, state.chaikinResult, kChaikinColor);
        drawSchemeView(right, state.fourPointResult, kFourPointColor);

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
