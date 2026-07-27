# 0.2.0 — Dialog system to a Squirrel plugin

Branch: `0.2.0` in avEngine, avData and avTests. The plugin is its own repository at
`~/Documents/repo/avEngineDialogPlugin`, on `master`.

## Progress

| Stage | State |
|---|---|
| 0 — plugin repo skeleton | done |
| 1 — Squirrel compiler | done |
| 2 — runtime: registry, executor, facade | done |
| 3 — port the tests out of avTests | done — 19 cases |
| 4 — verify against the unmodified engine | done — 19/19 |
| 5 — strip the C++ out of the engine | done — `avUnit` 189 tests / 24 suites |
| 6 — re-verify and port avData | done — 19/19, both avData projects ported |
| 7 — ProceduralExplorationGame | **not started** |

## What happened

The engine no longer provides dialog. `src/Dialog/` (~2,900 lines) and
`DialogSystemNamespace` are gone, along with the `DialogScript` and `DialogConstants`
keys in `avSetup.cfg`. A project that wants dialog now does:

```squirrel
::DIALOG_PLUGIN_PATH <- "res://../../repo/avEngineDialogPlugin";
_doFile(::DIALOG_PLUGIN_PATH + "/dialogPlugin.nut");
_dialogSystem.setImplementation("res://dialogImplementation.nut");
```

and calls `_dialogSystem.update()` from its own `update()`.

The `.dialog` file format is unchanged. The plugin parses the same XML through the
engine's existing `XMLDocument` / `XMLElement` Squirrel bindings, which is what made
keeping every existing asset and test case viable.

`GlobalRegistryNamespace` was collapsed while doing this. It carried a `(HSQUIRRELVM, bool)`
overload of every function purely so the dialog namespace could point the same code at a
second `ValueRegistry`. With the dialog local registry now living in Squirrel, `_registry`
only ever talks to the global one.

## Verification

The 18 avTests dialog cases moved into the plugin repo, plus one new case
(`DialogStringVariableErrors`) replacing the C++ unit tests that covered
`_scanStringForVariables`. They were run green **against the unmodified engine first** —
the plugin's `::_dialogSystem` shadows the engine's root table slot, so a green run there
is a real behavioural comparison and not just a green run of new code. Then green again
after the C++ was deleted.

```bash
python3 ~/Documents/avTools/testRunner/testRunner.py -p ~/Documents/repo/avEngineDialogPlugin/test/avTests.cfg -e ~/Documents/avEngine/build/Debug/av.app/Contents/MacOS/av
```

`avData/simpleDialog` runs and plays its dialog. `avData/characterController` is ported
but could not be exercised end to end on this machine: its `OgreResources.cfg` points at
`/home/edward/Documents/controllerAssetsOut`, a Linux path, so it dies on a missing
`baseMesh.mesh` long before any dialog is triggered. That failure is pre-existing and
unrelated. The plugin itself loads cleanly in it.

## Outstanding

- **ProceduralExplorationGame is broken until it is ported.** It is the only heavy
  consumer: `src/DialogScript.nut`, `src/System/Dialog/DialogManager.nut`,
  `DialogMetaScanner.nut`, `"DialogScript"` and `"DialogConstants"` in its `avSetup.cfg`,
  and it reads `_dialogSystem.registry.getKeys()` to build its actor name map. The
  migration is mechanical — the three table rows in the plugin README cover all of it —
  but it touches a live project.
- **`XMLDocument()` leaks.** `XMLDocumentUserData::createXMLDocument` news a
  `tinyxml2::XMLDocument` and never sets a release hook, so every dialog compile leaks one
  document. Pre-existing, and the scene editor hits it too. Fixing it needs care because
  `XMLElement` userdata holds raw pointers into the document, so a release hook could
  leave those dangling.
- ~~The plugin's `_doFile` loading convention is a placeholder for the real plugin system.~~
  Done — see [plugin-system.md](plugin-system.md). The plugin now loads from an `avPlugin.cfg`,
  pumps its own `update()`, and the `DIALOG_PLUGIN_PATH` global is gone.

## Behaviour deliberately preserved

The port reproduces a number of engine quirks rather than fixing them, because content
depends on them — `dialogString`'s actor id is always `1`, `<hideDialogWindow/>` fires
`shouldCloseDialogWindow`, `@constants@` in body text are not substituted, block level
`<set>` writes the global registry, option text is never interpolated. The full list, and
the short list of places the port deliberately differs, is in the plugin's README.
