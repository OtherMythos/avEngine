# 0.2.0 — Plugin system

Branch: `0.2.0` in avEngine, avData and avTests. Plugin repositories are on their own `master`.

## Progress

| Stage | State |
|---|---|
| 0 — prove `script://` resolution | done |
| 1 — avPlugin.cfg setup plumbing | done |
| 2 — repoint native loading | done — verified against avEngineIMGUI and PEG |
| 3 — ScriptPluginManager + `_plugin` namespace | done |
| 4 — tests | done — 3 new cases in `integration/PluginTests` |
| 5 — migrate the dialog plugin | done — 19/19 green |
| 6 — migrate consumers | done for avData, avEngineIMGUI and PEG's native half |

## What happened

A plugin is now a **directory containing an `avPlugin.cfg`**, and `avSetup.cfg` lists directories:

```json
"Plugins": [ "res://../avEngineDialogPlugin" ]
```

```json
{
    "Name": "avEngineDialogPlugin",
    "Description": "A dialog system for the avEngine.",
    "Version": "0.1.0",
    "EntryFile": "dialogPlugin.nut",
    "Bin": { "Path": [ "bin" ] }
}
```

`EntryFile` gives the plugin a Squirrel half, `Bin.Path` a native half. Either, both or neither.
Every path inside the cfg is relative to the plugin directory.

**The old `Plugins: [{name, path}]` form is gone.** It only ever found native libraries, and
keeping both would have meant two discovery paths forever.

### Setup happens in two stages

`SystemSetup::_processPlugins` now only *collects* the directory strings into a file-static
intermediate, because the data directory is not final while the setup files are still being read
(a second `avSetupSecondary.cfg` may follow). `_processPluginDirectories`, called from
`_processDataDirectory`, then resolves each directory and reads its `avPlugin.cfg` into a
`SystemSettings::PluginEntry`. This is the same shape `intermediateHlmsLibraries` already used.

Plugin directories accept `res://` and `user://`, which most setup paths do not — the resolution
runs `formatResToPath` before `_findDirectory`.

A broken plugin is reported and skipped, never fatal, and never stops later plugins loading.
`BrokenPluginCfg` pins that with five different kinds of broken.

### Native half

Unchanged below discovery. `PluginManager::_loadDynamicPlugins` skips script-only plugins and
otherwise hands `binPaths` to the existing `_scanToLoadPlugin`, so the arch/platform/build token
scan, `DynLib`, `dllStartPlugin` and `registerPlugin` all behave as before.

### Script half

New `ScriptPluginManager`, modelled on `ScriptingStateManager` but deliberately separate — states
are a user-facing game concept with their own `_scriptingState` API and unique-name registry.
Each plugin's entry file is a `CallbackScript`, so it gets its own private table and any of these
functions it declares are called; all are optional.

| Callback | When |
|---|---|
| `start()` | once, in `Base::_initialise` right after `PluginManager::initialise()` |
| `update()` / `update(dt)` | each fixed update, after `mScriptingStateManager->update()` |
| `sceneSafeUpdate()` | once per rendered frame, while the scene is flagged clean |
| `end()` | in `Base::shutdown`, **before** `ScriptVM::shutdown()` |

That `end()` placement matters: `PluginManager::shutdown()` runs *after* the vm is closed, so a
native plugin's shutdown cannot touch Squirrel. Script plugins get their own pass earlier.

Plugins start in the order they are declared. Loading and starting are two passes, so a plugin's
`start()` can rely on every other plugin being loaded regardless of order. **There is no
dependency resolution between plugins** — deliberately out of scope.

Entry files locate their own files with `script://`, which resolves relative to the running
script, so no plugin needs to be told where it lives.

`_plugin.getPlugins()` and `_plugin.isLoaded(name)` expose the loaded set to script.

### `_system.timeMilliseconds()`

Added while migrating the dialog plugin, which needs a real clock for its `<sleep>` tag. Squirrel
only had `_time()` at second granularity, so the port had been counting down with
`_timer.countdown` — and `TimerManager` is driven by fixed-step deltas. The engine runs **up to
four fixed steps per rendered frame** when catching up, so a delta-driven sleep can elapse several
times faster than the wall clock. That made `AttributeVariablesSleepTag` flaky: it passed twice
and then failed with a 1000ms sleep measuring as 0 seconds elapsed.

The C++ dialog system used `steady_clock` and never had this problem. `timeMilliseconds` is a
monotonic clock measured from its first call, and the plugin now sleeps against it.

## Verification

- 19/19 dialog plugin cases — the strongest signal, since they were written against the C++ dialog
  system and have survived both the Squirrel port and this migration.
- 3 new `integration/PluginTests` cases: lifecycle and ordering, metadata, and broken cfgs.
- `avUnit` 189 tests / 24 suites.
- Native loading verified by hand end to end against **avEngineIMGUI** (token scan picked
  `libAvImguiPlugin_Debug-macos-arm64.so` out of a directory of seven candidates) and **PEG**.
- `avData/simpleDialog` runs its dialog.

## Outstanding

- **ProceduralExplorationGame's dialog migration.** Its native plugin now loads under the new
  system, but it still uses the old `_doFile` dialog convention and the `DialogScript` /
  `DialogConstants` avSetup keys the engine no longer reads. Carried over from
  [dialog-plugin.md](dialog-plugin.md).
- Nothing verifies the native path on Windows or Linux; only the macOS scan was exercised.
- `PluginManager::_getFileExtension()` still returns `.so` on macOS and tests `WIN32` rather than
  `_WIN32`. Pre-existing, and the plugins in use are built to match, so it was left alone.
