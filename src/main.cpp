// Entry point: SDL2 window + Dear ImGui context and the main loop.
// Renders Chaikin (left) and four-point (right) subdivision of a shared
// control polygon side by side, with a live parameter panel.
//
// A headless-ish screenshot mode exists for reproducible documentation
// images:  --screenshot out.bmp [--preset name] [--iterations n]
//          [--chaikin-t x] [--fourpoint-w x]

#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "app.hpp"
#include "canvas.hpp"
#include "input.hpp"
#include "mesh_view.hpp"
#include "ui.hpp"
#include "viewport.hpp"

namespace {

struct CliOptions {
    const char* screenshotPath = nullptr; // when set: render, save, exit
    bool valid = true;
};

CliOptions parseArgs(int argc, char** argv, AppState& state) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        const char* value = (i + 1 < argc) ? argv[i + 1] : nullptr;
        if (std::strcmp(arg, "--screenshot") == 0 && value) {
            opts.screenshotPath = value;
            ++i;
        } else if (std::strcmp(arg, "--iterations") == 0 && value) {
            state.iterations = std::atoi(value);
            ++i;
        } else if (std::strcmp(arg, "--chaikin-t") == 0 && value) {
            state.chaikin.t = std::strtof(value, nullptr);
            ++i;
        } else if (std::strcmp(arg, "--fourpoint-w") == 0 && value) {
            state.fourPoint.w = std::strtof(value, nullptr);
            ++i;
        } else if (std::strcmp(arg, "--mode") == 0 && value) {
            if (std::strcmp(value, "2d") == 0)
                state.mode = ViewMode::Curves2D;
            else if (std::strcmp(value, "3d") == 0)
                state.mode = ViewMode::Loop3D;
            else
                opts.valid = false;
            ++i;
        } else if (std::strcmp(arg, "--loop-iterations") == 0 && value) {
            state.loopIterations = std::atoi(value);
            ++i;
        } else if (std::strcmp(arg, "--preset") == 0 && value) {
            if (std::strcmp(value, "square") == 0)
                state.polygon = subdiv::presets::square();
            else if (std::strcmp(value, "star") == 0)
                state.polygon = subdiv::presets::star();
            else if (std::strcmp(value, "zigzag") == 0)
                state.polygon = subdiv::presets::zigzag();
            else if (std::strcmp(value, "random") == 0)
                state.polygon = subdiv::presets::random();
            else
                opts.valid = false;
            ++i;
        } else {
            opts.valid = false;
        }
        if (!opts.valid) {
            std::fprintf(stderr,
                         "usage: subdivision_visualizer [--screenshot out.bmp] "
                         "[--preset square|star|zigzag|random] [--iterations n] "
                         "[--chaikin-t x] [--fourpoint-w x] "
                         "[--mode 2d|3d] [--loop-iterations n]\n");
            break;
        }
    }
    return opts;
}

// Reads the current backbuffer (call before SDL_RenderPresent) into a BMP.
bool saveScreenshot(SDL_Renderer* renderer, const char* path) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_Surface* surface =
        SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
    if (!surface)
        return false;
    const bool ok =
        SDL_RenderReadPixels(renderer, nullptr, surface->format->format,
                             surface->pixels, surface->pitch) == 0 &&
        SDL_SaveBMP(surface, path) == 0;
    SDL_FreeSurface(surface);
    if (!ok)
        std::fprintf(stderr, "screenshot failed: %s\n", SDL_GetError());
    return ok;
}

} // namespace

int main(int argc, char** argv) {
    AppState state;
    const CliOptions cli = parseArgs(argc, argv, state);
    if (!cli.valid)
        return 2;

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
    ImGui::GetIO().IniFilename = nullptr; // deterministic layout, no stray file
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    const SDL_Color kControlColor{140, 140, 150, 255};
    const SDL_Color kHandleColor{220, 220, 230, 255};
    const SDL_Color kChaikinColor{86, 156, 255, 255};
    const SDL_Color kFourPointColor{255, 170, 66, 255};
    const SDL_Color kDividerColor{70, 74, 84, 255};
    const SDL_Color kMeshColor{110, 220, 170, 255};

    // Viewports persist across frames so event handling can hit-test against
    // last frame's layout (at worst one frame stale after a resize).
    Viewport left, right;

    bool running = true;
    int frame = 0;
    int exitCode = 0;
    // Fixed angle in screenshot mode keeps figures reproducible.
    float meshAngle = 0.6f;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            const bool isMouse = event.type == SDL_MOUSEBUTTONDOWN ||
                                 event.type == SDL_MOUSEBUTTONUP ||
                                 event.type == SDL_MOUSEMOTION;
            if (isMouse && !ImGui::GetIO().WantCaptureMouse &&
                state.mode == ViewMode::Curves2D)
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
        Viewport full;
        full.screen = {0.0f, 0.0f, w, h};

        // Viewport labels, drawn behind ImGui windows but over the canvas.
        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        if (state.mode == ViewMode::Curves2D) {
            bg->AddText(ImVec2(left.screen.x + 12.0f, 10.0f),
                        IM_COL32(kChaikinColor.r, kChaikinColor.g, kChaikinColor.b, 255),
                        "Chaikin (approximating)");
            bg->AddText(ImVec2(right.screen.x + 12.0f, 10.0f),
                        IM_COL32(kFourPointColor.r, kFourPointColor.g, kFourPointColor.b, 255),
                        "Four-point (interpolating)");
        } else {
            bg->AddText(ImVec2(12.0f, 10.0f),
                        IM_COL32(kMeshColor.r, kMeshColor.g, kMeshColor.b, 255),
                        "Loop subdivision on a cube (3D bonus)");
        }

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 24, 26, 32, 255);
        SDL_RenderClear(renderer);

        if (state.mode == ViewMode::Curves2D) {
            SDL_SetRenderDrawColor(renderer, kDividerColor.r, kDividerColor.g, kDividerColor.b, 255);
            SDL_RenderDrawLineF(renderer, w / 2.0f, 0.0f, w / 2.0f, h);

            // One scheme view per viewport: control polygon, ghosted
            // intermediate levels fading in toward the finest curve, then
            // handles on top.
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
        } else {
            if (!cli.screenshotPath)
                meshAngle += 0.008f; // slow turntable, ~0.5 rad/s at 60 fps
            if (state.display.showControlPolygon && state.loopIterations > 0) {
                SDL_Color ghost = kControlColor;
                ghost.a = 90;
                meshview::draw(renderer, full, subdiv::cube(), meshAngle, ghost);
            }
            meshview::draw(renderer, full, state.loopMesh, meshAngle, kMeshColor);
        }

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        // Give ImGui a couple of frames to settle its layout, then capture.
        if (cli.screenshotPath && ++frame >= 3) {
            exitCode = saveScreenshot(renderer, cli.screenshotPath) ? 0 : 1;
            running = false;
        }

        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return exitCode;
}
