# 0.2.0 — Removing the World system

Branch: `0.2.0`. Nothing here lands on `master`.

## Why

The world system was built for one game: a large, chunked, streaming world with a floating
origin, loaded from a `maps` directory of pre-baked recipes and serialised into saves. No
current game uses it that way. What games actually do now is call `_scene.insertSceneFile()`,
build their content in Squirrel, and manage their own spatial logic.

What's left is a large amount of engine binary implementing a content model nobody targets,
plus a coordinate type (`SlotPosition`) that every other subsystem has to speak because the
world demanded it.

## Scope of this document

The removal of the world system only. Physics, dialog and UserSettings are separate 0.2.0
work items with their own plans. Serialisation is folded in here, because once the world is
gone roughly 90% of the serialisation code has nothing left to serialise.

## What `src/World/` actually contains

149 files, ~17.7k lines. Most of it is not the world:

| Path | Files | Lines | Fate |
|---|---:|---:|---|
| `World.{cpp,h}`, `WorldSingleton.{cpp,h}` | 4 | 469 | **Delete** |
| `Slot/` (SlotManager, ChunkRadiusLoader, ChunkCoordinate, SlotPosition) | 8 | 1743 | **Delete** |
| `Slot/Chunk/` (Chunk, ChunkFactory, TerrainManager) | 6 | 725 | **Delete** |
| `Slot/Chunk/Terrain/Terrain.{cpp,h}` (chunk-tied) | 2 | 319 | **Delete** |
| `Slot/Recipe/` (SceneParser, CollisionObjectSceneParser, *RecipeData) | 8 | 667 | **Delete** |
| `Serialisation/` (MeshSerialisationBuilder) | 3 | 115 | **Delete** |
| `Entity/Tracker/` (chunk-based entity tracking) | 4 | 270 | **Delete** |
| | **35** | **4308** | **deleted** |
| `Slot/Recipe/AvScene/` | 8 | 695 | → `src/Scene/AvScene/` |
| `Slot/Chunk/Terrain/TerrainObject.{cpp,h}` + `terra/` | 15 | 4390 | → `src/Terrain/` |
| `Entity/` (minus Tracker) | 46 | 2273 | → `src/Entity/` |
| `Physics/` | 21 | 2798 | → `src/Physics/` |
| `Nav/` | 8 | 854 | → `src/Nav/` |
| `Support/` (OgreMeshManager, ProgrammaticMeshGenerator, InternalTextureManager, Obj) | 10 | 1482 | → `src/Mesh/` |
| `Developer/` (DebugDrawer, MeshVisualiser) | 6 | 889 | → `src/Developer/` |

After this, `src/World/` no longer exists.

Deleted engine code outside `src/World/`:

- `src/Serialisation/` — SaveHandle, SerialisationManager, SerialiserStringStore
- `src/Threading/Jobs/Recipe{Scene,PhysicsBodies,NavMesh,DataPoint,CollisionObjects,ChunkMeta}Job`
- `src/Threading/Jobs/Entity{,De}SerialisationJob`
- `src/Event/Events/WorldEvent.h` and the `EventType::World` family
- `src/Scripting/ScriptNamespace/{World,SlotManager,Serialisation}Namespace`
- `src/Scripting/ScriptNamespace/Classes/{SlotPositionClass,SaveHandleClass}`
- `src/Scripting/ScriptNamespace/TestNamespace/TestModeSlotManagerNamespace`

## Design decisions

**Manager lifetime.** `World` currently owns `EntityManager`, `PhysicsManager`,
`NavMeshManager`, `SlotManager` and `ChunkRadiusLoader`. The three survivors become
engine-lifetime, created by `Base` at startup and destroyed at shutdown, reachable through
`BaseSingleton` like every other manager. There is no more "the world is not ready yet"
state — that existed only because deserialisation was threaded.

**`SlotPosition` → `Ogre::Vector3`.** `SlotPosition` exists to support origin shifting for a
world larger than float precision comfortably allows. With no chunk streaming there is no
origin to shift. Every signature taking a `SlotPosition` takes a `Vector3`, and
`WorldSingleton::getOrigin()` and its shift event disappear.

**Entity system survives, decoupled.** `World/Entity/` moves to `src/Entity/` with
engine lifetime and `Vector3` positions. The chunk-based `EntityTracker` goes — it existed
to keep entities alive across chunk load/unload boundaries.

**Terrain becomes standalone.** Only `TerrainObject` (the `_scene.createTerrain()` path) and
the vendored `terra` Ogre component survive, as `src/Terrain/`. `Terrain` and
`TerrainManager` are a per-chunk pool and go with the chunks.

**AvScene is the survivor of the scene story.** `Slot/Recipe/AvScene/` is already
slot-independent and is where scene loading is heading; it just happens to be filed under a
directory that is about to be deleted. It moves to `src/Scene/AvScene/`. The older
recipe/`SceneParser` format — the thing `maps/` directories contain — goes.

## Stages

Each stage compiles and passes `avUnit` on its own. Don't start the next until the previous
is green, and keep them as separate commits — stage 1 is a huge mechanical diff and mixing
it with logic changes makes the rest unreviewable.

### Stage 0 — baseline

Recorded on macOS Debug, so "thinner binary" is measurable rather than asserted:

| Metric | Baseline |
|---|---|
| `build/Debug/av.app/Contents/MacOS/av` | 34,804,528 bytes |
| `avUnit` | 284 tests, 36 suites, all passing |
| `src/**/*.{cpp,h}` | 76,570 lines |

### Stage 1 — relocate the innocents

Pure file moves plus include-path rewrites. No logic changes, no deletions.

```
World/Support/            → src/Mesh/
World/Developer/          → src/Developer/
World/Nav/                → src/Nav/
World/Physics/            → src/Physics/
World/Entity/             → src/Entity/
World/Slot/Recipe/AvScene/→ src/Scene/AvScene/
World/Slot/Chunk/Terrain/{TerrainObject.*,terra/} → src/Terrain/
```

Mirror the same moves under `test/unit/src/`.

Notes:

- `CMakeLists.txt` uses `file(GLOB_RECURSE srcs "src/*.cpp")`, so no source lists to edit —
  but a CMake **re-configure** is required, not just a rebuild.
- The five `src/System/OgreSetup/*.h` files include
  `World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h`; that path changes.
- ~80 files outside `src/World/` include a `World/` header. Rewrite mechanically, then
  confirm no `#include "World/` remains except for the genuine world headers.

At the end of this stage `src/World/` holds only `World`, `WorldSingleton`, `Slot/`
(minus AvScene) and `Serialisation/`.

### Stage 2 — kill the origin shift and `SlotPosition`

The hardest stage, and the one that breaks game scripts. Engine side:

- Delete `WorldSingleton::_origin`, `SlotManager::setOrigin`, `WorldEventOriginChange`.
- Physics: delete `DynamicsWorldThreadLogic::_performOriginShift`, and
  `PhysicsWorldThreadLogic::worldOriginChange{Offset,NewPosition}`. Physics itself is a later
  work item, but it can't keep speaking `SlotPosition` after this stage.
- `EntityManager::createEntity/setEntityPosition` and `PositionComponent` take `Vector3`.
- `SerialisationManager`, `SystemEventListenerObjects` — dealt with in stage 4, just note
  they still reference these types.

Script side — this is the break avData and avTests feel:

| Removed | Replacement |
|---|---|
| `SlotPosition(...)` class and constructor | `Vec3(...)` |
| `_slotManager.setOrigin` / `getOrigin` | none — no origin exists |
| `_slotManager.setCurrentMap` | none — see stage 3 |
| `_world.setPlayerPosition` / `getPlayerPosition` | game scripts track this themselves |
| `_world.setPlayerLoadRadius` / `getPlayerLoadRadius` | none |
| `_scene.testRayForSlot` | `_scene.testRayForObject`, or a `Vec3` variant if the hit point is genuinely wanted |
| Any engine function taking a `SlotPosition` | same function taking a `Vec3` |

Touches `ScriptGetterUtils`, `ScriptUtils.h`, `ScriptObjectTypeTags.h`, `ScriptVM.cpp`, and
the entity/component/physics/camera/scene namespaces.

### Stage 3 — delete the world

- Delete `World`, `WorldSingleton`, `SlotManager`, `ChunkRadiusLoader`, `ChunkCoordinate`,
  `Chunk`, `ChunkFactory`, `Terrain`, `TerrainManager`, `Slot/Recipe/`.
- Delete the six `Threading/Jobs/Recipe*Job` files.
- Delete `WorldEvent.h` and the `EventType::World` / `EventId::World*` entries.
- `Base`: construct `EntityManager`, `PhysicsManager`, `NavMeshManager` directly; expose via
  `BaseSingleton`; drop `TerrainManager` from the `BaseSingleton::initialise` argument list.
- `ThreadManager`: the physics thread's readiness is currently driven by world events
  (`WorldCreated` → `notifyWorldCreation(physicsManager)`, `WorldDestroyed`, `BecameReady`,
  `BecameUnready`). Drive it directly from `Base` startup/shutdown instead.
- `SystemSettings`: remove `mMapsDirectory`, `mMapsDirectoryViable`,
  `SystemSetup::_findMapsDirectory` and the `MapsDirectory` key in `avSetup.cfg`.
- Script: delete the `_world` and `_slotManager` namespaces entirely, and
  `_scene.registerChunkCallback` / `getNumDataPoints` / `getDataPointAt`. (Confirmed unused
  across both avTests and avData — only `testRayForSlot` has any callers.)
- **Needed replacement:** `_world.createWorld()` / `destroyWorld()` gave tests a clean slate
  between cases — 112 `createWorld` calls in avTests, and 16 `destroyWorld`. Add an explicit
  reset (e.g. `_entity.destroyAll()` plus a physics-world reset) before stage 6, or the test
  suite loses its isolation guarantee silently.

### Stage 4 — serialisation

- Delete `src/Serialisation/`, `World/Serialisation/`, and
  `Threading/Jobs/Entity{,De}SerialisationJob`.
- Delete `SerialisationNamespace` (`_serialisation.getAvailableSaves` / `clearAllSaves`) and
  `SaveHandleClass`. If saves-directory listing is worth keeping, it's a few lines on top of
  the existing file APIs, in a plugin or in `_system`.
- Remove `mSerialisationManager` from `Base` and `BaseSingleton`.
- Strip serialisation from `MeshClass`, `OgreMeshComponentLogic`, `ScriptComponentLogic`.

### Stage 5 — unit tests

Delete: `WorldSingletonTests`, `Slot/SlotManagerTests`, `Slot/SlotPositionTests`,
`Slot/ChunkCoordinateTests`, `Slot/ChunkRadiusChecksTests`, `Slot/ChunkFactoryMock`,
`Slot/Chunk/TerrainManagerTests`, `Slot/Recipe/SceneParserTests`,
`Slot/Recipe/CollisionObjectSceneParserTests`, `Entity/Tracker/EntityTrackerTests`,
`Scripting/ScriptNamespace/WorldNamespaceTests`.

Keep, relocated to match stage 1: `AvSceneFileParserTests`, `ObjMeshParserTests`, the
`Physics/` tests and mocks, `Nav/NavMeshManagerTests`, `Entity/UserComponents`,
`Entity/Callback`.

`test/unit/CMakeLists.txt` also globs, so again a re-configure rather than a list edit.

### Stage 6 — avTests

Of 243 `.nut` files, 103 touch a removed API. Two tiers:

- **32 files** use only lifecycle boilerplate (`createWorld` / `destroyWorld` /
  `setPlayerLoadRadius` and nothing else). Fully mechanical — worth a codemod script rather
  than 32 hand edits.
- **~71 files** additionally use `SlotPosition` (68 files), `_slotManager` (21) or player
  position, and need real edits.

Delete outright: `integration/SlotManagerTests`, `integration/SlotManagerMapSwitch`,
`integration/SlotManagerOriginShift`, `integration/SerialisationTests`,
`integration/TerrainTests`, `stress/World/WorldStartupShutdown`,
`stress/Terrain/TerrainChunk`.

`integration/TerrainTests` goes too — all four cases drive the chunk terrain via
`_slotManager.setCurrentMap()` and `_test.slotManager.getInUseTerrains()`, none touch
`_scene.createTerrain()`. `TerrainObject` has no integration coverage today; worth adding a
small replacement suite rather than leaving the survivor untested.

### Stage 7 — avData

Much lighter: 12 of 43 `.nut` files, and only 6 `createWorld` calls, 22 `SlotPosition`,
4 `_slotManager.setCurrentMap`, 2 `setOrigin`.

- Delete `avData/common/maps/` (the baked recipe directories).
- Remove the `"MapsDirectory"` key from every project's `avSetup.cfg`.
- Projects that loaded a map now load an AvScene file instead. `newSceneFormat/` is the
  reference for what that looks like.

## Risks

- **Stage 2 is where things get subtly wrong.** `SlotPosition`'s implicit conversions
  (`toOgre()` uses the current origin; `toOgreAbsolute()` doesn't) mean a mechanical swap to
  `Vector3` can silently change meaning wherever the origin was non-zero. Any code path
  using `toOgreWithOrigin` / `toBulletWithOrigin` needs reading, not replacing.
- **Physics is entangled but scheduled later.** Stage 2 has to strip origin shifting from the
  physics threads while leaving physics working. If that proves messy, pulling the physics
  removal forward to sit between stages 3 and 4 is the escape hatch.
- **avTests is the real cost.** ~100 files, and the suite is the safety net for everything
  else in 0.2.0. Getting the stage 3 reset primitive right before touching them matters more
  than it looks.
- **Sequencing.** Stages 1–5 are engine-only and can land before avTests/avData are touched,
  but the integration suite will be red in between. Either accept a red window on the branch
  or do stages 6/7 alongside stage 3.
