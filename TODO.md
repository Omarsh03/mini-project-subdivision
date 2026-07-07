# TODO — Phase Breakdown

Legend: **[REQUIRED]** must be done · **[OPTIONAL]** nice-to-have / stretch.
Every phase ends with a **git commit + push** to `mini-project/subdivision`.

References: features (M/N/S numbers) are defined in [PRD.md](PRD.md),
design details in [PLAN.md](PLAN.md).

---

## Phase 0 — Repo scaffolding & build system [REQUIRED]

- [ ] Prerequisites installed: `cmake`, `libsdl2-dev` (user runs the apt command)
- [ ] `.gitignore` (build/, binaries, IDE files) and `.gitattributes` (LF)
- [ ] Vendor Dear ImGui (pinned release) into `external/imgui/` with its LICENSE
- [ ] `CMakeLists.txt`: app target (SDL2 + ImGui) + test target + CTest
- [ ] Skeleton `src/main.cpp`: opens an SDL window with an empty ImGui panel,
      clean shutdown — proves the whole stack works under WSLg
- [ ] Stub README.md (title + build instructions)
- [ ] ☑ **Commit + push**: "Scaffold CMake/SDL2/ImGui project skeleton"
      (also push `main` so the empty remote gets a default branch)

## Phase 1 — Core subdivision math [REQUIRED]

- [ ] `vec2.hpp`: Vec2 with +, −, scalar ×, lerp
- [ ] `polygon.hpp`: Polygon struct + presets (square, star, random, zig-zag) (M7)
- [ ] `scheme.hpp`: `SubdivisionScheme` interface, `refine()` with level
      retention and divergence guard (M8 backend)
- [ ] `tests/test_subdivision.cpp` harness wired to CTest
- [ ] ☑ **Commit + push**: "Add math core: Vec2, Polygon, scheme interface"

## Phase 2 — Chaikin corner cutting [REQUIRED]

- [ ] `chaikin.hpp`: closed-polygon step with cut ratio `t` (M1)
- [ ] Unit test: one hand-computed Chaikin step of a unit square at t = 1/4;
      point count doubles per level
- [ ] Minimal canvas rendering (`viewport`, `canvas`): draw control polygon +
      subdivided curve for Chaikin only, fixed iteration count — first visual
- [ ] ☑ **Commit + push**: "Implement Chaikin corner cutting with adjustable ratio"

## Phase 3 — Four-point scheme & side-by-side view [REQUIRED]

- [ ] `fourpoint.hpp`: closed-polygon step with tension `w` (M2)
- [ ] Unit tests: original points preserved (interpolation property);
      hand-computed inserted point at w = 1/16
- [ ] Split window into two viewports sharing one control polygon (M3)
- [ ] ☑ **Commit + push**: "Add four-point scheme and side-by-side viewports"

## Phase 4 — Weight experimentation UI [REQUIRED]

- [ ] ImGui panel: iterations slider 0–8 (M5); Chaikin `t` slider and
      four-point `w` slider with canonical markers + reset buttons (M4)
- [ ] Recompute-on-change wiring (`dirty` flag)
- [ ] Divergence detection surfaced as a red warning + stats readout
      (points per level) (M8)
- [ ] Annotate the C¹ bound `w < (√5−1)/8` in the four-point section
- [ ] ☑ **Commit + push**: "Add weight experimentation panel with divergence warning"

## Phase 5 — Interaction & visualization polish [REQUIRED]

- [ ] Vertex dragging with mouse in both viewports (M7),
      respecting `WantCaptureMouse`
- [ ] Preset buttons wired into the panel (M7)
- [ ] Step-by-step convergence view: ghosted intermediate levels toggle (M6)
- [ ] Display toggles (control polygon / handles / final-only)
- [ ] Color & stroke pass: readable palette, control polygon visually distinct
      from curve levels
- [ ] ☑ **Commit + push**: "Add vertex editing, presets, and convergence view"

## Phase 6 — Documentation & report [REQUIRED]

- [ ] README.md: description, screenshot, build (incl. apt prerequisites),
      run, full controls reference
- [ ] Run the weight experiments; capture screenshots into `docs/img/`
      (canonical, rough, fractal, divergent for each scheme)
- [ ] `docs/REPORT.md`: implementation walkthrough, both algorithms explained,
      design decisions, experiment results with screenshots, challenges
- [ ] Final code pass: consistent naming, remove dead code, header comments
- [ ] ☑ **Commit + push**: "Add README and course report with experiment results"

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
