# Debug server: input and GUI

The [debug server](debug-server.md) endpoints for driving a running engine: injecting input,
and reading the Colibri GUI to find what to click. GUI positions, render captures and mouse
input share one normalised 0–1 coordinate space.

Under `--headless`, a hold counted in `frames` is far shorter than it looks (see
[headless.md](headless.md)). Holds can be counted in gameplay `ticks` instead, which is what
paused and stepped runs need; see [simulation-control.md](simulation-control.md#input-lifetimes).

## `GET /api/input/actions`

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

## `POST /api/input/action`

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

## `POST /api/input/mouse`

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

## `POST /api/input/clear`

Releases everything currently being spoofed — the panic button. **Mutating.**

```sh
curl -s -X POST localhost:8788/api/input/clear
# {"ok":true,"frame":903}
```

## `GET /api/input/state`

Reports what the debug server is currently spoofing. Read-only.

```jsonc
{
  "frame": 905,
  "active": [
    { "input": "button:Accept", "framesRemaining": -1 }   // -1 = held indefinitely
  ]
}
```

## `GET /api/input/layers`

The input layer stack, ordered from the layer offered input first to the layer offered
it last. Read-only. This is how you find out **who swallowed a click** when an injected
press appears to do nothing.

Input is arbitrated by the `InputRouter` (`src/Input/InputRouter.h`): each event is
offered down the stack, and a layer which consumes it denies it to everything below.
The engine registers three layers of its own — `engineHotkeys` (F1/F2), `gui` (Colibri)
and `game` (the `InputManager` that scripts poll) — and native plugins register their
own, such as the imgui plugin's overlay layer.

```jsonc
{
  "layers": [
    { "name": "engineHotkeys", "priority": 300, "enabled": true, "live": true,
      "external": false, "ownsPointer": false, "ownedButtons": 0, "ownedKeys": 0 },
    { "name": "imgui", "priority": 200, "enabled": true, "live": true,
      "external": true, "ownsPointer": true, "ownedButtons": 1, "ownedKeys": 0 },
    { "name": "gui", "priority": 100, /* ... */ },
    { "name": "game", "priority": 0, /* ... */ }
  ],
  "pluginInputEnabled": true,
  "guiConsumesInput": false
}
```

- `live` — whether the layer is in a position to answer at all. A plugin layer whose
  system has stopped drawing reports `false` and is skipped entirely, so stale capture
  can never latch on and swallow input forever.
- `ownedButtons` / `ownedKeys` — a layer which consumes a press **owns** it until the
  release, which is then delivered only to that layer. If an injected press seems to
  vanish, look for a non-zero `ownedButtons` above `game`.
- `guiConsumesInput` is `false` by default: Colibri reports what it intersected but does
  not block, which is the engine's long-standing behaviour. Opt in with
  `_input.setGuiConsumesInput(true)`.
- `pluginInputEnabled` mirrors `_input.setPluginInputEnabled(bool)` — the escape hatch
  for ruling a misbehaving plugin layer out without unloading it.

Mouse **position** is never consumed, only buttons, wheel and keys — so `getMouseX/Y`
stays truthful no matter which layer owns the pointer.

## `GET /api/gui/tree`

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

## `GET /api/gui/labels`

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

## `GET /api/gui/at?x=<f>&y=<f>`

Which widgets contain a normalised (0–1) point, outermost first — "if I click here, what
do I hit?" Pairs with a render capture: spot a control in the pixels, confirm what it is,
then click its centre via `/api/input/mouse`.

```jsonc
{ "point": [0.042, 0.075],
  "hits": [ {"id": 0, "type": "Window"},
            {"id": 3, "type": "Button", "text": "Press here"} ] }
```

## `GET /api/gui/widget/<id>`

Single-widget deep dive: geometry, state, text, `parentId`, and registered `childIds`.
404 if the id is unknown or stale.
