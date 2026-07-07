# TODO — Phase Breakdown

Legend: **[REQUIRED]** must be done · **[OPTIONAL]** nice-to-have / stretch.
Every phase ends with a **git commit + push** to `mini-project/subdivision`.

References: features (M/N/S numbers) are defined in [PRD.md](PRD.md),
design details in [PLAN.md](PLAN.md).

---

## Phase 0 — Repo scaffolding & build system [REQUIRED]

- [x] Prerequisites installed: `cmake`, `libsdl2-dev` (user runs the apt command)
- [x] `.gitignore` (build/, binaries, IDE files) and `.gitattributes` (LF)
- [x] Vendor Dear ImGui (pinned release) into `external/imgui/` with its LICENSE
- [x] `CMakeLists.txt`: app target (SDL2 + ImGui) + test target + CTest
- [x] Skeleton `src/main.cpp`: opens an SDL window with an empty ImGui panel,
      clean shutdown — proves the whole stack works under WSLg
- [x] Stub README.md (title + build instructions)
- [x] ☑ **Commit + push**: "Scaffold CMake/SDL2/ImGui project skeleton"
      (also push `main` so the empty remote gets a default branch)

## Phase 1 — Core subdivision math [REQUIRED]

- [x] `vec2.hpp`: Vec2 with +, −, scalar ×, lerp
- [x] `polygon.hpp`: Polygon struct + presets (square, star, random, zig-zag) (M7)
- [x] `scheme.hpp`: `SubdivisionScheme` interface, `refine()` with level
      retention and divergence guard (M8 backend)
- [x] `tests/test_subdivision.cpp` harness wired to CTest
- [x] ☑ **Commit + push**: "Add math core: Vec2, Polygon, scheme interface"

## Phase 2 — Chaikin corner cutting [REQUIRED]

- [x] `chaikin.hpp`: closed-polygon step with cut ratio `t` (M1)
- [x] Unit test: one hand-computed Chaikin step of a unit square at t = 1/4;
      point count doubles per level
- [x] Minimal canvas rendering (`viewport`, `canvas`): draw control polygon +
      subdivided curve for Chaikin only, fixed iteration count — first visual
- [x] ☑ **Commit + push**: "Implement Chaikin corner cutting with adjustable ratio"

## Phase 3 — Four-point scheme & side-by-side view [REQUIRED]

- [x] `fourpoint.hpp`: closed-polygon step with tension `w` (M2)
- [x] Unit tests: original points preserved (interpolation property);
      hand-computed inserted point at w = 1/16
- [x] Split window into two viewports sharing one control polygon (M3)
- [x] ☑ **Commit + push**: "Add four-point scheme and side-by-side viewports"

## Phase 4 — Weight experimentation UI [REQUIRED]

- [x] ImGui panel: iterations slider 0–8 (M5); Chaikin `t` slider and
      four-point `w` slider with canonical markers + reset buttons (M4)
- [x] Recompute-on-change wiring (`dirty` flag)
- [x] Divergence detection surfaced as a red warning + stats readout
      (points per level) (M8)
- [x] Annotate the C¹ bound `w < (√5−1)/8` in the four-point section
- [x] ☑ **Commit + push**: "Add weight experimentation panel with divergence warning"

## Phase 5 — Interaction & visualization polish [REQUIRED]

- [x] Vertex dragging with mouse in both viewports (M7),
      respecting `WantCaptureMouse`
- [x] Preset buttons wired into the panel (M7)
- [x] Step-by-step convergence view: ghosted intermediate levels toggle (M6)
- [x] Display toggles (control polygon / handles / final-only)
- [x] Color & stroke pass: readable palette, control polygon visually distinct
      from curve levels
- [x] ☑ **Commit + push**: "Add vertex editing, presets, and convergence view"

## Phase 6 — Documentation & report [REQUIRED]

- [x] README.md: description, screenshot, build (incl. apt prerequisites),
      run, full controls reference
- [x] Run the weight experiments; capture screenshots into `docs/img/`
      (canonical, rough, fractal, divergent for each scheme)
- [x] `docs/REPORT.md`: implementation walkthrough, both algorithms explained,
      design decisions, experiment results with screenshots, challenges
- [x] Final code pass: consistent naming, remove dead code, header comments
- [x] ☑ **Commit + push**: "Add README and course report with experiment results"

## Phase 7 — Optional extras [OPTIONAL]

Only if time remains, in this order (each independently committable):

- [ ] N1: insert/delete control vertices with the mouse
      — ☑ commit + push "Add vertex insertion/deletion"
- [ ] N3: asymmetric Chaikin (independent t₁/t₂)
      — ☑ commit + push "Add asymmetric Chaikin cuts"
- [ ] N2: open-polyline mode with endpoint handling
      — ☑ commit + push "Support open polylines"
- [ ] S1: 3D bonus — Loop subdivision on a cube, wireframe render
      (separate viewport or mode; no extraordinary-vertex correctness needed)
      — ☑ commit + push "Add Loop subdivision 3D bonus"
- [ ] Update README/REPORT for whichever extras landed
      — ☑ commit + push "Document optional features"
