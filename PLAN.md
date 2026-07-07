# PLAN — Technical Plan

Companion to [PRD.md](PRD.md). Describes *how* the visualizer is built.

---

## 1. Technology choices

| Concern | Choice | Why |
|---|---|---|
| Language / standard | C++17 | Course requirement; C++17 is universally supported by g++ 15 and keeps the code modern without exotic features |
| Build system | CMake ≥ 3.16 | De-facto standard for C++; single `CMakeLists.txt` is enough for this size |
| Window + 2D drawing | **SDL2** (system package) | Lightweight, stable, trivial line/point drawing via `SDL_RenderDrawLinesF`; works out of the box under WSLg |
| GUI controls | **Dear ImGui** (vendored in `external/imgui/`, SDL2 + SDLRenderer2 backend) | Sliders, checkboxes and buttons for the weight-experimentation UI with almost zero UI code; vendoring (~15 files, MIT license) pins the version and keeps the build offline-friendly |
| Tests | Plain `assert`-based test executable wired to CTest | The math core is small; a header-only framework would be over-engineering for a 1-day project |

Rendering rationale: the app draws *polylines and points* — nothing more. A
2D accelerated renderer (SDL_Renderer) is exactly the right tool; raw OpenGL
would add boilerplate (context, shaders) for zero visual benefit.

## 2. Repository layout

```
mini-project-subdivision/
├── CMakeLists.txt            # single build script (app + tests)
├── .gitignore                # build/, IDE files, binaries
├── .gitattributes            # enforce LF endings
├── README.md                 # build, run, controls
├── PRD.md  PLAN.md  TODO.md  # planning documents (this set)
├── docs/
│   ├── REPORT.md             # course-style report
│   └── img/                  # screenshots for README/REPORT
├── external/
│   └── imgui/                # vendored Dear ImGui (pinned release + license)
├── include/subdiv/           # public headers (math core, no SDL includes)
│   ├── vec2.hpp              # minimal 2D vector type
│   ├── polygon.hpp           # Polygon = points + closed flag, presets
│   ├── scheme.hpp            # SubdivisionScheme interface + refine-N helper
│   ├── chaikin.hpp           # ChaikinScheme (cut ratio t, optional t1/t2)
│   └── fourpoint.hpp         # FourPointScheme (tension w)
├── src/
│   ├── main.cpp              # entry point: init, main loop, shutdown (thin)
│   ├── app.cpp / app.hpp     # App: owns state, wires input→model→render
│   ├── ui.cpp / ui.hpp       # ImGui control panel (reads/writes AppState)
│   ├── viewport.cpp/.hpp     # world↔screen transform, per-viewport layout
│   └── canvas.cpp / canvas.hpp # polyline/handle drawing on SDL_Renderer
└── tests/
    └── test_subdivision.cpp  # hand-computed step verification per scheme
```

Design rule: **`include/subdiv/` is pure math** — no SDL, no ImGui. Only
`src/` touches the libraries. This keeps the core unit-testable and cleanly
separated, and would let the 3D stretch goal reuse the same pattern.

*As-built additions to this tree*: `src/input.{hpp,cpp}` (vertex dragging,
split out during Phase 5), `include/subdiv/vec3.hpp` +
`include/subdiv/trimesh.hpp` + `src/mesh_view.{hpp,cpp}` (3D bonus), and
`.github/workflows/ci.yml` (CI). The final layout is in REPORT.md §3.

## 3. Data structures

```cpp
struct Vec2 { float x, y; };                  // +, -, scalar *, lerp

struct Polygon {
    std::vector<Vec2> pts;
    bool closed = true;
};

// One subdivision run, all levels retained for the convergence view:
// levels[0] = control polygon, levels[k] = k-th refinement.
using Levels = std::vector<std::vector<Vec2>>;
```

Retaining every level is deliberate: the step-by-step convergence view (M6)
needs them, and memory is trivial — points double per level, so 8 iterations
of a 12-gon is ~3 000 points (~24 KB).

**Recompute-on-change**: whenever the control polygon or any parameter
changes, both schemes' `Levels` are recomputed from scratch. At these sizes a
full recompute is microseconds; caching logic would be over-engineering.

## 4. Core algorithms (design level)

### 4.1 Common interface

```cpp
class SubdivisionScheme {
public:
    virtual std::vector<Vec2> step(const std::vector<Vec2>& p, bool closed) const = 0;
};
Levels refine(const SubdivisionScheme&, const Polygon& p, int iterations);
```

`refine` applies `step` iteratively, records each level, and aborts early
with a `diverged` flag if any coordinate goes non-finite or exceeds a large
bound (protects the app when the user picks explosive weights — see §6).

### 4.2 Chaikin corner cutting (approximating)

For each edge (Pᵢ, Pᵢ₊₁) of a closed polygon, emit two points:

```
Q = (1−t)·Pᵢ + t·Pᵢ₊₁        (cut near Pᵢ)
R = t·Pᵢ + (1−t)·Pᵢ₊₁        (cut near Pᵢ₊₁)
```

Canonical `t = 1/4` gives the classic 1/4 : 3/4 rule; the limit curve is the
quadratic B-spline of the control polygon. The slider exposes `t ∈ [0.01, 0.99]`:
`t = 0.5` collapses each edge's two cuts to its midpoint (polygon degenerates),
`t > 0.5` makes the cuts *cross over*, producing self-intersecting, increasingly
wild polygons — an easy first "wrong weights" experiment.

*Nice-to-have (N3)*: independent `t₁`, `t₂` for the two cuts, breaking the
symmetry of the mask.

### 4.3 Four-point scheme (interpolating)

All old points are kept; between each consecutive pair a new point is
inserted using the four surrounding points (indices mod n for closed
polygons):

```
M = (1/2 + w)·(Pᵢ + Pᵢ₊₁) − w·(Pᵢ₋₁ + Pᵢ₊₂)
```

Canonical `w = 1/16` reproduces cubics and yields a C¹ limit curve. Because
old points are never moved, the limit curve **interpolates** the control
points — the visual contrast with Chaikin (M3). Known behavior to surface in
the UI/report: the scheme is C¹ for `0 < w < (√5−1)/8 ≈ 0.19`; at `w = 0` it
is trivial midpoint insertion (limit = the polygon itself); beyond the bound
the curve turns increasingly fractal; large `w` diverges.

### 4.4 Weight parameterization approach

Each scheme owns its natural parameters (`t` for Chaikin, `w` for four-point)
rather than exposing a raw mask table. Rationale: both masks are already
fully determined by one scalar *while preserving affine invariance* (weights
sum to 1 by construction), so a single slider per scheme spans the entire
interesting behavior range — smooth → rough → fractal → divergent — without
letting the user wander into a high-dimensional space where effects are hard
to attribute. The asymmetric-Chaikin option (N3) is the controlled way to add
one more degree of freedom.

## 5. Application architecture

```
main.cpp ── owns SDL window/renderer + ImGui context, runs the loop:
   1. poll SDL events  →  App::handleEvent()   (vertex dragging, quit)
   2. build ImGui UI   →  ui::drawPanel(state) (sliders/toggles mutate state)
   3. if state.dirty   →  recompute Levels for both schemes
   4. render           →  canvas::draw() per viewport, then ImGui
```

- `AppState` is a plain struct (polygon, scheme params, iteration count,
  display toggles, computed Levels + diverged flags). UI and input mutate it;
  a `dirty` flag triggers recompute. No observer machinery — one struct, one
  loop, appropriate for the project size.
- `Viewport` maps a fixed world rectangle to each half of the window and
  handles hit-testing for vertex handles (screen-space radius). Both
  viewports share the same world rect so the two schemes are visually
  comparable 1:1.

## 6. Risks / tricky parts

| Risk | Mitigation |
|---|---|
| **Divergent weights produce NaN/inf or gigantic coordinates**, freezing or crashing the renderer | `refine` clamps: stop iterating when any coordinate is non-finite or exceeds a bound; UI shows a "diverging" badge (M8). Point count is inherently capped by the iteration limit (8) |
| **Post-core additions destabilize working features** | All three 2026-07 additions (§7) are strictly additive: the plot only reads existing caches, open mode flips a flag the whole pipeline already honors, and asymmetric Chaikin defaults to linked sliders that reproduce the current behavior bit-for-bit |
| **Four-point needs ≥ 4 points** (and open-curve endpoints need special casing) | Enforce a minimum of 4 control points in the UI; ship closed-polygon mode first (must-have), open mode as N2 with endpoint duplication |
| **SDL2/ImGui under WSLg** (untested on this machine) | Verified WSLg display works (`DISPLAY=:0`); ImGui's SDL2+SDLRenderer2 backend is the most battle-tested pairing; pin a known-good ImGui release. Fallback: raylib swap would only touch `src/` |
| **Vertex dragging vs. ImGui input conflict** (clicks meant for the canvas hitting the panel and vice versa) | Standard guard: ignore canvas mouse events when `ImGui::GetIO().WantCaptureMouse` is true |
| **/mnt/c (Windows drive) filesystem is slow under WSL** | Project is tiny; build times are seconds. `build/` stays untracked, so no git overhead |
| **Time budget (~1 day)** | Must-haves front-loaded in TODO phases 0–5; every nice-to-have is genuinely skippable; the 3D stretch goal is last and fully optional |

## 7. Post-core additions (2026-07 scope decision)

Three additions after Phases 0–7, chosen for maximal reuse of what exists
(see PRD "consciously deferred" for what was set aside and why).

### 7.1 Live convergence plot (M9)

Lives entirely in `ui.cpp::schemeStats()`, which already receives the
`RefineResult` and `closed` flag. Per call, fill two small stack arrays
(≤ 9 levels) with `perimeter(levels[k], closed)` and a new
`maxEdgeLength(levels[k], closed)` from the math core, then render them with
`ImGui::PlotLines` (auto-scaled, ~40 px tall) under the existing stats text.
No AppState changes, no caching: recomputing both series per frame is a few
thousand float ops. `maxEdgeLength()` joins `scheme.hpp` beside
`perimeter()` — it is the sup-norm convergence indicator (must → 0 for a
convergent scheme) and gets the same unit-test treatment.

### 7.2 Open-polyline mode (N2)

No changes to canvas or viewport: `Polygon.closed` already flows through
`step()` (endpoint handling implemented and unit-tested in Phases 2–3),
`refine()`, `perimeter()` (skips the closing edge), and
`canvas::drawPolyline()` (doesn't append the first point). The work is one
"closed polygon" checkbox in the Control-polygon section (sets `dirty`),
one open preset — `presets::arc()`, a ~7-point circular arc — and a
`--preset arc` CLI value for reproducible figures. Presets carry their own
`closed` flag; the checkbox reflects and overrides it.

### 7.3 Asymmetric Chaikin (N3)

`ChaikinScheme` replaces `t` with `t1`, `t2`; `step()` emits
`lerp(A,B,t1)` and `lerp(A,B,1−t2)` — cuts measured from each edge end —
so `t1 == t2 == t` reproduces the current scheme exactly (the single-float
constructor keeps existing call sites and tests valid). The UI gains a
"link cuts (symmetric)" checkbox, default **on**: linked shows today's
single slider and mirrors it into both parameters; unlinked shows two
sliders. Reset restores 0.25/0.25 and re-links. Tests: a hand-computed
asymmetric step plus a linked-equals-symmetric equivalence check.
