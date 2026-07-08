# RayCaster2 — "Sightline" · Complete Blueprint

A first-person **stealth/evasion** raycaster. You infiltrate a procedurally
generated facility with no weapon. The building's entire security system —
guard vision, patrol routing, and alarm spread — runs on the **room-adjacency
graph** the engine extracts from the map. You win by reading the building's
*structure*: staying out of sightlines, cutting the graph by closing doors, and
routing yourself through rooms the guards can't reach in time.

> **Working title only.** "Sightline" is a placeholder — rename freely. The
> architecture is concept-agnostic (see §12): swap the objective layer and the
> same core becomes a containment game, a cartography game, etc.

---

## 1. Why this is a *completely different* project from RayCaster1

RayCaster1 is a Wolfenstein-style **renderer**: textured walls, billboard
sprites, health pickups, and enemies that animate but have *no AI*. Its
flood-fill exists only to answer a yes/no "does the map leak?" validation.

RayCaster2 inverts the emphasis:

| Axis | RayCaster1 | RayCaster2 (Sightline) |
|---|---|---|
| Genre | run-and-gun shooter | stealth / evasion (no shooting) |
| Center of gravity | pixel rendering, textures | **systems: graph + AI + simulation** |
| Enemies | static sprites, no AI | guards with vision + graph pathfinding |
| Flood-fill | boolean map validation | **region labeling → areas + adjacency** |
| Raycasting used for | drawing walls | drawing walls **+ guard sightlines** |
| Win condition | reach exit / survive | evade detection, exploit building structure |
| Deliberately shared | — | only `Vector2` + the DDA math (to *show* reuse) |

Everything below the "shared" line is written fresh. The two repos stand alone.

---

## 2. The three pillars (and their double payoff)

Every core system is chosen so that making the game fun also produces a
component the laser-wall RL floor-plan project needs.

### Pillar A — Regions
The map is flood-filled into labeled **rooms**, each with an `area`, `centroid`,
bounding box, and a **room type** (Hall, Server, Vault, Office…).
- **Game:** area sets how many guards a room holds; type sets objectives/loot.
- **RL:** `area` → area-target reward; `type` → the room program to satisfy.

### Pillar B — Adjacency & doors
Rooms connect through **doors = typed graph edges**. An alarm state propagates
across edges each tick. Closing a door **removes an edge**, halting the spread
and forcing guards to reroute.
- **Game:** contain the alarm, reroute patrols, plan a route the graph allows.
- **RL:** adjacency graph → graph-match reward vs the bubble diagram; door edges
  → the typed edges (door / arch / shared-wall) ResPlan encodes.

### Pillar C — Sightlines (beams)
Guards and cameras cast **vision rays via DDA** that stop at walls and closed
doors (occlusion). A ray that reaches the player raises detection.
- **Game:** stealth is the art of staying out of every beam; corners matter.
- **RL:** this *is* `castBeam` — the exact beam propagation + occlusion the
  laser-wall representation is built on.

---

## 3. Architecture

Hard split between an **SDL-free `core/`** (pure logic, unit-tested, eventually
shared with the Python RL env) and an **SDL `game/`** layer. The core never
includes SDL, so it compiles and tests with nothing but `g++`.

```
RayCaster2/
├── BLUEPRINT.md
├── CMakeLists.txt
├── third_party/
│   └── SDL2/                 # VENDORED (fixes the temp-dir problem, see §8)
├── include/
│   ├── core/                 # ── no SDL, portable, RL-shared ──
│   │   ├── Vector2.hpp        # ported verbatim from RayCaster1
│   │   ├── Grid.hpp           # the map == the RL plot
│   │   ├── RoomGraph.hpp      # regions, area, adjacency  ← THE FOCUS
│   │   ├── Beam.hpp           # DDA castBeam + occlusion   ← laser-wall seed
│   │   ├── Visibility.hpp     # vision cones from beams
│   │   ├── AlarmSim.hpp       # alert propagation over adjacency
│   │   ├── Patrol.hpp         # guard routing on the graph (BFS/Dijkstra)
│   │   └── LevelGen.hpp       # procedural gen == RL plot sampler
│   └── game/                  # ── SDL ──
│       ├── Renderer.hpp        # first-person view + top-down graph HUD
│       ├── Player.hpp
│       ├── Guard.hpp
│       └── Game.hpp
├── src/core/*.cpp
├── src/game/*.cpp
├── tests/                     # one dependency-free harness per core module
│   ├── test_roomgraph.cpp
│   ├── test_beam.cpp
│   ├── test_alarmsim.cpp
│   ├── test_patrol.cpp
│   └── test_levelgen.cpp
├── tools/
│   └── dump_graph.cpp         # headless: Grid → PPM/JSON (the RL debug viewer)
└── levels/
    └── demo1.txt
```

---

## 4. Core systems in detail (the RoomGraph-focused meat)

### 4.1 Grid & map format
`cells[y][x]`, legend `#` wall · `.` floor · `D` door · digits `0-9` optional
room-type seeds · `@` player start · `G` guard start · `C` camera. Maps come
from `levels/*.txt` **or** from `LevelGen`. Out-of-bounds reads as wall.

### 4.2 RoomGraph  *(already prototyped & green — moves in as M0)*
- **Algorithm:** 4-connected flood-fill labeling of interior cells; walls **and
  doors** are barriers, so each enclosed area becomes one room. O(W·H).
- **Per room:** id, `area` (cell count), `centroid`, bounding box, `type`.
- **Adjacency:** each door cell links the distinct rooms it touches → one typed
  `RoomEdge`. Dedup unordered pairs.
- **Extension (M-later):** scan shared wall segments to also emit `SharedWall`
  edges, matching ResPlan's three edge types.
- **Test:** two-rooms-one-door fixture asserts room count, areas, edge, labels
  (the 9 assertions already passing in the spike).

### 4.3 Beam / Visibility  *(the laser-wall seed)*
```cpp
struct BeamHit { double dist; int cellX, cellY; bool hitDoor; };
// March a ray from `origin` along `dir` until `stop(cell)` is true.
BeamHit castBeam(const Grid&, Vector2 origin, Vector2 dir,
                 const std::function<bool(int,int)>& stop);
```
`Visibility::cone(origin, facing, fovRad, nRays)` fans beams across the FOV and
returns the set of visible cells. `stop` closes over the live door states, so a
closed door blocks sight. Detection = accumulated exposure while any guard's
cone contains the player. **This is the identical primitive laser-wall uses**;
only the stop condition changes (wall/door → wall/beam/boundary).
- **Test:** ray down a corridor stops at the wall at the expected distance; a
  closed door blocks, an open door passes.

### 4.4 AlarmSim  *(pure graph propagation)*
Per-room state `Calm → Suspicious → Alert`. Each tick, an `Alert` room raises
its **open-edge** neighbours to at least `Suspicious`; states decay over time if
no stimulus. Deterministic (seeded), no SDL — fully headless-testable.
- **Test:** trip room A, assert the alert reaches D through open doors in N ticks
  and is *blocked* when the connecting door is closed.

### 4.5 Patrol  *(pathfinding on the graph)*
Guards path room-to-room via BFS/Dijkstra on the adjacency graph; `area` scales
dwell time (big rooms take longer to sweep). On alert, a guard routes toward the
player's last-known room by graph shortest path. Within a room, simple
grid A* to a waypoint.
- **Test:** shortest room-path around a closed door matches the hand-computed
  route; closing a door changes it.

### 4.6 LevelGen  *(== the RL plot sampler)*
BSP partition **or** random room placement + corridor carving. Guarantees a
**connected** adjacency graph and a target room count `n`. Emits a `Grid` plus
the ground-truth `RoomGraph`.
- **Game:** endless facilities; difficulty = room count + guard budget.
- **RL:** the same generator samples training plots; `n` drives curriculum.
- **Test:** generated level parses, is fully connected, has exactly `n` rooms.

---

## 5. Gameplay

- **Loop:** spawn → observe patrols & sightlines → move through rooms/doors to
  reach objective(s) → reach exit undetected. Getting seen fills a detection
  meter; full meter = caught = fail.
- **Player verbs:** move (WASD), turn, **open/close door** (E — edits the graph),
  peek. No weapon.
- **States:** Infiltrating · Spotted (meter rising) · Alarm (guards converge) ·
  Escaped / Caught.
- **HUD:** first-person view **plus a live top-down adjacency graph** — rooms
  colored by alert state, edges shown, guard positions ticking along paths. That
  graph HUD is literally the RL debug view, built for free.

---

## 6. Rendering plan
Software framebuffer (as RayCaster1). First-person walls **shaded by room type +
a simple light falloff** — no texture atlas (deliberately unlike RC1). Guard
vision cones and alarm heat render on the **top-down graph panel**. Minimal art,
maximal readability of *systems*.

---

## 7. Tech stack
C++17 · SDL2 (vendored) · CMake ≥ 3.16 · MinGW-w64 g++ 14.1. Core builds with
plain `g++ -std=c++17 -Wall -Wextra -Wpedantic`; only the game layer needs SDL.

## 8. Build & the SDL fix
The machine's only SDL2 lives in a **temp dir** that can be wiped. So RayCaster2
**vendors SDL2** via CMake `FetchContent` (download+build pinned 2.30.x) with a
fallback to `third_party/SDL2/`. The core library and all tests link **nothing**,
so the test loop is immune to SDL breakage.

---

## 9. Milestones (headless-first; each ends green & runnable)
- **M0 — Core seed:** `Grid` + `RoomGraph` + tests. *(done in spike; relocate.)*
- **M1 — Beams:** `Beam`/`Visibility` + tests + `dump_graph` PPM tool (see the
  segmentation & sightlines as an image, still no SDL).
- **M2 — Simulation:** `AlarmSim` + `Patrol` + tests (run a sim on a fixed level,
  print room states per tick).
- **M3 — Generation:** `LevelGen` + tests (assert connected, n rooms).
- **M4 — Window:** SDL render layer — first-person + top-down graph HUD.
- **M5 — Game:** player, guards, detection meter, win/lose glue.
- **M6 — Polish:** README, GIFs, tuning.

**One-week cut:** M0–M2 + a minimal M4 view = a demoable stealth prototype whose
core is *100% the RL components*. M3/M5/M6 are stretch.

## 10. Testing
Per-module dependency-free assert harness (RayCaster1 style). **Seeded RNG
everywhere** so sims and generation are reproducible and testable.

---

## 11. Component map — Sightline → RoomGraph → FloorPlan RL

| Sightline system | RoomGraph piece it exercises | FloorPlan RL use |
|---|---|---|
| Room objectives / guard budget | region labeling + `area` | area-target reward |
| Alarm propagation | adjacency graph traversal | graph-match reward |
| Close-door-to-contain | edit graph edges | typed edges (door/arch/wall) |
| Guard/camera vision | `castBeam` + occlusion | laser-wall beam propagation |
| Patrol routing | shortest path on graph | validity / reachability checks |
| Fog reveal per room | `roomAt(x,y)` lookup | per-region state readout |
| Facility generator | grid + ground-truth graph | RL plot sampler + curriculum |
| Top-down graph HUD | full RoomGraph render | RL debug viewer |

## 12. Concept is swappable
The objective layer is thin. Keep the whole `core/` and reskin:
- **Containment:** the alarm becomes fire/flood spreading the graph; you close
  doors to survive. (Uses adjacency + area even harder.)
- **Cartographer:** no guards; reconstruct the hidden RoomGraph by exploring
  (fog reveal per room). (Purest RoomGraph focus.)
Only `AlarmSim`/objective code changes; beams, regions, generation, rendering all
stay. That modularity is itself a point in the project's favor.

## 13. What ships straight into the FloorPlan RL project
`Grid`, `RoomGraph`, `Beam::castBeam`, `LevelGen` (plot sampler), and
`dump_graph` (debug viewer) — lifted directly, either via pybind11 or a NumPy
re-implementation (decided at RL stage). The stealth game is the *proving ground*
that makes each of these correct and visible before any learning is involved.
```
