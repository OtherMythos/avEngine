# Agent debug server

A **localhost REST API** for inspecting and driving a running avEngine instance. It lets
an AI agent (or a developer with `curl`) query the live Ogre scene graph, capture the
rendered frame, and execute Squirrel snippets — useful for answering questions like
*"why is my mesh not visible?"* without attaching a debugger.

The inspection endpoints (`GET`) never mutate engine state; the `POST` endpoints —
eval, input spoofing, simulation control, video capture and the profiler controls — can. This is a trusted local development tool: it binds
to `127.0.0.1` only, and is compiled in behind the `DEBUG_SERVER` build option (on by
default for desktop builds, excluded from iOS/Android).

## Enabling it

Launch the engine with the `--debugServer` flag, giving the path to a project's setup
file as usual:

```sh
av /path/to/project/avSetup.cfg --debugServer          # default port 8788
av /path/to/project/avSetup.cfg --debugServer 9000     # custom port
```

The flag takes an optional port. Because of how the argument parser works, put
`--debugServer` **after** the positional `avSetup.cfg` path.

Confirm it started — the log prints:

```
Debug server listening on 127.0.0.1:8788
```

and:

```sh
curl -s localhost:8788/api | jq
```

returns the endpoint catalog. The port is acquired exclusively at the OS socket level. If
another process is already using it, the engine logs an error, shuts down, and returns a
non-zero exit status. This makes the debug-server port a process-scoped lock when several
agents launch engines at the same time; the OS releases it when the owning process exits,
and the next engine can bind it straight away.

For any automated or agent-driven session, add `--headless` so no window appears; see
[headless.md](headless.md).

## Endpoints

All endpoints return `application/json` and are served under `/api`. `GET` endpoints are
read-only; `POST` endpoints mutate engine state. The full list, one line per endpoint, is in
[AGENTS.md](../AGENTS.md#agent-debug-server).

This page covers the status, scene, render and eval endpoints. The rest are documented with
their subject: input and GUI in [debug-server-input-and-gui.md](debug-server-input-and-gui.md),
audio in [audio.md](audio.md#audio-inspection), video in [video-capture.md](video-capture.md),
simulation in [simulation-control.md](simulation-control.md) and the profiler in
[script-profiler.md](script-profiler.md).

### Node identifiers

Ogre-Next scene nodes have **no names** — each is addressed by its numeric **id**
(`Ogre::Node::getId()`), which is stable for the life of the node. You obtain ids from
`/api/scene` and use them in `root=` and `/api/scene/node/<id>`.

### `GET /api/status`

```jsonc
{
  "engineVersion": "0.2.0 unstable",
  "gitHash": "6946087",
  "apiVersion": 5,
  "uptimeSeconds": 42.7,
  "project": "MyGame",
  "renderSystem": "Metal",
  "fps": 59.9,
  "avgFps": 60.1,
  "frameTimeMs": 16.6,
  "window": { "width": 1280, "height": 720, "title": "MyGame" }
}
```

### `GET /api/scene`

Dumps the scene graph from both the dynamic and static scene roots. Bounded by:

- `root` — a node id to root the dump at. Omit to dump from both scene roots.
- `depth` — how many levels of children to descend (default `3`).
- `max` — cap on total nodes serialised (default `500`), to protect an agent's context budget.

```jsonc
{
  "truncated": false,
  "nodes": [
    {
      "id": 1,
      "memoryType": "dynamic",         // only present on the two scene-root nodes
      "pos":        [0, 0, 0],         // local position [x, y, z]
      "derivedPos": [0, 0, 0],         // world-space position
      "scale":      [1, 1, 1],
      "orient":     [1, 0, 0, 0],      // quaternion [w, x, y, z]
      "objects": [
        {
          "name": "player.mesh",
          "type": "Item",              // Ogre movable type: Item, Light, Camera, ...
          "visible": true,
          "castShadows": true,
          "worldAabb": { "centre": [0, 1, 0], "halfSize": [0.5, 0.5, 0.5] }
        }
      ],
      "children": [ /* nested nodes, same shape */ ],
      "childCount": 14                 // present ONLY when truncated: real child count > children returned
    }
  ]
}
```

When a node is truncated (by `depth` or `max`), its `childCount` is emitted **instead of /
alongside** the returned `children`, and the top-level `"truncated"` is `true`. Drill into a
truncated branch with `?root=<id>` rather than raising `max`.

### `GET /api/scene/node/<id>`

```jsonc
{
  "id": 37,
  "pos":        [1, 0, 2],
  "derivedPos": [1, 0, 2],
  "scale":      [1, 1, 1],
  "orient":     [1, 0, 0, 0],
  "parentChain": [12, 1],              // parent ids, nearest first, up to the scene root
  "objects": [
    {
      "name": "player.mesh", "type": "Item", "visible": true, "castShadows": true,
      "worldAabb": { "centre": [1, 1, 2], "halfSize": [0.5, 0.5, 0.5] },
      "localAabb": { "centre": [0, 1, 0], "halfSize": [0.5, 0.5, 0.5] }
    }
  ],
  "childCount": 0
}
```

### `GET /api/render/frame`

Captures the frame that was just rendered and returns it in one of four **text forms**,
smallest first. The capture is synchronous GPU readback — expect a small frame-time
blip on the frame that services it.

- `form=stats` *(default, ~300 bytes)* — colour/luminance summary:
  ```jsonc
  {
    "frame": 8841, "captureWidth": 1280, "captureHeight": 720,
    "meanRgb": [104, 128, 190],
    "luminance": { "mean": 0.47, "min": 0.02, "max": 0.98,
                   "histogram": [2.1, 5.0, 9.2, ...] },      // 10 deciles, % of pixels
    "dominantColours": [ { "rgb": [92, 140, 220], "pct": 41.2 }, ... ]  // top 5
  }
  ```
- `form=grid` *(~1–4 KB)* — the workhorse. The frame downsampled to `w`×`h` cells
  (default 32×18, caps 96×54), each cell's average colour as 6 hex chars, one string
  per row:
  ```jsonc
  { "frame": 8841, "cellsX": 32, "cellsY": 18, "rows": ["5c8cdc5f8ed9...", ...] }
  ```
  Spatial layout survives: "the bottom third is brown, there's a red blob left of
  centre" is directly readable, and two grids diff cleanly.
- `form=ascii` *(~700 bytes)* — luminance only, one char per cell from the ramp
  `" .:-=+*#%@"` (dark→bright). The quick "did anything render at all?" check.
- `form=png` — base64 PNG (`{"png_base64": ..., "width": ..., "height": ...}`), resized
  so no dimension exceeds `maxDim` (default 512, cap 1024). Highest fidelity; use it if
  you can consume images, otherwise prefer the text forms.

Shared parameter: `region=x,y,w,h` (normalised 0–1 floats) crops before downsampling —
zoom into a quadrant at the same payload cost, e.g.
`/api/render/frame?form=grid&region=0.5,0,0.5,0.5` for the top-right quarter.

### `GET /api/render/hash`

A 64-bit perceptual hash (dHash) of the live frame — a few dozen bytes, so it's the cheap
way to *poll*. Use it to wait for the screen to settle instead of sleeping and hoping:
poll until the hash stops changing, then proceed.

```sh
curl -s localhost:8788/api/render/hash
# {"frame":251,"dhash":"0000000000000001"}
```

The hash encodes structure, not exact colour, so it is stable under small rendering
noise. Compare two hashes with the hamming distance reported by `/api/render/compare`
(0 identical, >10 clearly different).

### `POST /api/render/snapshot?name=<slot>`

Captures the current frame and stores it under a name, so you can compare against it
later without ever holding pixels yourself. Frames are stored downsampled (analysis
resolution, ≤320×180), and slots are capped at 16 — saving past that evicts the oldest.
This mutates only the debug server's own store, never engine state.

```sh
curl -s -X POST "localhost:8788/api/render/snapshot?name=before"
# {"name":"before","frame":558,"width":240,"height":180,"dhash":"0000000000000001"}

curl -s localhost:8788/api/render/snapshots        # {"snapshots":["before"]}
```

### `GET /api/render/compare?a=<slot>&b=<slot|live>`

Diffs two frames and tells you **how much** changed and **where**. `a` is a snapshot
name; `b` is another snapshot or `live` (default) to capture fresh.

```jsonc
{
  "hammingDistance": 0,          // dHash distance between the two frames
  "dhashA": "...", "dhashB": "...",
  "gridW": 32, "gridH": 18,
  "changedFraction": 0.004,      // fraction of grid cells over the threshold
  "meanDelta": 0.0002,           // mean per-cell difference across the whole frame, 0-1
  "changedCells": [ { "cell": [3, 4], "delta": 0.078 } ],   // biggest first, capped at 24
  "totalChangedCells": 9,
  "changedRegion": { "x": 0.0, "y": 0.056, "w": 0.083, "h": 0.056 }  // normalised bbox
}
```

Tuning — `w`/`h` set the grid (default 32×18, cap 96×54) and `threshold` the per-cell
delta that counts as changed (default `0.02`). Identical frames diff to exactly `0`, so
the risk is missing subtle changes rather than false positives: **for a small UI change
use a finer grid and a lower threshold**, e.g.
`?a=before&b=live&w=96&h=54&threshold=0.005`. `meanDelta` is threshold-independent, so a
non-zero `meanDelta` with an empty `changedCells` means "something changed, but below
your threshold".

### `GET /api/render/find?rgb=<hex>&tolerance=<n>`

Finds connected regions of a colour and returns their normalised centroids and bounding
boxes — useful for locating markers, or for turning something you spotted in a render
capture into coordinates you can click.

```sh
curl -s "localhost:8788/api/render/find?rgb=c0e8f6&tolerance=15"
```
```jsonc
{ "frame": 8511, "rgb": [192, 232, 246], "tolerance": 15,
  "matches": [ { "centre": [0.549, 0.541], "bbox": [0.0, 0.0, 1.0, 1.0], "pct": 86.98 } ] }
```

`tolerance` is a per-channel (0–255) allowance, default 40. Matches are largest first,
capped at 10, and tiny specks are filtered out. `region=x,y,w,h` restricts the search —
note the returned coordinates are then relative to that crop. Colours are
**display-referred sRGB**, i.e. what you would sample from a screenshot: read them off
`/api/render/frame?form=stats` (`dominantColours`) or `form=grid` rather than guessing.

### `POST /api/eval`

Compile and run a Squirrel snippet on the engine's main thread, with the full engine
script API (`_scene`, `_entity`, `_window`, `_gui`, ... — the same globals
project scripts see).

```sh
curl -s -X POST localhost:8788/api/eval -d '{"code": "1 + 1"}'
# {"result":2,"ok":true,"prints":[]}

curl -s -X POST localhost:8788/api/eval \
  -d '{"code": "local w = _window.getSize(); return {w = w.x, h = w.y};"}'
# {"result":{"w":1600,"h":1199},"ok":true,"prints":[]}
```

Semantics:

- **Bare expressions return their value** (`"1 + 1"` → `2`). Multi-statement snippets
  need an explicit `return`; without one the result is `null`.
- The optional `"timeoutMs"` field (default 2000, max 60000) bounds how long the HTTP
  request waits for the main thread.
- Response: `{"ok": true, "result": ..., "prints": [...]}` or
  `{"ok": false, "error": "compile error: ..." | "runtime error: ...", "prints": [...]}`.
  `print()` output is captured into `prints` (and still written to the engine log).
- Tables/arrays serialise recursively into JSON (depth-capped). Userdata, instances and
  closures serialise as `{"type": "...", "tostring": "..."}`.
- Snippets run in the root table: assignments to globals persist between evals
  (`::myDebugVar <- 5`), which is useful for multi-step investigations.
- **Do not eval unbounded loops.** Squirrel cannot be pre-empted: an infinite loop hangs
  the engine's main thread permanently. The HTTP request will 503, but the engine will
  not recover.

## Recipes

**"Is the engine alive and what is it running?"**
```sh
curl -s localhost:8788/api/status | jq
```

**"Why is my mesh not visible?"**
1. Dump the scene shallowly and locate the node whose `objects[].name` is your mesh:
   ```sh
   curl -s "localhost:8788/api/scene?depth=2" | jq
   ```
2. If it's inside a truncated branch, drill in with its parent's id:
   ```sh
   curl -s "localhost:8788/api/scene?root=<id>&depth=3" | jq
   ```
3. Inspect the node directly:
   ```sh
   curl -s localhost:8788/api/scene/node/<id> | jq
   ```
   Check the object's `visible`, the node's `derivedPos` (is it where you expect, or off at
   the origin / far away?), `scale` (accidentally zero?), and `worldAabb` (is it behind the
   camera or degenerate?).

**Navigating a large scene without blowing your context window:** start at `depth=2`, then
follow `childCount` and `root=<id>` to drill down. Prefer this over cranking `max` up, which
returns the whole tree at once.

**"What is actually on screen right now?"**
```sh
curl -s "localhost:8788/api/render/frame?form=ascii" | jq -r '.rows[]'   # silhouette check
curl -s "localhost:8788/api/render/frame?form=stats" | jq               # black screen? sky only?
curl -s "localhost:8788/api/render/frame?form=grid&w=48&h=27" | jq      # colour layout
```
A screen that renders nothing shows `lumMin == lumMax` and one dominant colour at ~100%.
Combine with the scene recipes: if `/api/scene` says the mesh exists and is visible but
the render grid shows nothing where `derivedPos` projects to, suspect camera transform or
material rather than scene structure.

**"Did my action actually do anything, and where?"** — the verification loop. Snapshot,
act, compare; you never handle a pixel:
```sh
curl -s -X POST "localhost:8788/api/render/snapshot?name=before"
# ... inject input, eval something, whatever you are testing ...
curl -s "localhost:8788/api/render/compare?a=before&b=live" | jq
```
`changedFraction` says how much moved and `changedRegion` says **where** (normalised
bbox) — so you can tell "the menu opened in the top-left" from "the whole scene
re-rendered". If you expect a small or subtle change (a button highlight, one label),
re-run with `&w=96&h=54&threshold=0.005`; a non-zero `meanDelta` with no `changedCells`
means something changed below your threshold.

**"Wait until the screen settles"** (after a load or transition) — poll the hash instead
of sleeping blindly:
```sh
curl -s localhost:8788/api/render/hash    # repeat until dhash stops changing
```

**"Read the on-screen text, then click a button":**
```sh
curl -s localhost:8788/api/gui/labels | jq          # every visible label + its center
```
Find the button you want by its `text`, then drive the mouse to its `center` (GUI, render
and input all share the 0–1 coordinate space):
```sh
curl -s -X POST localhost:8788/api/input/mouse -d '{"moveTo":[0.042,0.075]}'
curl -s -X POST localhost:8788/api/input/mouse -d '{"button":0,"pressed":true,"frames":2}'
```
`/api/gui/tree` gives the full structure when `labels` isn't enough, and
`/api/gui/at?x=&y=` tells you what sits under a point you spotted in a render capture; all
three are in [debug-server-input-and-gui.md](debug-server-input-and-gui.md).

## Semantics

- **Error shape:** `{ "error": "..." }` with an HTTP status:
  - `404` — the requested node id doesn't exist.
  - `503` — the engine didn't service the request in time (paused in a debugger, hung, or
    shutting down). Requests are executed on the engine's main thread once per rendered
    frame, so a stalled engine will time out after ~2s.
  - `400` — bad parameters.
- Each response is a **snapshot of a single frame**.
- All `GET` endpoints are read-only; the `POST` endpoints (`/api/eval`, `/api/input/*`)
  mutate. Check `apiVersion` in `/api` to detect capability changes.
