# Building avEngine

## How the build is laid out

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

## Building per host OS

The build tree is generator-specific. The commands below assume you're inside the build
directory.

- **macOS** (Xcode generator):
  ```sh
  # Either the native tool (best for the grep-filtered quick check in AGENTS.md):
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

## When you need to re-run CMake

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

## Build options

Toggle these with `-D<OPTION>=ON|OFF` at configure time:

| Option | Default | Purpose |
|---|---|---|
| `BUILD_UNIT_TESTS` | `ON` | Build the `avUnit` GoogleTest target. |
| `DEBUG_SERVER` | `ON` (desktop) | Compile in the [agent debug server](debug-server.md). Excluded on iOS/Android. Needs FFmpeg in `AV_LIBS_DIR` (`find_package(AVFFmpeg REQUIRED)`), and adds about 13 MB to a macOS Debug binary. |
| `SCRIPT_PROFILER` | `ON` | Compile in the [Squirrel script profiler](script-profiler.md). Available on every platform. |
| `TEST_MODE` | `ON` | Engine testing capabilities. |
| `DEBUGGING_TOOLS` | `ON` | Developer tools (debug draw, Squirrel debugging). |
| `USE_STATIC_PLUGINS` | `OFF` | Compile a `StaticPlugins.h` from `AV_PROJECT_DIR` (required for iOS/Android). |
| `ENABLE_ADMOB` | `OFF` | AdMob support behind `_monetisation`. Only an iOS implementation exists. |
| `ENABLE_MICROTRANSACTIONS` | `OFF` | StoreKit in-app purchases behind `_monetisation`. Only an iOS implementation exists. |

The defaults don't depend on the build type, so CI's Release builds carry the debug server, test
mode and debugging tools too. The debug server and profiler stay inactive until `--debugServer` or
`--profileScripts` is passed.
