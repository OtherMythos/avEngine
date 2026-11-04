# Running without a window (`--headless`)

Add `--headless` to run with **no window on screen at all** — no SDL video subsystem, no
OS window, and on macOS the process isn't even registered with the window server. The
scene still renders for real, with the real render system, into an offscreen texture that
[`/api/render/*`](debug-server.md) reads back exactly as it would a window's backbuffer.

```sh
av /path/to/project/avSetup.cfg --headless --debugServer
```

Prefer this for any automated or agent-driven run: it stops a window flashing up over
whatever the user is doing, and it's the only way to run the engine over SSH or in CI. It
is desktop only and is ignored on iOS/Android. Unlike `--debugServer`, `--headless` takes
no value, so it's safe to put on either side of the positional `avSetup.cfg` path.

Four things behave differently headless, all of them deliberate:

- **Frames free-run.** There's nothing to present to, so vsync is force-disabled and the
  engine renders as fast as the GPU allows — commonly 2000+ fps rather than 60. That makes
  tests finish sooner, and sidesteps the OS throttling an occluded window. It also means
  one CPU core stays pegged for the duration of the run.
- **Input hold durations are in *rendered* frames**, and there are now far more of them per
  fixed update. A `POST /api/input/action` with `"frames": 1` can be released before a
  single fixed-step update observes it. Hold inputs for many more frames than you would
  windowed, or hold them indefinitely and `POST /api/input/clear` when done, or count the hold in
  [`ticks`](simulation-control.md#input-lifetimes) instead.
- **The capture is 1:1 with the configured window size**, not the retina backing scale, so
  `captureWidth`/`captureHeight` are `WindowWidth`×`WindowHeight` rather than double that.
  Set the capture resolution with `WindowWidth`/`WindowHeight` in `avSetup.cfg`, or
  `_window.setIntendedWidth()`/`setIntendedHeight()` from the Squirrel `setup()` function.
- **Audio never reaches the speakers.** `--headless` implies
  [`--simulationAudio`](simulation-control.md#timing-boundaries-and-audio): OpenAL mixes into a loopback device
  on the simulation clock, so any number of engines can run alongside someone's desktop
  session in silence. Sounds advance with simulation ticks rather than the device clock,
  so a pause or a long stall freezes them along with gameplay. Pass
  `--simulationAudio false` to hear a headless run. If you expected sound and got none, the
  log says `Headless audio uses the simulation loopback`, and `/api/audio`'s `device` names
  the loopback.

Whether a render system can run with no display server depends on the platform and on the
avBuilt dependencies:

| Platform | Render system | Display server | Notes |
|---|---|---|---|
| macOS | Metal | Not needed | Uses `MetalWindowNull`, from avBuild's `ogreMetalHeadless.diff`. |
| Linux | OpenGL (default) | Not needed | Uses Ogre's EGL backend (`OGRE_GLSUPPORT_USE_EGL_HEADLESS`, which avBuild turns on). GLX logs `Couldn't open X display` first, which is expected. Without the EGL backend it falls back to a hidden GLX window, which needs X. |
| Linux | Vulkan | Not needed with avBuilt `deps-1e0294d` or later | Those deps carry avBuild's `ogreVulkanXcbHeadless.diff`. With older deps the render system aborts on construction with `Malformed resolution string` unless `DISPLAY` and `XAUTHORITY` point at a running X server. |
| Windows | Vulkan | Not needed | |
| Windows | OpenGL (default) | Hidden window | Runs over SSH. Logs `Option named 'Interface' does not exist` and a warning on every launch, which is expected. |
| Windows | D3D11 | Hidden window | Needs an interactive desktop session. Over SSH it fails with `Unable to create swap chain`. |

The engine logs a warning whenever it has to fall back.

`--rendersystem` accepts `D3D11`, `Metal`, `OpenGL` and `Vulkan`, ignoring case. Anything else,
including Ogre's own `GL3Plus`, only logs `User requested rendersystem '...' is unrecognised` and
carries on with the platform default. Grep the log for `User requested rendersystem` before
trusting a per-backend result.
