# avEngine Agent Debug Server

A **localhost REST API** for inspecting and driving a running avEngine instance. It lets
an AI agent (or a developer with `curl`) query the live Ogre scene graph, capture the
rendered frame, and execute Squirrel snippets — useful for answering questions like
*"why is my mesh not visible?"* without attaching a debugger.

The inspection endpoints (`GET`) never mutate engine state; the `POST` endpoints —
`/api/eval` and input spoofing — can. This is a trusted local development tool: it binds
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

returns the endpoint catalog. If the port is already in use, the engine logs an error and
keeps running without the server.

## Endpoints

All endpoints return `application/json` and are served under `/api`. Most are `GET`
(read-only); the `POST` endpoints mutate engine state and are marked **Mutating** below.

| Endpoint | Description |
|---|---|
| `GET /api` | Self-describing catalog of endpoints. Includes `apiVersion` for feature detection. |
| `GET /api/status` | Engine liveness summary. |
| `GET /api/scene?root=<id>&depth=<n>&max=<n>` | Scene graph dump. |
| `GET /api/scene/node/<id>` | Deep dive on a single node. |
| `GET /api/render/frame?form=<form>&...` | Capture the rendered frame in a text form. |
| `POST /api/eval` | Run a Squirrel snippet on the main thread. **Mutating.** |
| `GET /api/input/actions` | List the project's input action sets and names. |
| `POST /api/input/action` | Inject a button or axis action. **Mutating.** |
| `POST /api/input/mouse` | Press a mouse button or move the pointer. **Mutating.** |
| `POST /api/input/clear` | Release everything being spoofed. **Mutating.** |
| `GET /api/input/state` | What input is currently being spoofed. |

### Node identifiers

Ogre-Next scene nodes have **no names** — each is addressed by its numeric **id**
(`Ogre::Node::getId()`), which is stable for the life of the node. You obtain ids from
`/api/scene` and use them in `root=` and `/api/scene/node/<id>`.

### `GET /api/status`

```jsonc
{
  "engineVersion": "0.1.0 unstable",
  "gitHash": "6946087",
  "apiVersion": 1,
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

### `POST /api/eval`

Compile and run a Squirrel snippet on the engine's main thread, with the full engine
script API (`_scene`, `_world`, `_entity`, `_window`, `_gui`, ... — the same globals
project scripts see). The one endpoint that **mutates engine state**.

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

### `GET /api/input/actions`

Lists the input action sets the project defines, so you can learn valid action names
before injecting them. Read-only.

```jsonc
{
  "actionSets": [
    { "name": "Default", "actions": [
      { "name": "Accept",   "type": "button" },
      { "name": "LeftMove", "type": "axis" },
      { "name": "LeftTrigger", "type": "trigger" }
    ] }
  ]
}
```

### `POST /api/input/action`

Injects a button or stick/axis action, as if a player pressed it. The one input endpoint
that carries the shared timing semantics; the others reuse them. **Mutating.**

```sh
# Hold "Accept" for 10 frames, then it auto-releases:
curl -s -X POST localhost:8788/api/input/action -d '{"action":"Accept","type":"button","value":true,"frames":10}'
# {"ok":true,"frame":842,"releasesAtFrame":852}

# Push a stick fully right, held until released (omit frames), then recentre to release:
curl -s -X POST localhost:8788/api/input/action -d '{"action":"LeftMove","type":"axis","x":1.0,"y":0.0}'
curl -s -X POST localhost:8788/api/input/action -d '{"action":"LeftMove","type":"axis","x":0,"y":0}'
```

Semantics:

- Body fields: `action` (name from `/api/input/actions`), `type` (`"button"` or `"axis"`,
  default `"button"`), `value` (button bool), `x`/`y` (axis floats), `frames`.
- `frames` holds the input for N rendered frames then auto-releases. Omit it (or pass
  `-1`) to hold until you release it (`value:false`, recentre an axis, or `/api/input/clear`).
- Response: `{"ok": true, "frame": <n>, "releasesAtFrame": <n>}`, or
  `{"ok": false, "error": "..."}` (400) for an unknown action name. `releasesAtFrame` is
  omitted for indefinite holds; use it to time verification — poll a render capture until
  that frame passes, then check the effect.
- Injected input is indistinguishable from real hardware input downstream, deliberately,
  so the game behaves identically. Button actions target controller device 0, which also
  feeds the "any device" aggregate most games query. Real hardware input still works
  concurrently; last writer per frame wins.

### `POST /api/input/mouse`

Presses/releases a mouse button, or warps the pointer to a normalised window position.
**Mutating.**

```sh
# Move the pointer to screen centre (instantaneous, no lifetime):
curl -s -X POST localhost:8788/api/input/mouse -d '{"moveTo":[0.5,0.5]}'
# Left-click for 2 frames:
curl -s -X POST localhost:8788/api/input/mouse -d '{"button":0,"pressed":true,"frames":2}'
```

Semantics:

- Provide either `moveTo:[x,y]` (normalised 0–1) or `button` (0 left, 1 right, 2 middle)
  with `pressed` and optional `frames` (same lifetime rules as `/api/input/action`).
- Mouse buttons route through the GUI so hit-testing matches a real click. `moveTo` has
  no lifetime, so its response omits `releasesAtFrame`.

### `POST /api/input/clear`

Releases everything currently being spoofed — the panic button. **Mutating.**

```sh
curl -s -X POST localhost:8788/api/input/clear
# {"ok":true,"frame":903}
```

### `GET /api/input/state`

Reports what the debug server is currently spoofing. Read-only.

```jsonc
{
  "frame": 905,
  "active": [
    { "input": "button:Accept", "framesRemaining": -1 }   // -1 = held indefinitely
  ]
}
```

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
