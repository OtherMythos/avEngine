"""
JSON emitter.

Produces the machine readable form of the API model. This is the sidecar the
Markdown is rendered from, and the format other tools should consume rather than
re-parsing C++.
"""

import json


def _param(p):
    out = {"index": p.index, "types": p.types, "optional": p.optional}
    if p.label:
        out["label"] = p.label
    if p.desc:
        out["desc"] = p.desc
    return out


def _overload(o):
    out = {}
    if o.desc:
        out["desc"] = o.desc
    if o.params:
        out["params"] = [_param(p) for p in o.params]
    if o.returns:
        out["returns"] = o.returns
    if o.returnType:
        out["returnType"] = o.returnType
    if o.examples:
        out["examples"] = o.examples
    return out


def _function(fn):
    out = {
        "name": fn.name,
        "signature": fn.signature,
        "documented": fn.documented,
        "source": {"file": fn.sourceFile, "line": fn.line},
    }
    if fn.viaMacro:
        out["viaMacro"] = fn.viaMacro
    if fn.desc:
        out["desc"] = fn.desc
    if fn.params:
        out["params"] = [_param(p) for p in fn.params]
    if fn.returns:
        out["returns"] = fn.returns
    if fn.returnType:
        out["returnType"] = fn.returnType
    if fn.examples:
        out["examples"] = fn.examples
    if fn.see:
        out["see"] = fn.see
    if fn.overloads:
        out["overloads"] = [_overload(o) for o in fn.overloads]
    return out


def _constant(c):
    out = {"name": c.name, "documented": c.documented}
    if c.desc:
        out["desc"] = c.desc
    if c.value:
        out["value"] = c.value
    return out


def _scope(scope):
    out = {
        "name": scope.name,
        "kind": scope.kind,
        "path": scope.path,
    }
    if scope.vm != 'main':
        out["vm"] = scope.vm
    if scope.availability:
        out["availability"] = scope.availability
    if scope.typeTag:
        out["typeTag"] = scope.typeTag
    if scope.desc:
        out["desc"] = scope.desc
    if scope.returnedBy:
        out["returnedBy"] = scope.returnedBy
    if scope.functions:
        out["functions"] = [_function(f) for f in scope.functions]
    if scope.constants:
        out["constants"] = [_constant(c) for c in scope.constants]
    out["sourceFiles"] = scope.sourceFiles
    return out


def render(api, meta=None):
    document = {
        "meta": meta or {},
        "stats": {
            "functions": api.totalFunctions(),
            "documentedFunctions": api.documentedFunctions(),
            "constants": api.totalConstants(),
            "documentedConstants": api.documentedConstants(),
            "scopes": len(api.scopes),
        },
        "scopes": [_scope(s) for s in api.scopes],
    }
    return json.dumps(document, indent=2, ensure_ascii=False) + "\n"
