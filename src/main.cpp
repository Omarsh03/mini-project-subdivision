// Entry point: SDL2 window + Dear ImGui context and the main loop.
// Renders Chaikin (left) and four-point (right) subdivision of a shared
// preset polygon at a fixed iteration count, side by side; interactive
// controls arrive in later phases.

#include <SDL.h>
#include <cstdio>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "subdiv/chaikin.hpp"
#include "subdiv/fourpoint.hpp"
#include "subdiv/polygon.hpp"
#include "subdiv/scheme.hpp"

#include "canvas.hpp"
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

    const subdiv::Polygon polygon = subdiv::presets::star();
    const subdiv::ChaikinScheme chaikin(0.25f);
    const subdiv::FourPointScheme fourPoint(0.0625f);
    const int iterations = 4;

    const SDL_Color kControlColor{140, 140, 150, 255};
    const SDL_Color kHandleColor{220, 220, 230, 255};
    const SDL_Color kChaikinColor{86, 156, 255, 255};
    const SDL_Color kFourPointColor{255, 170, 66, 255};
    const SDL_Color kDividerColor{70, 74, 84, 255};

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
        ImGui::TextUnformatted("Fixed parameters: t = 0.25, w = 1/16, 4 iterations.");
        ImGui::TextUnformatted("Gray: shared control polygon.");
        ImGui::TextUnformatted("Left/blue: Chaikin approximates (pulls inside).");
        ImGui::TextUnformatted("Right/orange: four-point interpolates (passes through).");
        ImGui::TextUnformatted("Sliders and editing arrive in later phases.");
        ImGui::End();

        int outW = 0, outH = 0;
        SDL_GetRendererOutputSize(renderer, &outW, &outH);
        const float w = static_cast<float>(outW), h = static_cast<float>(outH);

        Viewport left, right;
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

        const subdiv::RefineResult chaikinResult = subdiv::refine(chaikin, polygon, iterations);
        const subdiv::RefineResult fourPointResult = subdiv::refine(fourPoint, polygon, iterations);

        SDL_SetRenderDrawColor(renderer, kDividerColor.r, kDividerColor.g, kDividerColor.b, 255);
        SDL_RenderDrawLineF(renderer, w / 2.0f, 0.0f, w / 2.0f, h);

        canvas::drawPolyline(renderer, left, polygon.pts, polygon.closed, kControlColor);
        canvas::drawHandles(renderer, left, polygon.pts, kHandleColor);
        canvas::drawPolyline(renderer, left, chaikinResult.levels.back(), polygon.closed, kChaikinColor);

        canvas::drawPolyline(renderer, right, polygon.pts, polygon.closed, kControlColor);
        canvas::drawHandles(renderer, right, polygon.pts, kHandleColor);
        canvas::drawPolyline(renderer, right, fourPointResult.levels.back(), polygon.closed, kFourPointColor);

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
