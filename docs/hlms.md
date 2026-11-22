# Hlms: per-datablock custom pieces

A datablock can carry its own Hlms piece file and its own Hlms properties. Its shaders then
get extra code that no other material sees, chosen explicitly rather than inferred from
unrelated state like `hlms_uv_count1` or a `userValue` mode.

This is a backport of Ogre-Next `master` (4.0) into the `v3-0` branch avEngine builds
against. It's `avBuild/patches/ogreDatablockCustomPieces.diff`, applied by every platform
script in avBuild after `ogreItemDatablock.diff`. The engine **will not compile** against an
`avBuilt` tree built without it.

## Script API

Every datablock, PBS or Unlit, has:

| Function | Does |
|---|---|
| `setCustomPieceFile(file, stage, group = autodetect)` | Parse `file`, a piece file found through the resource system, for this datablock's shaders at `stage`. An empty `file` clears the stage. |
| `setCustomPieceCode(name, code, stage)` | The same with the code passed in as a string. `name` identifies it. An empty `name` and `code` clears the stage. |
| `getCustomPieceFile(stage)` | The file or name set for `stage`, or `null`. |
| `setCustomProperties(table)` | Replace the datablock's custom Hlms properties with `{ name = int }`. Bools become `0`/`1`. |
| `getCustomProperties()` | The current custom properties, as a table. |

`stage` is one of `_HLMS_PIECE_STAGE_PRE`, `_VS`, `_PS`, `_GS`, `_HS` or `_DS`.

```squirrel
local datablock = _hlms.pbs.createDatablock("ghost");
datablock.setCustomPieceFile("ghost_ps.any", _HLMS_PIECE_STAGE_PS);
datablock.setCustomProperties({ ghostMode = 2 });
```

Each stage holds one piece file. Stages are independent, and `cloneBlock` copies both the
pieces and the properties.

A name must always mean the same code. Hlms keeps one copy of each piece per name, shared by
every datablock that uses it. Setting a name again with different code throws, and so does a
file that can't be found.

## In `.material.json`

```json
"custom_piece_file_ps" : [ "ghost_ps.any", "General" ],
"custom_properties" : { "ghostMode" : 2 }
```

The keys are `custom_piece_file_pre|vs|ps|gs|hs|ds`. Both keys work for any Hlms type.

## Writing the piece file

The file fills the `custom_*` hooks the templates already expose: `custom_ps_posExecution`,
`custom_VStoPS`, `custom_vs_preTransform`, `custom_ps_preLights` and the rest. Custom
properties are ordinary Hlms properties, so the file can branch on them:

```
@property( ghostMode == 2 )
@piece( custom_ps_posExecution )
	@property( !hlms_shadowcaster )
		outPs_colour0.w *= 0.5;
	@end
@end
@end
```

- **A piece can only be defined once per shader.** If a project's Hlms library already
  defines `custom_ps_posExecution`, a datablock piece that defines it too fails to compile.
  Give hooks to one or the other.
- Pieces are parsed in this order: the PRE stage (vertex shader only), then library pieces,
  then the datablock's piece, then the main template pieces, then the shader file.
- **Never write an at-sign in a comment** in a piece file. Hlms reads it as a directive, and
  the error shows up on a different, generated line.
- The piece file and the custom properties are part of the shader hash, so each combination
  becomes its own shader. One shared piece file gated on custom properties produces fewer
  files than one piece file per material.
- The shadow caster pass gets the piece and the properties too. Wrap code that only applies
  to colour in `@property( !hlms_shadowcaster )`.
- Code set from a string (`setCustomPieceCode`) can't go in the HlmsDiskCache. Use a file
  for shipped content.

Tests: `avTests/integration/HlmsTests/DatablockCustomPieces` covers the API, and
`DatablockCustomPiecesRender` checks the rendered output, including from JSON.
