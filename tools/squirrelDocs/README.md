# Squirrel API documentation generator

Generates a reference for the engine's Squirrel scripting API by reading the
engine source directly. It is meant to give both people and AI agents a complete,
current picture of what can be called from a project script.

## Why it reads the source rather than only comments

Every binding in the engine is registered through `ScriptUtils::addFunction`, and
every constant through `ScriptUtils::declareConstant`. Because nothing bypasses
those, the full API surface can be discovered from the code whether or not anyone
wrote a doc comment. The generator extracts that surface first, then attaches the
`/**SQ...` comments on top. An undocumented call still appears, with a signature
inferred from its Squirrel typemask, because knowing a call exists is useful even
without prose.

## Running it

```sh
python3 generate.py                 # writes into ../../docs
python3 generate.py --src ../../src --out /tmp/apidocs
python3 generate.py --check         # CI: fail if committed output is stale
```

Outputs, written to `docs/`:

- `squirrelApi.md` — the agent and human facing reference, one grep-able file.
- `squirrelApi.json` — the machine readable model, for other tooling.
- `squirrelApiCoverage.md` — undocumented bindings and documentation problems,
  i.e. the backfill worklist.

No third-party dependencies; standard-library Python 3 only.

## Documentation comment syntax

A block runs from its marker to `*/`. A tag's value continues until the next tag.

```cpp
/**SQNamespace
@name _camera
@desc Controls the scene camera.
*/

/**SQFunction
@name setPosition
@desc Set the camera position.
@param1:x: The x coordinate.
@param2:y: The y coordinate.
@param3:z: The z coordinate.
@returns Nothing.
@example _camera.setPosition(0, 0, 100);
*/
ScriptUtils::addFunction(vm, setPos, "setPosition", -2, ".n|unn");
```

Markers: `/**SQFunction`, `/**SQNamespace`, `/**SQConstant`, `/**SQObject`.

Tags:

| Tag | Meaning |
|-----|---------|
| `@name` | Script visible name (required except on `SQObject`). |
| `@desc` | Description. |
| `@param<N>:label: text` | Parameter N. The label and text may be glued after the colon. |
| `@returns` or `@returns:type: text` | Return value, with an optional type. |
| `@example` | A Squirrel snippet. The highest value tag for agents. |
| `@see` | A cross reference. |
| `@property name:type: text` | A field reached through a `_get`/`_set` metamethod. |
| `@typeTag` | On `SQObject`, the C++ type tag, for cross referencing. |
| `@returnedBy` | On `SQObject`, a call that produces one. |

### Overloads

Several `/**SQFunction` blocks before one binding document its overloads. Each
block describes one accepted call shape and all are kept.

### Documenting an object type

Delegate-table types (`SceneNode`, `Vector3`, widgets, ...) take a `/**SQObject`
block placed near their setup function:

```cpp
/**SQObject
@name SceneNode
@desc A node in the scene graph. Returned by _scene.getSceneNode().
@typeTag SceneNodeTypeTag
@returnedBy _scene.getSceneNode
*/
```

## How it works

The pipeline is a few small modules:

- `manifest.py` — reads `ScriptVM.cpp`, the authoritative list of what is
  registered and under which script name, and for which VM.
- `macros.py` — expands the parameterless concatenation macros
  (`BASIC_WIDGET_FUNCTIONS`, ...) so shared bindings are not missed.
- `bindings.py` — walks each registered setup function, tracking the Squirrel
  stack to attribute every binding to its namespace, object or class.
- `typemask.py` — turns a Squirrel typemask into a readable signature.
- `doccomments.py` — parses the `/**SQ...` blocks.
- `model.py` — fuses bindings and comments into one model.
- `emit_markdown.py`, `emit_json.py`, `report.py` — render the outputs.

The extractor reconciles exactly against the raw count of non-commented
`addFunction` calls, accounting for the handful of bindings the engine registers
in both the main and the reduced setup VM. If a future change to the binding
idioms breaks that, the count moves and the tests in `tests/` are the first place
to look.

## Tests

```sh
python3 -m unittest discover -s tests
```

The fixtures are small inline source snippets mirroring real engine idioms, so a
regression fails with a clear cause rather than only as a changed total.
