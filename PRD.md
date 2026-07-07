# PRD — Interactive 2D Subdivision Visualizer

**Course mini-project (Computer Graphics), solo work.**
Author: Omar El-Sheikh · Repo: <https://github.com/Omarsh03/mini-project-subdivision>

---

## 1. Problem statement / motivation

Subdivision schemes are a cornerstone of modern geometric modeling: starting
from a coarse control polygon, a simple refinement rule is applied repeatedly,
and the polygon converges to a smooth *limit curve*. The same idea, extended to
meshes, powers subdivision surfaces used throughout film and CAD (Catmull–Clark,
Loop).

Two properties make subdivision an ideal topic for an interactive visualizer:

1. **The rules are tiny, the behavior is rich.** Chaikin's corner cutting is
   two affine combinations per edge, yet it produces a quadratic B-spline.
   The four-point scheme is one four-term stencil, yet it interpolates its
   input and is C¹ — but only for a narrow range of its tension weight.
2. **Correctness is fragile in an instructive way.** Nudging the weights away
   from their canonical values makes the limit curve rough, then fractal, then
   divergent. Watching *when and how* smoothness breaks teaches more about why
   the canonical weights are what they are than any static derivation.

This project builds a C++ desktop app in which the user edits a control
polygon and watches, live and side-by-side, how an **approximating** scheme
(Chaikin) and an **interpolating** scheme (four-point) refine it — with full,
free control over the subdivision weights and the number of iterations.

## 2. Feature list

### MUST-HAVE

| # | Feature |
|---|---------|
| M1 | **Chaikin corner cutting** with adjustable cut ratio `t` (default 1/4 : 3/4), for closed polygons |
| M2 | **Four-point interpolating scheme** with adjustable tension weight `w` (default 1/16), for closed polygons |
| M3 | **Side-by-side view**: both schemes applied to the *same* control polygon in two adjacent viewports, to contrast approximating vs. interpolating behavior |
| M4 | **Free weight control**: sliders allow pushing `t` and `w` well outside their "correct" ranges (including values where the scheme stops converging smoothly), with the canonical value marked and a one-click reset |
| M5 | **Adjustable iteration count** (0–8) with immediate visual update |
| M6 | **Step-by-step convergence view**: intermediate subdivision levels can be shown simultaneously (ghosted/faded), so the polygon can be seen converging toward the limit curve |
| M7 | **Interactive control polygon**: drag vertices with the mouse; shape presets (square, star, random, zig-zag) to restart from |
| M8 | **Live stats**: point count per level, and a simple divergence warning when the curve blows up (NaN/huge coordinates) |

### NICE-TO-HAVE

| # | Feature |
|---|---------|
| N1 | Add / delete control vertices with the mouse (click on edge to insert, right-click to remove) |
| N2 | Open (non-closed) polyline mode with proper endpoint handling |
| N3 | Asymmetric Chaikin: two independent cut parameters `t₁`, `t₂` instead of the symmetric pair `t : 1−t` (richer breakage experiments) |
| N4 | Pan / zoom of the canvas |
| N5 | Overlay mode: both schemes drawn in one viewport for direct comparison |

### OUT-OF-SCOPE / STRETCH

| # | Feature |
|---|---------|
| S1 | **3D bonus**: Loop subdivision of a cube rendered as a wireframe/flat-shaded mesh — *only if the 2D core is finished and polished*. No correct treatment of extraordinary vertices required |
| — | Subdivision surfaces in general (Catmull–Clark etc.), texture/shading work, file import/export, animation export, touch/gesture input |

## 3. User-facing behavior

Running `./subdivision_visualizer` opens a single desktop window (via WSLg):

- **Canvas (main area), split into two viewports**: left = Chaikin,
  right = four-point. Both show the same control polygon (drawn as a thin
  gray polygon with draggable vertex handles) and the subdivided curve on
  top (colored, thicker). With the convergence view enabled, earlier
  subdivision levels appear as progressively fainter curves between the
  control polygon and the final level.
- **Control panel (Dear ImGui sidebar)** with:
  - Iterations slider (0–8), applying to both viewports.
  - *Chaikin* section: cut-ratio slider `t ∈ [0.01, 0.99]` with the
    canonical `t = 0.25` marked; reset button.
  - *Four-point* section: tension slider `w ∈ [−0.25, 0.40]` with the
    canonical `w = 1/16 = 0.0625` marked and the known C¹ bound annotated;
    reset button.
  - Display toggles: show control polygon, show vertex handles, show
    intermediate levels, show final curve only.
  - Preset buttons: Square, Star, Random, Zig-zag.
  - Stats readout: number of points at the displayed level per scheme;
    red "diverging" warning when coordinates explode.
- **Mouse**: drag a vertex handle in either viewport to move it — both
  viewports update instantly since they share the control polygon.

## 4. Success criteria ("done" looks like)

1. **Builds & runs** on Ubuntu/WSL with two commands
   (`cmake -B build && cmake --build build`), documented in README.md.
2. **Correct defaults**: at canonical weights, Chaikin visibly converges to a
   smooth curve that *approximates* (pulls inside) the control polygon, and
   four-point produces a smooth curve that *passes through* every control
   point. Unit tests pin down one hand-computed subdivision step per scheme.
3. **The experiment works**: moving the weight sliders demonstrably produces
   the rough → fractal → divergent progression, and the REPORT documents this
   with screenshots and short explanations.
4. **All MUST-HAVE features** implemented; app stays responsive (60 fps-ish)
   at 8 iterations on a typical control polygon.
5. **Deliverables complete**: README.md (build/run/controls), docs/REPORT.md
   (algorithms, design decisions, experiment results, challenges), clean
   multi-file source layout, meaningful git history on
   `mini-project/subdivision`, pushed to GitHub.
