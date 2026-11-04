# Simulation control

Pause, slow down and step fixed simulation ticks while the debug server stays responsive.
Available with [`--debugServer`](debug-server.md); API version **4** added this control. A tick is one fixed
update, not one rendered frame.

```sh
curl -s -X POST localhost:8788/api/simulation/pause -d '{"reason":"inspect player"}'
curl -s -X POST localhost:8788/api/input/action -d '{"action":"MoveForward","value":true,"ticks":3}'
curl -s -X POST localhost:8788/api/simulation/step -d '{"ticks":1}'
curl -s localhost:8788/api/simulation
curl -s -X POST localhost:8788/api/simulation/resume -d '{"speed":0.25}'
```

Use an action name returned by `/api/input/actions`. Await each response before sending
its dependent request. While paused, an acknowledged input change is applied before the
next step. Explicit input and eval calls may mutate the paused scene. During free running,
HTTP arrival time does not identify a deterministic tick.

| Endpoint | Body / behaviour |
|---|---|
| `GET /api/simulation` | `mode`, completed `tick`, `simulationSeconds`, `fixedUpdateRate`, `speed`, `pauseReason`, and the latest `step` if any. |
| `POST /api/simulation/pause` | Optional `reason` string, at most 1024 bytes. Completes at a safe tick boundary. Interrupts an active batch and discards its remaining ticks. |
| `POST /api/simulation/resume` | Optional `speed` in `(0,1]`; omission retains the previous speed. Returns `409` during an active step; pause that step first. |
| `POST /api/simulation/step` | Optional integer `ticks` from 1 to 10000, default 1. Requires paused mode (`409` otherwise). Finishes paused; a script pause can stop the batch early. |

```json
{
  "mode": "paused",
  "tick": 817,
  "simulationSeconds": 13.616666666666667,
  "fixedUpdateRate": 60,
  "speed": 1.0,
  "pauseReason": "Player lost health",
  "step": {
    "id": 2,
    "startTick": 800,
    "ticksRequested": 120,
    "ticksExecuted": 17,
    "complete": true
  }
}
```

`pauseReason` is the last pause reason, including while running/stepping. `step` describes
the latest batch, not a fresh batch for each status query. Only completed ticks contribute
to simulation time; speed changes tick pacing and never changes the fixed update delta.

A step returns **200** after completion and rendering. If it is still running after two
seconds, it returns **202** with its accepted step ID and initial status. Poll
`GET /api/simulation` until that step's `complete` is true; **do not repeat the POST**.
The main thread continues servicing requests between ticks, so a batch can be interrupted.
An acknowledgement timeout returns **503**: inspect status before retrying a mutation,
because a request already claimed by the main thread may still finish. A single blocked
Squirrel/native callback cannot be pre-empted. Shutdown wakes waiting step requests.

## Script-triggered pauses

Call this from the game's existing damage handler, a collision callback, or test script:

```squirrel
_debug.pauseSimulation("Player lost health");
```

The call returns normally and the rest of the current tick completes. No following tick
runs until an agent steps or resumes. If several callbacks request a pause in one tick,
the first reason wins. A pause on the final requested tick retains the script reason.
This is not a Squirrel debugger breakpoint and does not retain a suspended call stack.
The namespace is compiled with `DEBUG_SERVER`, is absent from worker VMs, and the function
requires the runtime `--debugServer` flag so unattended games cannot silently stop.

## Input lifetimes

The existing action and mouse-button APIs accept **`ticks`** as an alternative to `frames`.
Positive values release after exactly that many completed gameplay ticks; `-1` holds until
explicit release or `/api/input/clear`. Supplying both fields is an error. Replies include
`tick` and, for finite tick holds, `releasesAtTick`; `/api/input/state` reports `ticksRemaining`.
An input is pressed once, so holding it across several ticks fires the pressed edge once.

Legacy `frames` holds still count main-loop/render iterations during ordinary running;
their lifetime is frozen while paused or stepping. Use `ticks` or indefinite holds for
controlled tests. Inputs persist across a pause, including a script-triggered pause;
release them explicitly if the next step should not continue the action.

## Timing boundaries and audio

Gameplay updates, scene-safe script callbacks, worker-result delivery, entity updates,
physics, animation and GUI simulation run on fixed ticks. Physics consumes queued commands
before gameplay and completes before the tick can be acknowledged. Commands issued by
that gameplay update are consumed by the next tick. Scene-safe callbacks now run once per
simulation tick rather than once per rendered frame.

Ogre receives the simulation duration advanced by that iteration. Paused inspection renders
receive zero delta. Manual steps render each tick; normal wall-paced catch-up can combine
several ticks into one render. Native frame callbacks and window event handling continue to
run while paused, so developer UI stays responsive. A native plugin gets no per-tick callback:
`Plugin::frameUpdate()` runs once per rendered frame with no delta. A plugin that needs time
should take it from an Ogre frame listener, which receives the simulation delta (zero while
paused).
Custom wall-clock logic, random seeds, worker completion order, external I/O and different
render schedules are not made deterministic. This is fixed-tick execution control, not
cross-platform bit-identical replay. Existing game pause masks continue to apply within ticks.

Paused wall time and time spent waiting for an agent are discarded on resume. Future
recorders must derive timestamps from simulation ticks and submit frames/audio only when
simulation advances; repeated inspection renders must not add paused time to a movie.
Performance FPS and frame-time statistics continue to measure wall time.

Ordinary OpenAL playback is suspended using `ALC_SOFT_pause_device` during pauses and manual
stepping, then resumed during continuous running. Its normal playback speed still follows
the device clock, including at simulation speeds below 1. For sample-counted audio during
stepping or slowdown, launch with **`--simulationAudio`**, which `--headless` already implies:

```sh
./av ~/Project/avSetup.cfg --headless --noDebugger --debugServer --simulationAudio
```

This selects OpenAL Soft loopback at startup, with stereo 48 kHz float samples and **no
speaker output**. Each simulation tick mixes its exact sample count, carrying fractional
samples across ticks. Pauses mix zero samples. This provides the audio clock for
recording. `/api/audio` reports
`simulationClock`; check `available` and `setupError` too. `DisableAudio=true` still disables
audio. Without the device-pause extension, ordinary playback logs a warning and cannot be
frozen; use loopback for controlled tests.

Test-mode wall-clock watchdogs still run during a pause. For interactive agent sessions,
use a project without `TestMode` or configure a suitable watchdog timeout.
