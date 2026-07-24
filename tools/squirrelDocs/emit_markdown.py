"""
Markdown emitter.

Produces the single agent facing document. It is written to be grepped: every
function heading carries its fully qualified call name, so searching for
'_camera.setFOVy' or 'SceneNode.setPosition' lands directly on the entry.
Undocumented bindings are included with their inferred signature, since knowing
a call exists is useful even without a description.
"""


def _anchor(text):
    return text.lower().replace('.', '').replace(' ', '-').replace('(', '').replace(')', '')


def _availabilityNote(availability):
    if not availability:
        return None
    conditions = ', '.join(availability)
    return "Available only when built with `%s`." % conditions


def _renderParams(lines, params):
    documented = [p for p in params if p.label or p.desc]
    if not documented:
        return
    lines.append("")
    for p in params:
        types = '|'.join(p.types)
        label = p.label or ("arg%d" % p.index)
        optional = " _(optional)_" if p.optional else ""
        desc = (" — " + p.desc) if p.desc else ""
        lines.append("- `%s` (%s)%s%s" % (label, types, optional, desc))


def _renderFunction(lines, fn, qualifier):
    call = qualifier + fn.name
    lines.append("#### `%s`" % call)
    lines.append("")

    #The inferred signature is always shown; it is the one guaranteed fact.
    lines.append("```squirrel")
    lines.append(qualifier + fn.signature)
    lines.append("```")

    if not fn.documented:
        lines.append("")
        lines.append("_No description yet._")
        lines.append("")
        return

    if fn.desc:
        lines.append("")
        lines.append(fn.desc)

    _renderParams(lines, fn.params)

    if fn.returns or fn.returnType:
        lines.append("")
        typePart = ("`%s` — " % fn.returnType) if fn.returnType else ""
        lines.append("**Returns:** %s%s" % (typePart, fn.returns or ""))

    for i, overload in enumerate(fn.overloads, start=2):
        lines.append("")
        lines.append("_Overload %d:_ %s" % (i, overload.desc or ""))
        _renderParams(lines, overload.params)
        if overload.returns or overload.returnType:
            typePart = ("`%s` — " % overload.returnType) if overload.returnType else ""
            lines.append("")
            lines.append("**Returns:** %s%s" % (typePart, overload.returns or ""))

    allExamples = list(fn.examples)
    for overload in fn.overloads:
        allExamples.extend(overload.examples)
    for example in allExamples:
        lines.append("")
        lines.append("```squirrel")
        lines.append(example)
        lines.append("```")

    if fn.see:
        lines.append("")
        lines.append("_See also:_ " + ', '.join("`%s`" % s for s in fn.see))

    lines.append("")


def _renderConstants(lines, constants):
    if not constants:
        return
    lines.append("**Constants**")
    lines.append("")
    for c in constants:
        if c.desc:
            lines.append("- `%s` — %s" % (c.name, c.desc))
        else:
            lines.append("- `%s`" % c.name)
    lines.append("")


def _qualifierFor(scope):
    if scope.kind == 'namespace':
        return scope.path + '.'
    if scope.kind in ('object', 'class'):
        #Delegate methods are called on an instance with a dot; a lower-cased
        #receiver name signals that it stands for any instance of the type.
        receiver = scope.name[0].lower() + scope.name[1:]
        return "%s." % receiver
    return ''


def _renderScope(lines, scope):
    if scope.kind == 'namespace':
        heading = scope.path
    elif scope.kind in ('object', 'class'):
        heading = "%s %s" % (scope.name, "(object)" if scope.kind == 'object' else "(class)")
    else:
        heading = "Global functions"

    lines.append("### %s" % heading)
    lines.append("")

    note = _availabilityNote(scope.availability)
    if note:
        lines.append("> %s" % note)
        lines.append("")

    if scope.typeTag and scope.kind in ('object', 'class'):
        lines.append("_Type tag: `%s`_" % scope.typeTag)
        lines.append("")

    if scope.desc:
        lines.append(scope.desc)
        lines.append("")

    if scope.indexableInto:
        lines.append("_Indexable: `%s[i]` returns a `%s` object._"
                     % (scope.path, scope.indexableInto))
        lines.append("")

    if scope.returnedBy:
        lines.append("_Returned by:_ " + ', '.join("`%s`" % r for r in scope.returnedBy))
        lines.append("")

    documented = sum(1 for f in scope.functions if f.documented)
    if scope.functions:
        lines.append("_%d functions, %d documented._" % (len(scope.functions), documented))
        lines.append("")

    qualifier = _qualifierFor(scope)
    for fn in scope.functions:
        _renderFunction(lines, fn, qualifier)

    _renderConstants(lines, scope.constants)


def _renderContents(lines, api):
    lines.append("## Contents")
    lines.append("")

    namespaces = [s for s in api.namespaces() if s.vm == 'main']
    objects = api.objects()

    lines.append("**Namespaces:** " + ', '.join(
        "[%s](#%s)" % (s.path, _anchor(s.path)) for s in namespaces))
    lines.append("")
    lines.append("**Objects and classes:** " + ', '.join(
        "[%s](#%s)" % (s.name, _anchor(s.name + '-object' if s.kind == 'object' else s.name + '-class'))
        for s in objects))
    lines.append("")


def render(api, meta=None):
    meta = meta or {}
    lines = []

    lines.append("# avEngine Squirrel API")
    lines.append("")
    lines.append("_Generated by `tools/squirrelDocs`. Do not edit by hand; change the "
                 "engine source or its doc comments and regenerate._")
    lines.append("")

    tf, df = api.totalFunctions(), api.documentedFunctions()
    tc, dc = api.totalConstants(), api.documentedConstants()
    lines.append("This document is generated from the engine source. It lists every "
                 "function and constant reachable from Squirrel, whether or not it "
                 "carries a written description.")
    lines.append("")
    lines.append("- **%d functions** (%d documented, %.0f%%)" % (tf, df, 100 * df / tf if tf else 0))
    lines.append("- **%d constants** (%d documented, %.0f%%)" % (tc, dc, 100 * dc / tc if tc else 0))
    lines.append("")

    _renderContents(lines, api)

    lines.append("---")
    lines.append("")

    #Namespaces first, then objects, then globals. The model is already sorted.
    mainNamespaces = [s for s in api.scopes if s.kind == 'namespace' and s.vm == 'main']
    setupNamespaces = [s for s in api.scopes if s.kind == 'namespace' and s.vm == 'setup']
    objects = api.objects()
    globals_ = [s for s in api.scopes if s.kind == 'root']

    lines.append("## Namespaces")
    lines.append("")
    for scope in mainNamespaces:
        _renderScope(lines, scope)

    if objects:
        lines.append("## Objects and classes")
        lines.append("")
        lines.append("These types are not constructed directly by name unless noted; "
                     "they are returned from the calls above and carry their own methods.")
        lines.append("")
        for scope in objects:
            _renderScope(lines, scope)

    for scope in globals_:
        lines.append("## Global functions and constructors")
        lines.append("")
        _renderScope(lines, scope)

    if setupNamespaces:
        lines.append("## Setup script namespaces")
        lines.append("")
        lines.append("These are available only inside the project setup script, which "
                     "runs in a reduced VM before the main engine starts.")
        lines.append("")
        for scope in setupNamespaces:
            _renderScope(lines, scope)

    return '\n'.join(lines) + '\n'
