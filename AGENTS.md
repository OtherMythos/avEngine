# avEngine — Agent Guide

avEngine is a data-driven, cross-platform 3D game engine (Ogre-Next + Squirrel). One `av`
binary runs any project; a project is a directory with an `avSetup.cfg`.

This file holds what every session needs: how to build, test and verify a change, and how to
launch the engine. Each subsystem's reference lives in [`docs/`](docs/), and the
[table at the end](#reference-docs) says which file to open.

## Building and verifying a change

Any non-trivial change should be verified before it's considered done:

- **Always** rebuild the engine and confirm it still compiles.
- Run the unit tests (avUnit) and confirm there are no failures.
- Run the integration tests (avTests) in headless mode.
- For changes to **runtime/render behaviour** — launch the engine against an avData project
  and inspect it live with the [agent debug server](docs/debug-server.md).

### Building

Third-party dependencies are pre-built into `avBuilt/<BuildType>` by the separate
[avBuild](https://github.com/OtherMythos/avBuild) scripts, and a configured build tree already
exists at `$AV_ENGINE_DIR/build` (Xcode generator on macOS), so day to day you only rebuild.
`xcodebuild` is extremely verbose, so filter it down to the result line and any errors:

```sh
cd "$AV_ENGINE_DIR/build"
xcodebuild -project av.xcodeproj -target av -configuration Debug build 2>&1 | grep -iE "error:|BUILD SUCCEEDED|BUILD FAILED" | head
```

A single-file change is a fast incremental rebuild; you don't need a clean build.

**Added, removed or renamed a source file? Run `cmake ..` from the build directory first.**
Sources are globbed at configure time, so an incremental build can't see the new file. Editing
a `CMakeLists.txt`, changing a build option or updating the dependencies needs it too. Other
host OSes, fresh build trees and the build options are in [docs/building.md](docs/building.md).

### Unit tests

```sh
cd "$AV_ENGINE_DIR/build"
xcodebuild -project av.xcodeproj -target avUnit -configuration Debug build 2>&1 | grep -iE "error:|BUILD SUCCEEDED|BUILD FAILED" | head
cd test/unit/Debug && ./avUnit 2>&1 | grep -iE "FAILED|PASSED" | tail
```

A healthy run ends with a line like `[  PASSED  ] 403 tests.`. On macOS the engine sources are
compiled into `avUnit` as well as `av`, so building it recompiles them.

**No unit test may take longer than 30ms**, and most should be under 1ms. A slower test is an
integration test in the wrong place, or is sleeping when it could be waiting.
[docs/testing.md](docs/testing.md) covers finding and fixing slow tests, reaching private
members, and running avUnit on Linux and Windows.

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

What each runner flag buys, and how to find the slowest test cases, is in
[docs/testing.md](docs/testing.md#running-the-integration-test-suite).

## Running the engine

Pass the project's `avSetup.cfg` first and flags after it. `--debugServer` and
`--profileScripts` take an optional value, and the argument parser misreads them if they come
before the path.

```sh
"$AV_ENGINE_DIR/build/Debug/av.app/Contents/MacOS/av" /path/to/project/avSetup.cfg --headless --noDebugger --debugServer
```

| Flag | Effect |
|---|---|
| `--headless` | No window at all; the scene still renders for real, offscreen. Use it for every automated or agent-driven run. See [docs/headless.md](docs/headless.md). |
| `--noDebugger` | A script failure doesn't park the engine waiting for a debugger, which would hang a headless run until it times out. |
| `--debugServer [port]` | Localhost REST API for inspecting and driving the engine, on port 8788 by default. See [docs/debug-server.md](docs/debug-server.md). |
| `--fixedUpdateRate <n>`, `--physicsUpdateRate <n>` | Tick rates, clamped to 1–240. Always change both together. |
| `--simulationAudio [false]` | Mix audio on the simulation clock into a loopback device, with no speaker output. `--headless` implies it. See [docs/simulation-control.md](docs/simulation-control.md#timing-boundaries-and-audio). |
| `--rendersystem <name>` | `D3D11`, `Metal`, `OpenGL` or `Vulkan`. Anything else is logged as unrecognised and the platform default is used. |
| `--profileScripts [path]` | Profile Squirrel execution. See [docs/script-profiler.md](docs/script-profiler.md). |
| `--logFile <path>` | Write the engine log to this path instead of the default. |

The log goes to `~/Library/Logs/av/av.log` on macOS, `%APPDATA%/av/av.log` on Windows and
`~/.local/share/av/logs/av/av.log` on Linux, and is truncated at startup. `--logFile`
creates missing parent directories and replaces an existing file. It exists so several
engines can run at once without truncating and interleaving into the same `av.log`, which is
how the integration test runner uses it.

## Agent debug server

`--debugServer` serves a REST API on `127.0.0.1:8788` for inspecting and driving a running
engine. `GET` endpoints are read-only and `POST` endpoints mutate. `curl -s localhost:8788/api`
returns the live catalog and its `apiVersion`. Parameters and response shapes are in
[debug-server.md](docs/debug-server.md) (status, scene, render, eval),
[debug-server-input-and-gui.md](docs/debug-server-input-and-gui.md),
[simulation-control.md](docs/simulation-control.md), [video-capture.md](docs/video-capture.md),
[audio.md](docs/audio.md#audio-inspection) and [script-profiler.md](docs/script-profiler.md).

| Endpoint | What it does |
|---|---|
| `GET /api` | Catalog of endpoints, with `apiVersion` for feature detection. |
| `GET /api/status` | Engine version, project, render system, fps and window size. |
| `GET /api/scene` | Scene graph dump, bounded by `root`, `depth` and `max`. |
| `GET /api/scene/node/<id>` | One node: transform, parent chain, attached objects and their AABBs. |
| `GET /api/render/frame` | The rendered frame as `stats`, `grid`, `ascii` or `png`. |
| `GET /api/render/hash` | Perceptual hash of the frame; poll it until the screen settles. |
| `POST /api/render/snapshot` | Store the current frame under a name. |
| `GET /api/render/snapshots` | List stored snapshot names. |
| `GET /api/render/compare` | Diff a snapshot against another or the live frame: how much changed, and where. |
| `GET /api/render/find` | Locate regions of a colour on screen. |
| `POST /api/eval` | Run a Squirrel snippet on the main thread and return its result. |
| `GET /api/input/actions` | The project's input action sets and action names. |
| `POST /api/input/action` | Press a button or push an axis, for N frames or ticks or until released. |
| `POST /api/input/mouse` | Move the pointer or press a mouse button. |
| `POST /api/input/clear` | Release all spoofed input. |
| `GET /api/input/state` | What input is currently being spoofed. |
| `GET /api/input/layers` | The input layer stack, and which layer is swallowing input. |
| `GET /api/gui/tree` | GUI window and widget hierarchy. |
| `GET /api/gui/labels` | Every on-screen text element with its position. |
| `GET /api/gui/at` | Which widgets are under a point. |
| `GET /api/gui/widget/<id>` | One widget: geometry, state, text, parent and children. |
| `GET /api/simulation` | Mode, tick, simulation time, speed and the latest step. |
| `POST /api/simulation/pause` | Pause at the next tick boundary. |
| `POST /api/simulation/resume` | Resume, optionally at a slower speed. |
| `POST /api/simulation/step` | Run N ticks while paused. |
| `POST /api/video/start` | Start recording an MP4, optionally with the engine's audio. |
| `POST /api/video/stop` | Stop recording and drain the encoder. |
| `GET /api/video/status` | Recording state, frame counts, output path and any error. |
| `GET /api/audio` | Audio device, listener, gain, object counts and event cursor. |
| `GET /api/audio/sources` | Paged list of live audio sources. |
| `GET /api/audio/source/<id>` | One source: playback and spatial state, and silence diagnostics. |
| `GET /api/audio/buffers` | Paged list of loaded audio assets. |
| `GET /api/audio/buffer/<id>` | One asset: metadata, load errors and peak/RMS. |
| `GET /api/audio/events` | Recent audio history, including sounds that have already ended. |
| `GET /api/profiler` | Top Squirrel functions by time or call count. Needs `--profileScripts`. |
| `GET /api/profiler/function/<id>` | One function: callers, callees and hottest lines. |
| `GET /api/profiler/frames` | Script time and call count per frame. |
| `GET /api/profiler/lines` | Hottest source lines by self time. |
| `POST /api/profiler/start`, `/stop`, `/reset` | Control profiler collection. |
| `POST /api/profiler/dump` | Write the full profiler report to disk. |

## Reference docs

| Doc | Open it when you're… |
|---|---|
| [docs/building.md](docs/building.md) | Building on Linux or Windows, configuring a fresh build tree, or toggling a build option. |
| [docs/testing.md](docs/testing.md) | Running avUnit on Linux or Windows, writing or speeding up a unit test, testing private members, or working out what the integration suite's flags do and which tests are slow. |
| [docs/plugins.md](docs/plugins.md) | Writing or loading a plugin, Squirrel or native, including the native ABI and symbol-export traps. |
| [docs/headless.md](docs/headless.md) | Running headless: what behaves differently, and which render systems run with no display. |
| [docs/debug-server.md](docs/debug-server.md) | Inspecting a running engine: scene graph, frame capture and comparison, `eval`, and debugging recipes. |
| [docs/debug-server-input-and-gui.md](docs/debug-server-input-and-gui.md) | Driving a running engine: injecting input, reading the GUI, clicking widgets, or finding what swallowed a click. |
| [docs/simulation-control.md](docs/simulation-control.md) | Pausing and stepping fixed ticks, pausing from script, holding input for N ticks, or working out what runs per tick. |
| [docs/video-capture.md](docs/video-capture.md) | Recording frame-exact MP4 footage with the engine's audio. |
| [docs/audio.md](docs/audio.md) | Diagnosing silence, inspecting audio sources and buffers, or streaming long music with `_audio.newStream`. |
| [docs/script-profiler.md](docs/script-profiler.md) | Finding which Squirrel functions are slow or called too often. |

Design plans for each release are in `plan/<version>/`.

When you change a subsystem, update its doc in the same change. A new subsystem gets its own
`docs/<name>.md` and a row in this table, not a section in this file: keep this file to what
every session needs. A new debug server endpoint gets one line in the list above, with its
detail in the subsystem's doc.
