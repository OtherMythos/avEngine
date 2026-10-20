# avEngine

#### A data-driven, cross-platform 3D game engine
[![Build Linux](https://github.com/OtherMythos/avEngine/actions/workflows/buildLinux.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildLinux.yml)
[![Build Windows](https://github.com/OtherMythos/avEngine/actions/workflows/buildWindows.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildWindows.yml)
[![Build MacOS](https://github.com/OtherMythos/avEngine/actions/workflows/buildMacOS.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildMacOS.yml)
[![Build Android](https://github.com/OtherMythos/avEngine/actions/workflows/buildAndroid.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildAndroid.yml)
[![Build iOS](https://github.com/OtherMythos/avEngine/actions/workflows/buildIos.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildIos.yml)

The avEngine is a modern, general purpose 3D game engine.
The engine is written with an emphasis on data driven content, meaning multiple projects can be built from the same engine executable, without any need to re-compile.

Projects using the avEngine are written using the programming language Squirrel.
This language is easy to learn and allows rapid creation of projects.
With it practically any sort of project can be created, even besides games.
For instance, a number of the editor tools created for the engine are built ontop of the engine using Squirrel.

Creation of a project is easy, and the script api is simple:


```js
function start(){
    //Create a global variable for animation.
    ::animCount <- 0.0;

    //Create a mesh object.
    ::myMesh <- _mesh.create("cube");

    //Point the camera so it can see the mesh
    _camera.setPosition(Vec3(0, 0, 100));
    _camera.lookAt(0, 0, 0);
}

//Called each fixed update.
function update(){
    animCount += 0.01;

    local value = cos(animCount);
    myMesh.setScale(value, value, value);
}
```

### Features

A complete list of major features of the engine include:

 * Realtime scene based 3D including lighting, models and on demand shader generation.
 * An embedded scripting language allowing scripting for projects.
 * A plugin system for both native and script code.
 * Scene files for loading authored content into the scene.
 * Dynamic physics and collision detection using Bullet, plus lightweight octree collision worlds.
 * 3D spatial audio.
 * Controller and input support based around actions.
 * Entity component system.
 * 3D heightmap terrain.
 * GUI system with layout, widgets and support for a variety of writing systems.
 * Support for Lottie animations.
 * Easy 2D texture drawing.
 * Script workers, for running Squirrel on background threads.
 * Developer tools such as debug drawing, Squirrel debugging and a script profiler.
 * A localhost debug server for inspecting and driving a running engine, with headless rendering, simulation control and video capture.
 * Data driven design with an emphasis on no need to recompile the engine.
 * Portable, docker based asset generation pipeline.
 * Lots of automated testing!

Version 0.2.0 thinned the engine down towards a framework. It removed the streaming world system and slot positions, the maps directory, serialisation and the save directory, the timer namespace and the nav mesh system. The dialog system now lives in the [avEngineDialog](https://github.com/OtherMythos/avEngineDialog) plugin.

The engine supports the following platforms:

 * Windows
 * MacOS
 * Linux
 * iOS
 * Android

### Dependencies

The engine uses some common libraries to provide certain functionality.
These include:

 * [Ogre-next 3.0](https://github.com/OGRECave/ogre-next)
 * [Bullet Physics](https://github.com/bulletphysics/bullet3)
 * [Squirrel](https://github.com/albertodemichelis/squirrel)
 * [EntityX](https://github.com/alecthomas/entityx)
 * [ColibriGUI](https://github.com/darksylinc/colibrigui/)
 * [OpenAL-soft](https://github.com/kcat/openal-soft)
 * [libsndfile](https://github.com/libsndfile/libsndfile)
 * [SDL2](https://github.com/libsdl-org/SDL)
 * [rapidjson](https://github.com/Tencent/rapidjson)
 * [tinyXML2](https://github.com/leethomason/tinyxml2)
 * [RLottie](https://github.com/Samsung/rlottie)
 * [cpp-httplib](https://github.com/yhirose/cpp-httplib) and [FFmpeg](https://ffmpeg.org), for the desktop debug server

### Planned features

Features planned for eventual implementation into the engine include:

 * Behaviour tree implementation
 * Easy support for localisation

## Building

Build scripts to build the engine's dependencies are maintained in this repository:

http://github.com/OtherMythos/avBuild

These scripts produce an avBuilt directory which contains all the built dependencies.

From this, building the engine can be done simply with:

```bash
mkdir build
cd build
cmake -DAV_LIBS_DIR=~/avBuilt/Debug ..
```

Desktop builds compile in the debug server by default, which needs the FFmpeg libraries in avBuilt.
Pass `-DDEBUG_SERVER=OFF` to build without it.

## Documentation
Documentation for the engine is hosted here:

https://avdocumentation.readthedocs.io

## Testing
The engine has been built with a heavy emphasis on testing and test automation.
Infact, the tests that have been written are written entirely using Squirrel.
You can find a copy of the test scripts here:

http://github.com/OtherMythos/avTests
