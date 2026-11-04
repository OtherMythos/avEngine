# Audio

Inspecting the engine's OpenAL state through the debug server, then streamed playback for
long music.

## Audio inspection

Read-only inspection of the engine's OpenAL playback state and decoded assets. Available
with [`--debugServer`](debug-server.md), including headless runs; no additional flag is needed. API version
**3** adds these endpoints. Trigger sounds through the existing
[input](debug-server-input-and-gui.md) or [eval](debug-server.md#post-apieval) endpoints.

```sh
curl -s localhost:8788/api/audio | jq
curl -s 'localhost:8788/api/audio/sources?state=playing&max=20' | jq
curl -s 'localhost:8788/api/audio/buffers?path=runningWater' | jq
```

`/api/audio` reports `backend`, `enabled` (the inverse of `DisableAudio`), `setup`,
`available`, `device`, `setupError`, `masterGain`, `distanceModel`, listener
`position`/`velocity`/`forward`/`up`, `sourcesByState`, `sourceCount`, `bufferCount`,
`decodedBytes` and `latestSequence`. `outputMeasured` is always false: **playing is an
OpenAL state, not proof that sound reached speakers**. OS mute, disconnected speakers,
the final mixed level and backend resource leaks are not measured. Object counts cover
engine wrappers, not every native OpenAL allocation.

### Sources and buffers

Source and buffer IDs are numeric, stable for each object's lifetime, and never reused
within the process. A destroyed object's detail endpoint returns `404`; its history can
still be read from `/api/audio/events`. Registries do not keep objects alive.

Lists return `sources` or `buffers`, `total` (all matches, including before the cursor),
`truncated` and `nextAfter`. They are ordered by ascending ID. Pass `after=nextAfter`
for the next page. `max` defaults to 50 and caps at 500; zero is invalid. `path` is a
case-sensitive substring of the resolved asset path. Source `state` accepts `initial`,
`playing`, `paused`, `stopped`, `unavailable`, `opening`, `ready`, `buffering`,
`ended` or `failed`; omit it for all states. Invalid parameters
return `400` and normal main-thread timeouts return `503`, using `{"error":"..."}`.

Every response carries `frame` (the debug server's rendered-frame counter, zero during
startup) and `timeSeconds` (monotonic seconds since audio-manager construction). OpenAL
advances independently of rendering, so these mark observation time, not an atomic
sample-accurate snapshot. Headless rendering and accelerated fixed updates do not speed
up audio playback: headless audio follows the simulation clock, which still advances in
real time unless paused or slowed.

Source summaries contain `id`, `bufferId`, `path`, `state`, `offsetSeconds`, `gain`,
`pitch`, `looping`, `relative` and `position`. A source detail adds velocity, direction,
rolloff, reference/max distance, min/max gain, cone angles/gain, `distanceFromListener`,
the buffer detail and `diagnostics`. Positions use engine world units; for relative
sources the distance is the length of their listener-relative position. A source past
`maxDistance` is **not necessarily silent**; that value belongs to the attenuation model.

Unavailable backend fields are `null`, with `unavailableReason`; they are not fabricated
zeroes or a fake stopped state. This also makes `DisableAudio` and failed device setup
safe to inspect.

Buffer summaries contain `ready`, `durationSeconds`, `channels`, `sampleRate`,
`decodedBytes` and `sourceCount`, alongside `id` and `path`. Detail adds `sampleFrames`,
`lastAttemptPath`, `loadError` and `sampleStats`. A failed reload preserves metadata for
the previously loaded asset while reporting the failed attempt separately.

```jsonc
"sampleStats": {
  "scope": "wholeDecodedAsset",
  "peak": [0.72, 0.68],
  "rms": [0.14, 0.13]
}
```

Peak and RMS arrays have one entry per decoded channel. Samples are normalised by 32768
from the 16-bit PCM uploaded to OpenAL; statistics cover the whole asset, **not current
source loudness or mixed output**. They are computed once while decoding, and no extra
PCM copy is retained. Before a successful load these statistics are `null`.

### Diagnosing silence

Start with `/api/audio`, find the source by asset path, then query its detail:

```sh
curl -s 'localhost:8788/api/audio/sources?path=runningWater' | jq
# Use an id returned above:
curl -s localhost:8788/api/audio/source/3 | jq
```

Each diagnostic has a stable `code` and explanatory `evidence`:

| Code | Evidence to check |
|---|---|
| `audioDisabled` | `DisableAudio=true`. |
| `setupFailed` | Device/context setup error. |
| `backendUnavailable` | No inspectable OpenAL source/backend. |
| `missingBuffer` / `bufferNotReady` | No attached asset, or no successful load. |
| `loadFailed` | The most recent load attempt and its error. A previous asset may remain ready. |
| `sourceNotPlaying` | Actual initial, paused or stopped state. |
| `zeroMasterGain` / `zeroSourceGain` | The reported gain is exactly zero. |
| `silentAsset` | Every decoded sample is zero. |

An empty diagnostics array is not an audibility guarantee. Use listener/source positions,
attenuation settings, device information and asset statistics to narrow down the rest.

### Catching a short sound

Read `/api/audio`'s `latestSequence`, trigger the action, then request events after that
cursor. The sound need not still exist:

```sh
cursor=$(curl -s localhost:8788/api/audio | jq -r .latestSequence)
# Trigger the game action using /api/input/* or /api/eval.
curl -s "localhost:8788/api/audio/events?after=$cursor" | jq
```

The latest 1,024 events are retained. Each has `sequence`, `frame`, `timeSeconds`, `type`,
`sourceId`, `bufferId`, `path` and `detail`; IDs not applicable to an event are null.
Events include `setupSucceeded`, `setupFailed`, source/buffer creation and destruction,
`loadSucceeded`, `loadFailed`, `bufferAssigned`, `play`, `pause`, `stop`, `seek`,
`stateObserved` and `operationFailed`.

Commands record requests, not proof of playback. `stateObserved` records the actual
backend state when first observed or changed; natural completion is checked once per
rendered frame for previously playing sources. It does not claim an exact completion
time. Creation may precede buffer assignment; destruction and playback events retain
asset context after the object disappears.

Without `after`, reads return the latest 50 matching events, oldest first. With `after`,
they return subsequent events oldest first. `source=<id>` filters by source, including
expired IDs. `nextAfter` advances over examined nonmatching events, `truncated` signals
another page, `latestSequence` reports the global cursor, and `historyGap=true` means
entries after the requested cursor have already been overwritten. Reads never consume
events. Poll using `nextAfter`; don't reset or clear the history between observations.

## Streamed audio

`_audio.newStream(path)` opens an asset asynchronously on the existing job pool and
plays it through a bounded queue of OpenAL buffers. Use it for long mono/stereo music;
keep `newSoundBuffer` / `newSourceFromBuffer` for shared short effects. WAV works with
the current dependencies. This does not add Ogg codecs or change existing callers.

```squirrel
local music = _audio.newStream("res://music/theme.wav");
music.setVolume(0.6);
music.setLooping(true);
music.play(); //Records intent while the file is opening.
```

The returned source supports the existing volume, pitch, relative/spatial, looping,
play/pause/stop and seek controls. Each stream owns an independent decoder cursor.
`getAudioBuffer()` returns null for a stream. A static buffer cannot be assigned to it.

| Source query | Result |
|---|---|
| `getState()` | `opening`, `ready`, `playing`, `paused`, `stopped`, `buffering`, `ended`, `failed`, or `unavailable`. Static sources support the applicable states too. |
| `getDurationSeconds()` | Asset duration, or null before metadata is available. |
| `getPositionSeconds()` | Consumed position within the asset, accounting for queue reuse, seeking and looping. |
| `getBufferedSeconds()` | Audio already uploaded to the stream's AL queue; zero for static sources. |
| `getError()` | Empty string on success, otherwise the load/decode/backend failure. |

Natural completion is `ended` only after the final queued sample is consumed.
`buffering` is temporary starvation, not completion. A paused or stopped source never
restarts just because a decode job finishes. Stop rewinds; seeking after stop sets the
next play position. Seek is non-negative and finite; past-end seeks clamp to the end.
Loops wrap PCM frames inside the decoder, without padding at buffer boundaries.

Four 8,192-frame PCM blocks and four AL buffers bound storage at about 256 KiB per
stereo stream, plus decoder/backend overhead. The initial two-block prebuffer is an
amount to decode, not a fixed sleep. The full AL queue covers about 683 ms at 48 kHz,
pitch 1. Higher pitches consume it faster.

Decode jobs are short high-priority jobs on the existing worker pool, one outstanding
job per stream. Priority passes queued ordinary work but **does not preempt running
script jobs**. A fully occupied pool can starve audio. The main/context thread pumps AL;
a main-thread stall longer than its queued audio can also cause an underrun. This
version recovers and reports underruns, but does not promise uninterrupted playback
through arbitrary stalls. No extra worker or audio-service threads are created.

Headless/simulation audio refills between small mix slices. In this mode only, each
slice waits until the stream's AL queue is full (or the decoder has reached the end),
so the mixed samples never depend on decoder or scheduler timing. Pitch, Doppler and
resampler look-ahead read far less than the queued blocks. A
five-second decode wait reports a failed stream rather than waiting indefinitely.
Normal device playback does not wait for disk I/O. Global simulation pause freezes
playback; the stream pump can still fill its queues.

The debug source endpoints add `type`, `durationSeconds`, `queuedSeconds`,
`decodedSeconds`, `pcmCapacityBytes`, `underruns` and `error` for streams. They retain
asset paths even without a static buffer, and report `streamBuffering` / `streamFailed`
rather than `missingBuffer`. They do not scan a whole streamed asset for peak/RMS.
Source/buffer counts remain wrapper counts; each stream also owns its four native AL
buffers. Backend objects are now released on destruction and before context shutdown.
Reloading a static buffer is rejected while sources reference it; an unreferenced
buffer can be reloaded, and a failed reload preserves its previous contents.

Streaming runtime regressions live in the separate avTests repository, under
`integration/Audio/StreamingPlayback`, `StreamingErrors` and `StreamingLifetime`.
They are discovered by the normal integration suite. See avTests' README for the
focused `avTestsAudio.cfg` runner command and the 1 Hz long-tick check. Fixtures
are generated by Squirrel under `user://` and cleaned up; no standalone Python
harness or debug server is required.

The unit suites `AudioStreamTest`, `StreamOpenALTest`, `AudioSamples`,
`AudioLifetime` and `JobDispatcherTests` cover decoder samples, bounded queues,
controls, pool saturation, priority, cancellation, backend lifetime,
static/streamed sample equivalence and stream inspector JSON. The normal avTests
integration suite is still required after the final rebuild.
