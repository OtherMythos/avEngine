"""
Coverage and lint report.

This is the backfill worklist. It groups undocumented bindings by scope so the
gaps can be filled where they matter most, and it collects the documentation
problems worth fixing: malformed tags, orphaned comments, and typemasks the
Squirrel VM would reject.
"""

def render(api, extractor=None):
    lines = []
    tf, df = api.totalFunctions(), api.documentedFunctions()
    tc, dc = api.totalConstants(), api.documentedConstants()

    lines.append("# Squirrel API documentation coverage")
    lines.append("")
    lines.append("Functions: %d/%d documented (%.1f%%)" % (df, tf, 100 * df / tf if tf else 0))
    lines.append("Constants: %d/%d documented (%.1f%%)" % (dc, tc, 100 * dc / tc if tc else 0))
    lines.append("")

    #Undocumented functions grouped by scope, most gaps first.
    byScope = {}
    for path, fn in api.undocumentedFunctions:
        byScope.setdefault(path, []).append(fn)

    if byScope:
        lines.append("## Undocumented functions")
        lines.append("")
        for path in sorted(byScope, key=lambda p: (-len(byScope[p]), p)):
            fns = byScope[path]
            lines.append("### %s — %d undocumented" % (path, len(fns)))
            for fn in sorted(fns, key=lambda f: f.name):
                macro = (" (via %s)" % fn.viaMacro) if fn.viaMacro else ""
                lines.append("- `%s`  — %s:%d%s" % (fn.signature, fn.sourceFile, fn.line, macro))
            lines.append("")

    undocConst = {}
    for path, c in api.undocumentedConstants:
        undocConst.setdefault(path, []).append(c)
    if undocConst:
        lines.append("## Undocumented constants")
        lines.append("")
        for path in sorted(undocConst, key=lambda p: (-len(undocConst[p]), p)):
            consts = undocConst[path]
            names = ', '.join("`%s`" % c.name for c in sorted(consts, key=lambda c: c.name))
            lines.append("### %s — %d undocumented" % (path, len(consts)))
            lines.append(names)
            lines.append("")

    if api.issues:
        lines.append("## Documentation problems")
        lines.append("")
        for issue in api.issues:
            lines.append("- %s" % issue)
        lines.append("")

    if api.orphanedBlocks:
        lines.append("## Orphaned documentation")
        lines.append("")
        lines.append("Comments whose named binding could not be found. Usually a "
                     "rename or a copy-paste that drifted from its function.")
        lines.append("")
        for orphan in api.orphanedBlocks:
            lines.append("- %s" % orphan)
        lines.append("")

    if api.warnings:
        lines.append("## Extraction warnings")
        lines.append("")
        lines.append("Bindings the generator could not place, or that appear to be "
                     "unreachable from script.")
        lines.append("")
        for warning in api.warnings:
            lines.append("- %s" % warning)
        lines.append("")

    return '\n'.join(lines) + '\n'


def summarise(api):
    """A short human summary for the console."""
    tf, df = api.totalFunctions(), api.documentedFunctions()
    tc, dc = api.totalConstants(), api.documentedConstants()
    parts = [
        "%d functions (%d documented, %.0f%%)" % (tf, df, 100 * df / tf if tf else 0),
        "%d constants (%d documented, %.0f%%)" % (tc, dc, 100 * dc / tc if tc else 0),
        "%d scopes" % len(api.scopes),
    ]
    if api.issues:
        parts.append("%d doc problems" % len(api.issues))
    if api.orphanedBlocks:
        parts.append("%d orphaned" % len(api.orphanedBlocks))
    return ', '.join(parts)
