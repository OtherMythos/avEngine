# avEngine

#### A data-driven, cross-platform 3D game engine
[![Build Linux](https://github.com/OtherMythos/avEngine/actions/workflows/buildLinux.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildLinux.yml)
[![Build Windows](https://github.com/OtherMythos/avEngine/actions/workflows/buildWindows.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildWindows.yml)
[![Build MacOS](https://github.com/OtherMythos/avEngine/actions/workflows/buildMacOS.yml/badge.svg)](https://github.com/OtherMythos/avEngine/actions/workflows/buildMacOS.yml)

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

//Called each frame.
function update(){
    animCount += 0.01;

    local value = cos(animCount);
    myMesh.setScale(value, value, value);
}
```

More examples can be found in the [avData](http://gitlab.com/edherbert/avData) repository.

### Features

A complete list of major features of the engine include:

 * Realtime scene based 3D including lighting, models and on demand shader generation.
 * An embedded scripting language allowing scripting for projects.
 * Multi-threaded implementation of dynamic physics and collision detection.
 * Support for a large, streamable world baked into the engine design.
 * 3D spatial audio.
 * Controller and input support based around actions.
 * Entity component system.
 * 3D heightmap terrain.
 * Powerful and versatile dialog system, built using a custom, easy editable dialog script format.
 * GUI system with layout, widgets and support for a variety of writing systems.
 * Support for Lottie animations.
 * Easy 2D texture drawing.
 * Efficient path finding on nav meshes.
 * Support for custom native plugins, with methods to load both dynamically and statically.
 * Developer tools such as debug drawing and squirrel debugging .
 * A wide variety of tools including a custom made level editor called [Southsea](http://gitlab.com/edherbert/Southsea).
 * Data driven design with an emphasis on no need to recompile the engine.
 * Portable, docker based asset generation pipeline.
 * Lots of automated testing!

The engine supports the following platforms:

 * Windows
 * MacOS
 * Linux
 * iOS

### Dependencies

The engine uses some common libraries to provide certain functionality.
These include:

 * [Ogre-next 2.3](https://github.com/OGRECave/ogre-next)
 * [Bullet Physics](https://github.com/bulletphysics/bullet3)
 * [Squirrel](https://github.com/albertodemichelis/squirrel)
 * [EntityX](https://github.com/alecthomas/entityx)
 * [ColibriGUI](https://github.com/darksylinc/colibrigui/)
 * [OpenAL-soft](https://github.com/kcat/openal-soft)
 * [libsndfile](https://github.com/libsndfile/libsndfile)
 * [Detour](https://github.com/recastnavigation/recastnavigation)
 * [SDL2](https://github.com/libsdl-org/SDL)
 * [rapidjson](https://github.com/Tencent/rapidjson)
 * [tinyXML2](https://github.com/leethomason/tinyxml2)
 * [RLottie](https://github.com/Samsung/rlottie)

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
cmake -DAV_LIBS_DIR=~/avBuilt/builtType ..
```

## Documentation
Documentation for the engine is hosted here:

https://avdocumentation.readthedocs.io

## Testing
The engine has been built with a heavy emphasis on testing and test automation.
Infact, the tests that have been written are written entirely using Squirrel.
You can find a copy of the test scripts here:

http://github.com/OtherMythos/avTests
