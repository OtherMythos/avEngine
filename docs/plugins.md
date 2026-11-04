# Plugins

A plugin is a **directory containing an `avPlugin.cfg`**. A project lists plugin directories in
its `avSetup.cfg`, and both `res://` and `user://` work there:

```json
"Plugins": [ "res://plugins/avImguiPlugin", "res://plugins/avDialogPlugin" ]
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

| Key | Required | Meaning |
|---|---|---|
| `Name` | yes | Identifies the plugin. A native library's filename must start with it. |
| `Description`, `Version` | no | Metadata, reported by `_plugin.getPlugins()`. |
| `EntryFile` | no | The Squirrel half. |
| `Bin.Path` | no | The native half: a string or an array of candidates, each a directory to scan or a direct path to a library. |

Paths inside `avPlugin.cfg` are relative to the plugin directory. A broken plugin (a missing
directory, a missing or invalid cfg, no `Name`, an `EntryFile` that doesn't exist) is logged and
skipped, never fatal. Plugins load in the order listed, and there is no dependency resolution
between them. Scripts can query `_plugin.getPlugins()` and `_plugin.isLoaded(name)`.
`avTests/integration/PluginTests` covers the Squirrel half and broken cfgs; nothing in the suite
loads a native plugin.

## The Squirrel half

The entry file is a callback script with its own table. Every callback is optional:

| Callback | When |
|---|---|
| `start()` | Once at startup, after every plugin has been loaded, native halves included, so it can use a native plugin's namespace. |
| `update()` / `update(dt)` | Every fixed tick. |
| `sceneSafeUpdate()` | Every fixed tick, while the scene is safe to modify. |
| `end()` | At shutdown, before the Squirrel VM closes. |

An entry file finds its own files with `script://`, which resolves relative to the running script.

## The native half

The library is named `lib<Name>_<Debug|Release>-<platform>-<arch>.so`, or `.dll` on Windows.
macOS also uses `.so`. The platform is `macos`, `linux`, `windows` or `ios`, and the arch is
`arm64` on Apple silicon and `x86_64` everywhere else. The first matching file in `Bin.Path` wins.
The engine opens it and calls an exported `dllStartPlugin()`, which registers a subclass of
`AV::Plugin`:

```cpp
extern "C" DLLEXPORT void dllStartPlugin(void){
    AV::PluginManager::registerPlugin(new MyPlugin());
}
```

| `AV::Plugin` virtual | When |
|---|---|
| `initialise()` | Inside `registerPlugin`, at startup. Register Squirrel namespaces here with `AV::ScriptVM::setupNamespace("_name", setupFunc)`. |
| `frameUpdate()` | Once per rendered frame, before input is dispatched. Not per tick, and it gets no delta. |
| `earlyShutdown()` | At shutdown, after the script plugins' `end()`, while the VM, input, GUI and Ogre are still alive. |
| `shutdown()` | After the Squirrel VM has closed. |

Plugins initialise after Ogre setup, resource script parsing and compositor pass provider
registration. So scripts parsed at startup can't refer to a plugin's custom compositor passes, and
a plugin has to add its own resource locations from code (`_resources.addResourceLocation` in
`start()`, for example). On iOS and Android, `USE_STATIC_PLUGINS` compiles the native halves in
instead: `AV_PROJECT_DIR` provides a `StaticPlugins.h` defining `registerStaticPlugins()`, which
calls `REGISTER_PLUGIN("Name", Namespace::PluginClass)` for each one.

## Native plugin traps

- **Nothing checks that a native plugin matches the engine.** `dllStartPlugin` takes no version,
  and `avPlugin.cfg` has no engine version field. Rebuild a native plugin whenever
  `src/System/Plugins/Plugin.h`, or any engine header it uses, changes. **Failure signature:** the
  plugin loads and works, then the engine crashes on exit inside `PluginManager::earlyShutdown()`.
  That's a segfault on Linux and `0xC0000005` on Windows, and it means the binary predates
  `earlyShutdown()` (added 26 Aug 2026).
- **A plugin can only call what the engine exports, and that differs by platform.** The engine
  exports a deliberate plugin surface: its own API, the whole Squirrel API (`sq_*` and `sqstd_*`)
  and SDL.

  | Platform | Engine API | Squirrel | SDL | Anything else statically linked (Bullet, Colibri, OpenAL) |
  |---|---|---|---|---|
  | macOS | `av` exports every global | all of it | yes | yes, by accident of Mach-O exporting everything |
  | Linux | `libavCore.so` exports all of it | `av`, through `CMake/pluginExports.list` | `av`, same list | only what the engine happens to reference |
  | Windows | `avCore.lib`, functions only | `avCore.lib`, through `CMake/GeneratePluginExports.cmake` | SDL is a DLL, so link `SDL2d.lib`/`SDL2.lib` | no |

  How a plugin links: on macOS with `-undefined dynamic_lookup`; on Linux with its undefined
  symbols left for `dlopen` to resolve; on Windows against `avCore.lib`. A Windows plugin should
  not link its own copy of Squirrel any more. Older plugins do, and it only works while they're
  built from the same avBuilt and use the same C runtime as the engine.

  - Because macOS exports everything, a plugin calling Bullet, Colibri or OpenAL internals works
    there and fails to load on Linux (`undefined symbol`) or to link on Windows (`LNK2019`).
  - On Windows, `CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS` exports functions but not static data, so an
    inline getter over a static member fails to link.
  - To widen the surface, add a pattern to `CMake/pluginExports.list` for Linux and to the regex in
    `CMake/GeneratePluginExports.cmake` for Windows. Exporting Squirrel and SDL costs the Linux binary
    about 50 KB; exporting everything would cost about 2.7 MB.

  Check a plugin's undefined symbols before shipping it: `nm -u <plugin>.so` on Linux, or link it on
  Windows.
