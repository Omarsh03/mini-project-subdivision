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
- [x] S1: 3D bonus — Loop subdivision on a cube, wireframe render
      (separate viewport or mode; no extraordinary-vertex correctness needed)
      — ☑ commit + push "Add Loop subdivision 3D bonus"
- [x] Update README/REPORT for whichever extras landed
      — ☑ commit + push "Document optional features"

## Phase 8 — Post-core additions (2026-07 scope decision) [REQUIRED]

Priority order fixed by the scope decision; design notes in PLAN.md §7.
N1, N5, CI, limit-curve overlay and shaded 3D stay deferred (PRD).

### 8.1 Live convergence plot (M9)

- [ ] `maxEdgeLength()` in `scheme.hpp` (sup-norm convergence indicator)
      + unit tests alongside `perimeter()`'s
- [ ] Per-scheme `ImGui::PlotLines` of perimeter and max edge length per
      level in `ui.cpp::schemeStats()` (no AppState changes)
- [ ] Manual check: canonical weights → flat/decaying plots; fractal
      weights → growing perimeter, non-vanishing max edge
- [ ] ☑ **Commit + push**: "Add live convergence plots to the panel"

### 8.2 Open-polyline mode (N2)

- [ ] "closed polygon" checkbox in the Control-polygon section (sets dirty)
- [ ] `presets::arc()` open preset + preset button + `--preset arc` CLI value
- [ ] Unit test for the arc preset; manual check that both schemes keep
      endpoints (math already unit-tested since Phases 2–3)
- [ ] ☑ **Commit + push**: "Add open-polyline mode with arc preset"

### 8.3 Asymmetric Chaikin (N3)

- [ ] `ChaikinScheme`: `t1`/`t2` members, cuts `lerp(A,B,t1)` and
      `lerp(A,B,1−t2)`; single-float constructor keeps call sites valid
- [ ] "link cuts (symmetric)" checkbox (default on = current behavior);
      unlinked shows two sliders; reset restores 0.25/0.25 and re-links
- [ ] `--chaikin-t2` CLI flag for reproducible figures
- [ ] Unit tests: hand-computed asymmetric step; linked == symmetric
- [ ] ☑ **Commit + push**: "Add asymmetric Chaikin cuts"

### 8.4 Documentation

- [ ] README controls table: plots, closed-polygon toggle, Arc preset,
      link-cuts toggle, new CLI flags
- [ ] REPORT: convergence plot as the live form of §5; open-polyline and
      asymmetric-Chaikin observations with figures
- [ ] ☑ **Commit + push**: "Document convergence plots, open polylines,
      asymmetric Chaikin"
