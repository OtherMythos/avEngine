# avEngine — Agent Guide

avEngine is a data-driven, cross-platform 3D game engine (Ogre-Next + Squirrel). This
document covers two things an agent working in this repo needs:

1. **[Building and verifying a change](#building-and-verifying-a-change)** — how to compile
   the engine and run the unit tests on each host OS, when a CMake re-configure is needed,
   and how to run the (separate-repo) integration suite fast enough to use routinely.
2. **[Agent debug server](#agent-debug-server)** — a localhost REST API for inspecting and
   driving a *running* engine, to verify runtime behaviour.
3. **[Squirrel script profiler](#squirrel-script-profiler)** — measures which script
   functions are slow and which are called often.

## Building and verifying a change

Any non-trivial change should be verified before it's considered done:

- **Always** rebuild the engine and confirm it still compiles.
- Run the unit tests (avUnit) and confirm there are no failures.
- Run the integration tests (avTests) in headless mode.
- For changes to **runtime/render behaviour** — launch the engine against an avData project
  and inspect it live (see the [Agent debug server](#agent-debug-server)).

### Required verification workflow

**The integration suite is mandatory for every non-trivial completed change.** Run the
normal `avTestsIntegration.cfg` suite after the final engine rebuild, even when `avUnit`
passes and even when the change appears isolated. Do not report the change as complete while
the suite has failures, or before it has run. If an integration failure leads to another edit,
rebuild and rerun the full suite again; an earlier passing run does not validate the final tree.

```sh
export AV_ENGINE_DIR=/path/to/avEngine
export AV_TESTS_DIR=/path/to/avTests
export AV_TOOLS_DIR=/path/to/avTools
mkdir -p /tmp/av-integration-logs
cd "$AV_TOOLS_DIR/testRunner"
python3 testRunner.py \
  -p "$AV_TESTS_DIR/avTestsIntegration.cfg" \
  -e "$AV_ENGINE_DIR/build/Debug/av.app/Contents/MacOS/av" \
  -f "--headless --noDebugger --fixedUpdateRate 240 --physicsUpdateRate 240" \
  -j 8 \
  -l /tmp/av-integration-logs \
  -o /tmp/av-integration-results.xml
```

- The runner writes `avTestFile.txt` result markers into every executed test directory in
  `avTests`; an agent sandboxed to `avEngine` must request the required write permission.
  These markers are runner output, not test-source edits.
- `-l` only accepts an **existing directory**, hence `mkdir -p` above. Keep JUnit XML and
  logs under `/tmp` so verification artifacts do not dirty either repository.
- A successful suite ends with `You have no failing tests` and the runner exits `0`.
  Individual engine processes commonly exit with code `1` after writing their test marker;
  this is expected and is not itself a test failure. Trust the runner's final summary and
  exit status.
- `avTestsIntegration.cfg` is the routine suite (it deliberately excludes slow `stress/`).
  Keep both update-rate flags at the same value: changing only one breaks tests that assert
  collision-event counts per update.

### How the build is laid out

The engine uses **CMake**. Third-party dependencies (Ogre-Next, Bullet, Squirrel, SDL2,
Colibri, …) are **pre-built** into an `avBuilt/<BuildType>` directory by the separate
[avBuild](https://github.com/OtherMythos/avBuild) scripts, and their location is passed to
CMake as `AV_LIBS_DIR`. You do **not** rebuild dependencies as part of normal work.

A configured build tree normally already exists in a local checkout, so day to day you just
rebuild it rather than re-configuring from scratch:

| | macOS |
|---|---|
| Build directory | `$AV_ENGINE_DIR/build` |
| Generator | Xcode |
| Dependencies (`AV_LIBS_DIR`) | `/path/to/avBuilt/Debug` |
| Engine binary | `build/Debug/av.app/Contents/MacOS/av` |
| Unit test binary | `build/test/unit/Debug/avUnit` |

### Quick build check

The fastest way to confirm a change compiles. `xcodebuild` is extremely verbose, so filter
its output down to the result line and any errors:

```sh
cd "$AV_ENGINE_DIR/build"
xcodebuild -project av.xcodeproj -target av -configuration Debug build 2>&1 | grep -iE "error:|BUILD SUCCEEDED|BUILD FAILED" | head
```

A single-file change is a fast incremental rebuild (recompile the changed translation unit
+ relink — you don't need a clean build.

### Building per host OS

The build tree is generator-specific. The commands below assume you're inside the build
directory.

- **macOS** (Xcode generator):
  ```sh
  # Either the native tool (best for the grep-filtered quick check above):
  xcodebuild -project av.xcodeproj -target av -configuration Debug build
  # …or the generator-agnostic CMake wrapper:
  cmake --build . --target av --config Debug
  ```
- **Windows** (Visual Studio / MSVC — multi-config, so `--config` is required):
  ```sh
  cmake --build . --target av --config Debug
  ```
- **Linux** (Make or Ninja — single-config, `--config` is ignored):
  ```sh
  cmake --build . --target av
  ```

### Running the unit tests

The tests are a GoogleTest executable, the `avUnit` target, built when `BUILD_UNIT_TESTS`
is `ON` (the default). Build the target, then run the binary directly.

- **macOS**:
  ```sh
  cd "$AV_ENGINE_DIR/build"
  xcodebuild -project av.xcodeproj -target avUnit -configuration Debug build 2>&1 | grep -iE "error:|BUILD SUCCEEDED|BUILD FAILED" | head
  cd build/test/unit/Debug && ./avUnit
  ```
  (On Apple platforms there is no shared `avCore` library — the engine sources are compiled
  directly into both `av` and `avUnit`, so an `avUnit` build recompiles the engine sources.)
- **Linux**:
  ```sh
  cmake --build . --target avUnit
  cd build/test/unit && ./avUnit
  ```
- **Windows**:
  ```sh
  cmake --build . --target avUnit --config Debug
  .\test\unit\Debug\avUnit.exe
  ```

A healthy run ends with a line like `[  PASSED  ] 222 tests.`. Filter to just the summary
and failures with `./avUnit 2>&1 | grep -iE "FAILED|PASSED|OK \]" | tail`.

#### Unit tests must be fast — 30ms is the hard ceiling

**No single unit test may take longer than 30ms**, and most should be under 1ms. The whole
`avUnit` binary is meant to run in a fraction of a second so it can be used constantly, as a
matter of course after any change, rather than being something you avoid running. A test that
needs seconds is either an integration test in the wrong place, or is sleeping when it could be
waiting. gtest prints each test's duration, so the offenders are easy to find:

```sh
./avUnit 2>&1 | grep "OK \]" | sed 's/^\[       OK \] //' | sort -t'(' -k2 -rn | head
```

Nearly always the cost is one of these, none of which is inherent to what's being tested:

- **Polling for something instead of blocking on it.** If production code signals a
  `std::condition_variable`, the test should wait on that same variable with a `wait_for`
  timeout — it wakes in microseconds *and* stays bounded, so a regression still fails rather
  than hanging the run. A `while(!done) sleep_for(1ms)` loop costs a full millisecond per
  iteration no matter how fast the thing being waited for actually is.
- **Spawning a thread per operation.** Thread creation dwarfs most operations under test. Wait
  on the real synchronisation primitive from the test thread instead, and reserve extra threads
  for the cases whose whole point is that two threads are involved.
- **A fixed `sleep_for` used as "wait long enough".** Yield in a bounded loop until the
  condition actually holds, or restructure so the case is deterministic.
- **Huge iteration counts on a probabilistic race test.** These have sharply diminishing
  returns; a race worth catching reproduces in hundreds of iterations, not tens of thousands.

If a test genuinely cannot be brought under 30ms, it does not belong in `avUnit` — move it to
the integration suite, where per-test cost is expected.

### When you need to re-run CMake

The build tree is already configured, but some changes require a **re-configure** before a
rebuild will pick them up:

- **You added, removed, or renamed a source file.** The build globs sources with
  `file(GLOB_RECURSE srcs "src/*.cpp")`, which is only evaluated at configure time — new
  files are invisible to an incremental build until you reconfigure. This is the most common
  reason a build "doesn't see" your new `.cpp`/`.h`.
- **You edited a `CMakeLists.txt`** (root or `test/unit/`).
- **You changed a build option** (see the table below) or the build type.
- **The pre-built dependencies were updated.**

Re-configure by re-running CMake from the build directory (it reuses the cached variables):

```sh
cd "$AV_ENGINE_DIR/build"
cmake ..
```

To configure a fresh build tree from scratch (e.g. a new build directory), pass the
dependency path and — on macOS — the Xcode generator:

```sh
mkdir build && cd build
cmake -DAV_LIBS_DIR=/path/to/avBuilt/Debug -DCMAKE_BUILD_TYPE=Debug -GXcode ..   # macOS
cmake -DAV_LIBS_DIR=/path/to/avBuilt/Debug -DCMAKE_BUILD_TYPE=Debug ..           # Linux / Windows
```

### Build options

Toggle these with `-D<OPTION>=ON|OFF` at configure time:

| Option | Default | Purpose |
|---|---|---|
| `BUILD_UNIT_TESTS` | `ON` | Build the `avUnit` GoogleTest target. |
| `DEBUG_SERVER` | `ON` (desktop) | Compile in the [agent debug server](#agent-debug-server). Excluded on iOS/Android. |
| `SCRIPT_PROFILER` | `ON` | Compile in the [Squirrel script profiler](#squirrel-script-profiler). Available on every platform. |
| `TEST_MODE` | `ON` | Engine testing capabilities. |
| `DEBUGGING_TOOLS` | `ON` | Developer tools (debug draw, Squirrel debugging). |
| `USE_STATIC_PLUGINS` | `OFF` | Compile a `StaticPlugins.h` from `AV_PROJECT_DIR` (required for iOS/Android). |

### Redirecting the engine log (`--logFile`)

By default the engine writes its log to one fixed per-user location — `~/Library/Logs/av/av.log`
on macOS, `%APPDATA%/av/av.log` on Windows, `~/.local/share/av/logs/av/av.log` on Linux —
and truncates it on startup. `--logFile` overrides that with a full path to write to instead:

```sh
av /path/to/project/avSetup.cfg --headless --logFile /tmp/testLogs/MyTest.log
```

Missing parent directories are created, and an existing file at the path is removed, exactly
as with the default location. This exists so **several engine processes can run at the same
time**: without it they all truncate and interleave into the same `av.log`, which is what the
integration test runner does when running tests concurrently.

### Running the integration test suite

The integration tests are a separate pair of repos, not part of this one:

- **[avTests](https://github.com/OtherMythos/avTests)** — one `avSetup.cfg` + `squirrelEntry.nut`
  pair per test case, grouped into plans (`integration/<Plan>/<TestCase>/`). Point at
  `avTestsIntegration.cfg` for routine runs — it covers everything except the `stress/` plan,
  which is slow load/lifetime testing you don't need for a normal check.
- **[avTools/testRunner](https://github.com/OtherMythos/avTools)** — `testRunner.py` drives
  the engine against each test case and collects `avTestFile.txt` (the pass/fail/still-running
  marker the engine writes on exit) into a summary, optionally as JUnit XML.

Run serially against a single engine window and this suite takes on the order of ten minutes
and puts a window on screen once per test case. Three independent levers bring that down to
under two minutes with nothing visible at all — always use `--headless` for any automated or
agent-driven run, for the same reasons as a single engine launch (see "Running without a
window" above):

```sh
cd "$AV_TOOLS_DIR/testRunner"
python3 testRunner.py \
  -p "$AV_TESTS_DIR/avTestsIntegration.cfg" \
  -e "$AV_ENGINE_DIR/build/Debug/av.app/Contents/MacOS/av" \
  -f "--headless --noDebugger --fixedUpdateRate 240 --physicsUpdateRate 240" \
  -j 8 \
  -o /tmp/results.xml
```

- **`-f "--headless --noDebugger ..."`** — flags forwarded to every engine invocation.
  `--headless` is what stops 200+ windows flashing on screen; `--noDebugger` stops a script
  failure parking a headless engine in a debugger it can't show you, which would otherwise hang
  that test case until the wall-clock timeout. `-f`'s value needs at least two words, or
  `argparse` swallows a single leading `--...` as if it were its own flag — `-f "--headless"`
  alone fails with "expected one argument".
- **`--fixedUpdateRate <n> --physicsUpdateRate <n>`** (engine flags, both required together,
  clamp 1–240) — most test scripts wait on a tick count (a grace period, a retry loop, N
  updates of no change) rather than a fixed real-time duration, so raising the update rate
  compresses those waits proportionally. It does **not** speed up anything keyed to real
  time — animation playback still takes one real second per real second
  elapsed regardless of the rate, so a test dominated by one of those won't get faster this
  way — the per-test timing below is how you find which ones those are. **Both flags must move together**:
  physics steps once per script update only when `PhysicsUpdateRate / FixedUpdateRate == 1`,
  which is what every test asserting an exact collision-event count per update relies on —
  raising `FixedUpdateRate` alone breaks them. 240 is the ceiling both clamp at, so it's the
  natural fast setting rather than an arbitrary choice.
- **`-j <n>`** — how many test cases `testRunner.py` runs at once, as genuine concurrent OS
  processes (a `ThreadPoolExecutor` whose workers block in `subprocess.communicate()`, which
  releases the GIL, so this is real parallelism, not thread-scheduling inside one process). Set
  it to roughly your core count; each engine process pins close to one core, mostly in
  file/resource loading rather than CPU-bound work, so pushing far past the core count buys
  little. Running concurrently buffers each test's console output until it finishes (serial
  output still streams live), so nothing interleaves.

Every test case's wall time is timed to sub-millisecond precision and printed as it finishes
(`[i/total] TestName (Plan) - 1234.567ms`), and written into the JUnit `time` attribute (`-o`)
in fractional seconds. That's the quickest way to find what's actually slow after applying the
levers above — sort the XML descending:

```sh
python3 -c "
import xml.etree.ElementTree as ET
rows = []
for s in ET.parse('/tmp/results.xml').getroot().iter('testsuite'):
    for tc in s.iter('testcase'):
        t = tc.get('time')
        if t: rows.append((float(t) * 1000.0, s.get('name'), tc.get('name')))
rows.sort(reverse=True)
for ms, plan, name in rows[:20]:
    print('%8.1fms  %-25s %s' % (ms, plan, name))
"
```

A test that's still slow after `--fixedUpdateRate`/`--physicsUpdateRate` are maxed out is
either waiting on real time (an animation) or doing genuinely heavy work
(chunk/resource loading) rather than idling on a tick count — worth shortening the test itself
rather than expecting the rate flags to help further.

## Agent debug server

A **localhost REST API** for inspecting and driving a running avEngine instance. It lets
an AI agent (or a developer with `curl`) query the live Ogre scene graph, capture the
rendered frame, and execute Squirrel snippets — useful for answering questions like
*"why is my mesh not visible?"* without attaching a debugger.

The inspection endpoints (`GET`) never mutate engine state; the `POST` endpoints —
`/api/eval` and input spoofing — can. This is a trusted local development tool: it binds
to `127.0.0.1` only, and is compiled in behind the `DEBUG_SERVER` build option (on by
default for desktop builds, excluded from iOS/Android).

### Enabling it

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

### Running without a window (`--headless`)

Add `--headless` to run with **no window on screen at all** — no SDL video subsystem, no
OS window, and on macOS the process isn't even registered with the window server. The
scene still renders for real, with the real render system, into an offscreen texture that
`/api/render/*` reads back exactly as it would a window's backbuffer.

```sh
av /path/to/project/avSetup.cfg --headless --debugServer
```

Prefer this for any automated or agent-driven run: it stops a window flashing up over
whatever the user is doing, and it's the only way to run the engine over SSH or in CI. It
is desktop only and is ignored on iOS/Android. Unlike `--debugServer`, `--headless` takes
no value, so it's safe to put on either side of the positional `avSetup.cfg` path.

Three things behave differently headless, all of them deliberate:

- **Frames free-run.** There's nothing to present to, so vsync is force-disabled and the
  engine renders as fast as the GPU allows — commonly 2000+ fps rather than 60. That makes
  tests finish sooner, and sidesteps the OS throttling an occluded window. It also means
  one CPU core stays pegged for the duration of the run.
- **Input hold durations are in *rendered* frames**, and there are now far more of them per
  fixed update. A `POST /api/input/action` with `"frames": 1` can be released before a
  single fixed-step update observes it. Hold inputs for many more frames than you would
  windowed, or hold them indefinitely and `POST /api/input/clear` when done.
- **The capture is 1:1 with the configured window size**, not the retina backing scale, so
  `captureWidth`/`captureHeight` are `WindowWidth`×`WindowHeight` rather than double that.
  Set the capture resolution with `WindowWidth`/`WindowHeight` in `avSetup.cfg`, or
  `_settings.setDefaultWidth()` from the Squirrel `setup()` function.

Which render system can genuinely run with no display server depends on the backend:
Metal and Vulkan create no OS window at all, GL3Plus needs Ogre built with
`OGRE_GLSUPPORT_USE_EGL_HEADLESS`, and D3D11 has no windowless mode so it falls back to a
hidden window — nothing appears on screen, but a window station is still required. Use
`--rendersystem Vulkan` for true headless on Windows. The engine logs a warning whenever
it has to fall back.

### Endpoints

All endpoints return `application/json` and are served under `/api`. Most are `GET`
(read-only); the `POST` endpoints mutate engine state and are marked **Mutating** below.

| Endpoint | Description |
|---|---|
| `GET /api` | Self-describing catalog of endpoints. Includes `apiVersion` for feature detection. |
| `GET /api/status` | Engine liveness summary. |
| `GET /api/scene?root=<id>&depth=<n>&max=<n>` | Scene graph dump. |
| `GET /api/scene/node/<id>` | Deep dive on a single node. |
| `GET /api/render/frame?form=<form>&...` | Capture the rendered frame in a text form. |
| `GET /api/render/hash` | Perceptual hash of the live frame (cheap to poll). |
| `POST /api/render/snapshot?name=<slot>` | Store a frame for later comparison. |
| `GET /api/render/snapshots` | List stored snapshot names. |
| `GET /api/render/compare?a=<slot>&b=<slot\|live>` | Diff two frames; says how much and *where*. |
| `GET /api/render/find?rgb=<hex>&tolerance=<n>` | Locate regions of a colour on screen. |
| `POST /api/eval` | Run a Squirrel snippet on the main thread. **Mutating.** |
| `GET /api/input/actions` | List the project's input action sets and names. |
| `POST /api/input/action` | Inject a button or axis action. **Mutating.** |
| `POST /api/input/mouse` | Press a mouse button or move the pointer. **Mutating.** |
| `POST /api/input/clear` | Release everything being spoofed. **Mutating.** |
| `GET /api/input/state` | What input is currently being spoofed. |
| `GET /api/gui/tree?window=<id>&depth=<n>&max=<n>&visibleOnly=<bool>` | GUI window/widget hierarchy. |
| `GET /api/gui/labels?visibleOnly=<bool>` | Flat list of on-screen text with positions. |
| `GET /api/gui/at?x=<f>&y=<f>` | Which widgets are under a normalised point. |
| `GET /api/gui/widget/<id>` | Deep dive on a single widget. |

#### Node identifiers

Ogre-Next scene nodes have **no names** — each is addressed by its numeric **id**
(`Ogre::Node::getId()`), which is stable for the life of the node. You obtain ids from
`/api/scene` and use them in `root=` and `/api/scene/node/<id>`.

#### `GET /api/status`

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

#### `GET /api/scene`

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

#### `GET /api/scene/node/<id>`

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

#### `GET /api/render/frame`

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

#### `GET /api/render/hash`

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

#### `POST /api/render/snapshot?name=<slot>`

Captures the current frame and stores it under a name, so you can compare against it
later without ever holding pixels yourself. Frames are stored downsampled (analysis
resolution, ≤320×180), and slots are capped at 16 — saving past that evicts the oldest.
This mutates only the debug server's own store, never engine state.

```sh
curl -s -X POST "localhost:8788/api/render/snapshot?name=before"
# {"name":"before","frame":558,"width":240,"height":180,"dhash":"0000000000000001"}

curl -s localhost:8788/api/render/snapshots        # {"snapshots":["before"]}
```

#### `GET /api/render/compare?a=<slot>&b=<slot|live>`

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

#### `GET /api/render/find?rgb=<hex>&tolerance=<n>`

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

#### `POST /api/eval`

Compile and run a Squirrel snippet on the engine's main thread, with the full engine
script API (`_scene`, `_world`, `_entity`, `_window`, `_gui`, ... — the same globals
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

#### `GET /api/input/actions`

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

#### `POST /api/input/action`

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

#### `POST /api/input/mouse`

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

#### `POST /api/input/clear`

Releases everything currently being spoofed — the panic button. **Mutating.**

```sh
curl -s -X POST localhost:8788/api/input/clear
# {"ok":true,"frame":903}
```

#### `GET /api/input/state`

Reports what the debug server is currently spoofing. Read-only.

```jsonc
{
  "frame": 905,
  "active": [
    { "input": "button:Accept", "framesRemaining": -1 }   // -1 = held indefinitely
  ]
}
```

#### `GET /api/gui/tree`

The Colibri GUI hierarchy — windows and their nested widgets. Read-only; use
`/api/input/mouse` to actually click things. Same bounding as `/api/scene`
(`depth` default 4, `max` default 400, `childCount` on truncated nodes). `window=<id>`
roots the dump at one widget; `visibleOnly=true` prunes hidden subtrees.

All positions are normalised 0–1 (`rect` is `[x, y, w, h]`, `center` is `[x, y]`), the
**same coordinate space as `/api/input/mouse` and render captures** — so a widget's
`center` can be fed straight to a mouse move/click. `sizeCanvas` gives the raw
Colibri-canvas-unit size for reference.

```jsonc
{
  "canvasSize": [1600, 1200],
  "truncated": false,
  "windows": [
    {
      "id": 0, "userId": 0, "type": "Window", "name": "mainMenu",
      "hidden": false, "disabled": false, "visible": true, "state": "Idle",
      "rect": [0.0, 0.0, 0.31, 0.42], "center": [0.156, 0.208], "sizeCanvas": [500, 500],
      "children": [
        { "id": 3, "type": "Button", "text": "WateringCan", "visible": true,
          "state": "Idle", "rect": [0.006, 0.059, 0.071, 0.032], "center": [0.042, 0.075],
          "children": [ /* the button's internal label, anonymous (no id) */ ] }
      ]
    }
  ]
}
```

Fields: `id` (stable widget handle, present only for project-created widgets — internal
sub-widgets are anonymous), `userId` (project-assigned tag via `setUserId`, 0 if unset),
`type`, `name` (windows only), `hidden`/`disabled`/`state` (raw Colibri state), and
`visible` (the effective visibility after walking the ancestor hidden-chain).

#### `GET /api/gui/labels`

A flat list of every on-screen text element — the project's `Label`s, `Button`s and
`Editbox`es with their text and position (internal sub-widget labels are excluded, so no
duplicates). `visibleOnly` defaults **true**; pass `visibleOnly=false` for hidden ones too.

```jsonc
{
  "canvasSize": [1600, 1200],
  "labels": [
    { "id": 1, "type": "Label",  "text": "Art demos",   "visible": true,
      "rect": [0.006, 0.008, 0.054, 0.019], "center": [0.033, 0.018] },
    { "id": 3, "type": "Button", "text": "Press here", "visible": true,
      "rect": [0.006, 0.059, 0.071, 0.032], "center": [0.042, 0.075] }
  ]
}
```

#### `GET /api/gui/at?x=<f>&y=<f>`

Which widgets contain a normalised (0–1) point, outermost first — "if I click here, what
do I hit?" Pairs with a render capture: spot a control in the pixels, confirm what it is,
then click its centre via `/api/input/mouse`.

```jsonc
{ "point": [0.042, 0.075],
  "hits": [ {"id": 0, "type": "Window"},
            {"id": 3, "type": "Button", "text": "Press here"} ] }
```

#### `GET /api/gui/widget/<id>`

Single-widget deep dive: geometry, state, text, `parentId`, and registered `childIds`.
404 if the id is unknown or stale.

### Recipes

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
`/api/gui/at?x=&y=` tells you what sits under a point you spotted in a render capture.

### Semantics

- **Error shape:** `{ "error": "..." }` with an HTTP status:
  - `404` — the requested node id doesn't exist.
  - `503` — the engine didn't service the request in time (paused in a debugger, hung, or
    shutting down). Requests are executed on the engine's main thread once per rendered
    frame, so a stalled engine will time out after ~2s.
  - `400` — bad parameters.
- Each response is a **snapshot of a single frame**.
- All `GET` endpoints are read-only; the `POST` endpoints (`/api/eval`, `/api/input/*`)
  mutate. Check `apiVersion` in `/api` to detect capability changes.

## Squirrel script profiler

Measures the engine's Squirrel execution: **how often each function is called**, **how
long it takes** inclusive and exclusive of its callees, **which function called it**,
**which frames were expensive**, and **which source lines** the vm spends its time on.

It is compiled in behind the `SCRIPT_PROFILER` build option and off until you ask for it,
like the debug server. Results come out two ways: a **report file**, and — when the debug
server is also running — **`/api/profiler` endpoints**.

### Enabling it

```sh
av /path/to/project/avSetup.cfg --profileScripts                    # log a summary at exit
av /path/to/project/avSetup.cfg --profileScripts /tmp/profile.json  # also dump the full report
av /path/to/project/avSetup.cfg --debugServer --profileScripts      # query it live as well
```

- `--profileScripts [path]` — collect from engine startup. The optional path is where the
  full report is written at shutdown; the format follows the extension (`.json` → JSON,
  anything else → the text table). With or without a path, a top-10 summary goes to the
  engine log at shutdown.
- `--profileScriptsLines <0|1>` — per line timing, on by default. This is the profiler's
  most expensive collector because it does work on every executed line rather than every
  call, so turn it off when measuring something call heavy.

As with `--debugServer`, put these **after** the positional `avSetup.cfg` path.

The report is written during engine shutdown, so let the engine close normally — kill it
and you get nothing. Over the debug server, `_shutdownEngine()` through `POST /api/eval`
closes it cleanly, or use `POST /api/profiler/dump` to write a report without exiting.

### Reading a report

```
==== Squirrel script profile ====
Stopped, collected for 19.73s over 1005 frames
Script time 0.112s (0.6% of wall) | 1156 calls | 18 functions | 42311 lines executed

   excl ms    incl ms        calls     avg us     max us  excl%  function
------------------------------------------------------------------------------------
     63.38      63.38          103     615.38    1161.17  56.4%  move    squirrelEntry.nut:75
     46.79     110.17         1034     106.55    1207.83  41.7%  update  squirrelEntry.nut:122
```

- **exclusive** — time in the function itself, *not* counting Squirrel functions it
  called. This is what you sort by to find the slow code.
- **inclusive** — time from entry to return, callees included. Only the outermost frame of
  a recursive chain contributes, so recursion is not multiply counted.
- The file dump also carries the call edges, the per line breakdown and the frame timeline.

### Endpoints

Available when the engine is run with both `--debugServer` and `--profileScripts`. Without
the latter they answer `{"enabled": false}` with a hint (`409` for the POSTs).

| Endpoint | Description |
|---|---|
| `GET /api/profiler?sort=<key>&max=<n>&minCalls=<n>&include=<sections>` | Top functions. `sort` is `exclusive` (default), `inclusive`, `calls` or `avg`; `max` defaults to 25. |
| `GET /api/profiler/function/<id>` | One function in full: callers, callees, hottest lines. |
| `GET /api/profiler/frames?max=<n>&worst=<bool>` | Per frame script time and call count, with each frame's most expensive root call. |
| `GET /api/profiler/lines?max=<n>` | Hottest source lines by self time. |
| `POST /api/profiler/start` \| `/stop` \| `/reset` | Control collection. **Mutating.** |
| `POST /api/profiler/dump?path=<file>&format=json\|text` | Write the full uncapped report to disk. **Mutating.** |

Bulk sections are opt-in on `/api/profiler`: pass `include=edges,lines,frames` for any
combination. The default response is just the function table.

**`reset` is the important one.** Collection starts at engine startup, so a session is
dominated by load-time work. To profile one specific thing, `POST /api/profiler/reset`,
do the thing, then read the report:

```sh
curl -s -X POST localhost:8788/api/profiler/reset
# ... play, inject input, eval something ...
curl -s "localhost:8788/api/profiler?sort=exclusive&max=15" | jq
curl -s localhost:8788/api/profiler/function/14 | jq          # drill into the top entry
```

Function `id`s are stable across a `reset`, so you can reset and keep drilling into the
same function.

### What it can and cannot see

- **Native engine functions are invisible.** Squirrel's debug hook does not fire for C++
  closures, so `_scene.*`, `_gui.*`, physics and rendering never appear. Their cost lands
  in the **exclusive** time of the Squirrel function that called them. High exclusive time
  means "slow itself **or** calling something expensive in C++" — check the line
  breakdown to tell which.
- **Expect Squirrel to run roughly 1.5–3× slower** while profiling. It is a measurement
  tool, not a monitor, and it perturbs what it measures: functions called very often with
  small bodies look relatively worse than they are.
- **In a `Debug` build the absolute per line numbers are inflated**, because the
  profiler's own per line work is a large share of what it measures. Relative ordering
  still holds. `SCRIPT_PROFILER` is a separate build option from `DEBUGGING_TOOLS`
  precisely so you can profile an optimised build.
- The profiler and the Squirrel debugger share the vm's single debug hook via
  `SquirrelHookDispatcher` ([src/Scripting/SquirrelHookDispatcher.h](src/Scripting/SquirrelHookDispatcher.h)),
  so breakpoints still work with profiling on.
