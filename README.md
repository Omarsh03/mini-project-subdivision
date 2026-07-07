# Interactive 2D Subdivision Visualizer

[![CI](https://github.com/Omarsh03/mini-project-subdivision/actions/workflows/ci.yml/badge.svg?branch=mini-project%2Fsubdivision)](https://github.com/Omarsh03/mini-project-subdivision/actions/workflows/ci.yml)

A C++ desktop application that visualizes curve subdivision schemes —
**Chaikin corner cutting** (approximating) and the **four-point scheme**
(interpolating) — side by side on a shared, editable control polygon, with
free control over the subdivision weights and iteration count.

![Chaikin vs four-point on the star preset](docs/img/hero.png)

Computer Graphics course mini-project (solo). Planning documents:
[PRD.md](PRD.md), [PLAN.md](PLAN.md), [TODO.md](TODO.md).
Course report with the algorithm write-up and weight-convergence
experiments: **[docs/REPORT.md](docs/REPORT.md)**.

## Features

- Chaikin corner cutting with adjustable cut ratio `t` (canonical 1/4 : 3/4)
- Four-point interpolating subdivision with adjustable tension `w`
  (canonical 1/16), C¹ bound annotated in the UI
- Side-by-side viewports sharing one control polygon — the
  approximating-vs-interpolating contrast at a glance
- 0–8 iterations with a ghosted step-by-step convergence view
- Weight sliders reach far beyond the canonical values; live perimeter
  statistics flag when refinement stops converging (rough/fractal regime),
  plus a hard guard against numeric blow-up
- **Live convergence plots** per scheme: perimeter and max edge length per
  level, flipping visibly from decaying-to-flat to growing as weights leave
  the convergent regime
- **Open-polyline mode** with an arc preset — endpoints stay fixed while the
  interior smooths
- **Asymmetric Chaikin**: unlink the cut sliders to give the two cuts
  independent depths t₁/t₂
- Draggable control vertices, shape presets (square, star, zig-zag, random,
  arc)
- **3D bonus**: Loop subdivision of a cube as a rotating, depth-cued
  wireframe (switchable mode, 0–5 rounds)
- Reproducible screenshot mode for documentation

## Prerequisites (Ubuntu / WSL2)

```sh
sudo apt install -y cmake g++ libsdl2-dev
```

Dear ImGui v1.92.8 is vendored in `external/imgui/` — no further dependencies.

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

Unit tests cover the math core: vector ops, presets, hand-computed
subdivision steps for both schemes, the interpolation property, perimeter
measurement, and the divergence guard.

## Controls

| Control | Effect |
|---|---|
| **2D curves / 3D Loop** | Switch between the curve comparison and the 3D bonus view |
| **iterations** slider | Number of subdivision rounds (0–8), applied to both schemes |
| **cut ratio t** slider | Chaikin cut parameter; `reset` returns to the canonical 0.25 |
| **link cuts (symmetric)** | Unlink to control the two Chaikin cuts t₁ (from A) and t₂ (from B) independently |
| **tension w** slider | Four-point tension; `reset` returns to the canonical 0.0625 |
| **Square / Star / Zig-zag / Random / Arc** | Load a preset control polygon (Random differs each click; Arc is open) |
| **closed polygon** | Toggle between a closed polygon and an open polyline with fixed endpoints |
| **control polygon / handles** | Toggle drawing of the input polygon and its vertex markers |
| **intermediate levels** | Ghosted earlier levels showing convergence toward the limit curve |
| **Left-drag a vertex** | Move a control point (works in either viewport; both update) |
| **Loop iterations** (3D mode) | Rounds of Loop subdivision on the cube (0–5); the base cube shows as a ghost |

Per scheme, the panel reports levels computed, point count, live plots of
perimeter and max edge length across all levels, and the finest level's
perimeter with its per-level growth ratio. An orange warning appears when
the perimeter keeps growing (the scheme is no longer converging); a red one
if coordinates overflow the safety bound entirely.

## Screenshot mode

Renders one frame with the given parameters and exits — used to produce
every image in the report reproducibly:

```sh
./build/subdivision_visualizer --screenshot out.bmp \
    --preset star --iterations 6 --fourpoint-w 0.30
```

Flags: `--preset square|star|zigzag|random|arc`, `--iterations n`,
`--chaikin-t x`, `--chaikin-t2 x`, `--fourpoint-w x`, `--mode 2d|3d`,
`--loop-iterations n`.

## Repository layout

```
include/subdiv/   header-only subdivision math (no SDL/ImGui) — unit-tested
src/              application: main loop, app state, ImGui panel, canvas,
                  viewport mapping, mouse input
tests/            assert-based tests, wired to CTest
external/imgui/   vendored Dear ImGui (pinned v1.92.8, MIT)
docs/             course report and screenshots
```
