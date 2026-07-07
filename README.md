# Interactive 2D Subdivision Visualizer

A C++ desktop application that visualizes curve subdivision schemes —
**Chaikin corner cutting** (approximating) and the **four-point scheme**
(interpolating) — side by side on a shared, editable control polygon, with
free control over the subdivision weights and iteration count.

Computer Graphics course mini-project. Planning documents: [PRD.md](PRD.md),
[PLAN.md](PLAN.md), [TODO.md](TODO.md).

> **Status:** Phase 0 — project skeleton (SDL2 window + empty ImGui panel).

## Prerequisites (Ubuntu / WSL2)

```sh
sudo apt install -y cmake g++ libsdl2-dev
```

Dear ImGui is vendored in `external/imgui/` — no further dependencies.

## Build

```sh
cmake -B build
cmake --build build -j
```

## Run

```sh
./build/subdivision_visualizer
```

Under WSL2 the window opens on the Windows desktop via WSLg.

## Tests

```sh
ctest --test-dir build
```

## Controls

To be documented as features land (see [TODO.md](TODO.md)).
