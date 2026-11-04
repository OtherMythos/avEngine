# Video capture

API version **5** records video directly to MP4 with the statically linked FFmpeg
libraries. Encoding uses VideoToolbox on macOS and OpenH264 elsewhere, with a
60 Mbps target, H.264 High profile and an sRGB transfer tag. Downscaling uses bicubic
filtering. Start the engine with [`--debugServer`](debug-server.md), then send JSON:

```sh
curl -s -X POST localhost:8788/api/video/start \
  -d '{"outputDir":"/tmp/av-take-01","fps":60,"frameLimit":120,"width":320,"height":180}'
curl -s localhost:8788/api/video/status
# The finished file is /tmp/av-take-01/video.mp4
```

`outputDir` must be an absolute path to a **new** directory. The recorder reserves it
on start and never overwrites an existing take. `fps` defaults to 60 and must divide
the engine's fixed update rate exactly. For 24 fps, launch with
`--fixedUpdateRate 120 --physicsUpdateRate 120`. `width` and `height` are optional,
must be supplied together, must be even and at most 8192, and can only reduce the
rendered size. Without them the native render size must be even. `frameLimit` is an
optional positive image count; the recorder stops automatically after that many images.
To end a take manually, `POST /api/video/stop`, then poll status until `stopped` or
`failed`. An automatic limit ends in `completed`. A `draining` state means buffered
images are still being encoded. Status reports `framesCaptured`, `framesEncoded`,
`queuedFrames`, `audioSampleFrames`, `outputPath` and `lastError`.
If a paused 30 fps take has advanced only one of its next two 60 Hz ticks,
manual stop returns `409` with the number of ticks needed to finish that image.
Step those ticks, then stop; no completed simulation tick is silently omitted.

Recording grants exactly one output frame's worth of simulation ticks before each
image. Image zero follows the first `1/fps` of simulation and has MP4 presentation
time zero; a take of N images lasts exactly N/fps seconds. Script and agent pauses
can last any wall time. Inspection renders during a pause add no frames. Steps record
an image whenever they complete the next group of ticks; at 30 fps with a 60 Hz
simulation, each image follows two ticks. Slow GPU readback or encoding fills a
two-image queue and blocks further simulation, keeping the movie complete. Returning
to normal play after a take discards wall time spent encoding. The simulation speed
setting does not affect offline capture; pause and step still work normally.

For the engine's OpenAL mix, launch with `--simulationAudio` (implied by `--headless`) and set
`"audio":"engine"` in `/api/video/start`. This records 48 kHz stereo AAC in the
MP4. `"audio":"none"` is the default. Engine audio requires `DisableAudio=false`;
an unavailable simulation mixer rejects the recording request. Simulation pauses
add no audio, and each captured image carries the sound mixed during its ticks.
The FFmpeg build is a desktop debug dependency; mobile builds do not include it.
