# avEngine

#### A cross-platform 3d game engine
The avEngine is a modern, cross platform, general purpose 3D game engine.
It includes many common required features such as powerful multiplatform 3D rendering, dynamic physics, an entity component system, collision detection, controller input and an embedded scripting language.
The engine is written with an emphasis on data driven content, meaning multiple projects can be built from the same engine executable, without any need to re-compile.

The avEngine allows its users to use the scripting language Squirrel to create project code.
This embedded scripting language has a clean and familiar syntax, and fits into a tight memory profile.
With it practically any sort of project can be created, even besides games.
For instance, a number of the editor tools created for the engine are built ontop of the engine using Squirrel.

Creation of a project is easy, and the script api is simple yet complete.

```js
function start(){
    //Create a global variable for animation.
    ::animCount <- 0.0;

    //Create a mesh for animation.
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

A complete list of major features of the engine includes:

 * Realtime scene based 3D including lighting, models and realtime shader generation.
 * An embedded scripting language incorporated into every other feature of the engine.
 * Threaded implementation of dynamic physics.
 * Support for a large streamable world baked into the engine design.
 * Sophisticated collision detection system.
 * Controller and input support based around actions.
 * Entity component system.
 * 3D heightmap terrain.
 * Powerful and versatile dialog system, built using a custom, easy editable dialog script format.
 * GUI system with layout, widgets and support for different writing methods such as Arabic.
 * Developer tools such as debug drawing and squirrel debugging.
 * Easy 2D texture drawing.
 * A wide variety of tools including a custom made level editor called [Southsea](http://gitlab.com/edherbert/Southsea).
 * Cross platform support on Windows, MacOS and Linux.
 * Data driven design with an emphasis on no need to recompile the engine.
 * Lots of automated testing!

### Dependencies

The engine uses some common libraries to provide some functionality.
These include:

 * Ogre3D 2.2
 * Bullet Physics
 * Squirrel
 * EntityX
 * ColibriGUI
 * rapidjson
 * tinyXML2

### Planned features

Features planned for eventual implementation into the engine include:

 * Nav meshes and path finding
 * Behaviour tree implementation
 * 3D spacial audio with OpenAL.
 * Easy support for localisation

#### Documentation
Documentation for the engine is hosted here:

https://avdocumentation.readthedocs.io

#### Testing
The engine has been built with a heavy emphasis on testing and test automation.
Infact, the tests that have been written are written entirely using Squirrel.
You can find a copy of the test scripts here:

http://gitlab.com/edherbert/avTests

#### Development Progress
The engine is still very much in development. My progress is outlined here:
https://trello.com/b/WyJv4Tb4/avengine
