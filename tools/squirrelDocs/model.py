"""
The unified API model.

This ties the binding surface extracted from the source to the documentation
comments, producing one tree that the emitters render. Every binding appears
whether or not it carries documentation, so the model is a complete picture of
the script API rather than only its documented parts.

Documentation is matched to bindings by source file and name. Several function
blocks sharing a name describe overloads and are kept together. A block whose
name matches no binding in its file is reported as orphaned, which usually means
a function was renamed or removed without updating its comment.
"""

import doccomments


class Param:
    def __init__(self, index, types, optional, label=None, desc=None):
        self.index = index
        #Types inferred from the typemask.
        self.types = types
        self.optional = optional
        #Label and description come from documentation.
        self.label = label
        self.desc = desc


class Overload:
    """One documented call shape for a function."""

    def __init__(self, desc=None, params=None, returns=None,
                 returnType=None, examples=None):
        self.desc = desc
        self.params = params or []
        self.returns = returns
        self.returnType = returnType
        self.examples = examples or []


class Function:
    def __init__(self, name, signature, sourceFile, line):
        self.name = name
        #Signature inferred from the typemask.
        self.signature = signature
        self.sourceFile = sourceFile
        self.line = line
        self.viaMacro = None
        self.cFunction = None

        self.documented = False
        #The primary description, taken from the first documented overload.
        self.desc = None
        self.returns = None
        self.returnType = None
        self.examples = []
        self.see = []
        #Parameters, merged from the typemask and the first overload.
        self.params = []
        #Additional documented call shapes beyond the first.
        self.overloads = []
        #Whether more than one binding of this name exists (a true overload set).
        self.isOverloaded = False


class Property:
    def __init__(self, name, type_, desc):
        self.name = name
        self.type = type_
        self.desc = desc


class Constant:
    def __init__(self, name, value, sourceFile, line):
        self.name = name
        self.value = value
        self.sourceFile = sourceFile
        self.line = line
        self.documented = False
        self.desc = None


class ApiScope:
    def __init__(self, name, kind, path, availability, sourceFiles, vm):
        self.name = name
        self.kind = kind
        self.path = path
        self.availability = availability
        self.sourceFiles = sourceFiles
        self.vm = vm
        self.typeTag = None
        self.desc = None
        self.returnedBy = []
        self.functions = []
        self.constants = []
        self.properties = []
        #Set on a namespace that is indexed to reach an object, e.g.
        #_physics.collision[0]. Names the object reached by indexing.
        self.indexableInto = None

    def documentedFunctionCount(self):
        return sum(1 for f in self.functions if f.documented)

    def documentedConstantCount(self):
        return sum(1 for c in self.constants if c.documented)


class Api:
    def __init__(self):
        self.scopes = []
        self.issues = []
        #Undocumented bindings, for the coverage report.
        self.undocumentedFunctions = []
        self.undocumentedConstants = []
        self.orphanedBlocks = []
        self.warnings = []

    def namespaces(self):
        return [s for s in self.scopes if s.kind == 'namespace']

    def objects(self):
        #Objects reachable from the main VM. A type also registered in the setup
        #VM (Vector2) is the same type and is listed once here.
        return [s for s in self.scopes if s.kind in ('object', 'class') and s.vm == 'main']

    def globalScope(self):
        for s in self.scopes:
            if s.kind == 'root':
                return s
        return None

    def totalFunctions(self):
        return sum(len(s.functions) for s in self.scopes)

    def totalConstants(self):
        return sum(len(s.constants) for s in self.scopes)

    def documentedFunctions(self):
        return sum(s.documentedFunctionCount() for s in self.scopes)

    def documentedConstants(self):
        return sum(s.documentedConstantCount() for s in self.scopes)


def _resolveAnonymous(scopes):
    """
    Give the anonymous tables reached through a _get metamethod a real identity.

    The engine exposes a few indexable namespaces: _physics.collision[i] and
    _component.user[i] each return an object with its own methods. These arrive
    as two anonymous tables per parent — one holding the _get metamethod, one
    holding the object's methods. The object is named after its access path so
    it is not lost, and the parent is marked as indexing into it.

    Returns a map of parent-scope id -> the name it indexes into, for the parent
    to display.
    """
    indexableNotes = {}
    byParent = {}
    for scope in scopes:
        if scope.name is None and scope.parent is not None:
            byParent.setdefault(id(scope.parent), []).append(scope)

    for scope in scopes:
        siblings = byParent.get(id(scope))
        if not siblings:
            continue
        parent = scope
        methodTables = [s for s in siblings
                        if any(not f.name.startswith('_') for f in s.functions)]
        for table in methodTables:
            name = _indexedName(parent)
            table.name = name
            table.kind = 'object'
            table.parent = None
            indexableNotes[id(parent)] = name

    #The bare metamethod-only tables are folded away; drop their bindings so they
    #do not skew coverage, leaving the parent's indexable note to explain them.
    for scope in scopes:
        siblings = byParent.get(id(scope))
        if not siblings:
            continue
        for table in siblings:
            if table.name is None:
                table.functions = []
                table.constants = []

    return indexableNotes


def _indexedName(parent):
    """A stable, greppable name for an object reached by indexing a namespace."""
    segment = parent.name or 'element'
    #_physics.collision -> CollisionWorldEntry style, kept simple and unique.
    base = segment[0].upper() + segment[1:]
    return base + "Element"


def _indexBlocksByName(blocksByFile):
    """
    (sourceFile, name) -> list of function/constant blocks in source order.
    Namespace and object blocks are indexed separately by file.
    """
    functionBlocks = {}
    constantBlocks = {}
    scopeBlocks = {}

    for relPath, blocks in blocksByFile.items():
        for block in blocks:
            if block.kind == 'Function':
                functionBlocks.setdefault((relPath, block.name), []).append(block)
            elif block.kind == 'Constant':
                constantBlocks.setdefault((relPath, block.name), []).append(block)
            else:
                #Namespace or Object.
                scopeBlocks.setdefault(relPath, []).append(block)

    return functionBlocks, constantBlocks, scopeBlocks


def _buildParams(binding, overloadBlock):
    """
    Combine the typemask derived parameters with any documented ones.
    Documentation fills in labels and descriptions; the typemask supplies types
    and which parameters are optional.
    """
    from typemask import buildParams

    inferred, _ = buildParams(binding.numParams, binding.typemask)
    documented = {p["index"]: p for p in (overloadBlock.orderedParams()
                                          if overloadBlock else [])}

    params = []
    count = max(len(inferred), max(documented, default=0))
    for i in range(1, count + 1):
        types = inferred[i - 1]["types"] if i - 1 < len(inferred) else ['any']
        optional = inferred[i - 1]["optional"] if i - 1 < len(inferred) else False
        doc = documented.get(i)
        params.append(Param(
            i, types, optional,
            label=doc["label"] if doc else None,
            desc=doc["desc"] if doc else None))
    return params


def build(extractor, blocksByFile, docIssues):
    """
    extractor: the Extractor from bindings.extract.
    blocksByFile, docIssues: from doccomments.parseTree.
    """
    from typemask import formatSignature

    api = Api()
    api.issues.extend(str(i) for i in docIssues)
    api.warnings.extend(extractor.warnings)

    functionBlocks, constantBlocks, scopeBlocks = _indexBlocksByName(blocksByFile)
    #Constants are global identifiers, and their documentation is usually written
    #beside the enum in a header while the binding is generated from an array in
    #a .cpp, so they are matched by name across the whole tree.
    constantBlocksByName = {}
    for (relPath, name), blocks in constantBlocks.items():
        constantBlocksByName.setdefault(name, []).append((relPath, blocks))
    usedFunctionKeys = set()
    usedConstantNames = set()

    scopes = extractor.scopes + list(extractor.roots.values())
    indexableNotes = _resolveAnonymous(scopes)
    #Skip scopes that never received a binding, and unnamed leftovers. An
    #indexable namespace is kept even with no direct functions, since the object
    #it indexes into is its whole purpose.
    scopes = [s for s in scopes
              if (s.functions or s.constants or id(s) in indexableNotes)
              and (s.name is not None or s.kind == 'root')]

    for scope in scopes:
        path = scope.path() if scope.kind != 'root' else '(global)'
        apiScope = ApiScope(
            scope.name if scope.kind != 'root' else '(global)',
            scope.kind, path, scope.availability,
            sorted(scope.sourceFiles), scope.vm)
        apiScope.typeTag = scope.typeTag

        apiScope.indexableInto = indexableNotes.get(id(scope))

        _attachScopeDoc(apiScope, scope, scopeBlocks)

        for binding in scope.functions:
            fn = _buildFunction(binding, functionBlocks, usedFunctionKeys, api)
            apiScope.functions.append(fn)
            if not fn.documented:
                api.undocumentedFunctions.append((path, fn))

        for const in scope.constants:
            c = Constant(const.name, const.value, const.sourceFile, const.line)
            #Prefer a doc block in the same file, then fall back to any file.
            blocks = constantBlocks.get((const.sourceFile, const.name))
            if not blocks:
                byName = constantBlocksByName.get(const.name)
                if byName:
                    blocks = byName[0][1]
            if blocks:
                usedConstantNames.add(const.name)
                c.documented = True
                c.desc = blocks[0].desc
            else:
                api.undocumentedConstants.append((path, c))
            apiScope.constants.append(c)

        api.scopes.append(apiScope)

    _reportOrphans(api, functionBlocks, usedFunctionKeys,
                   constantBlocks, usedConstantNames)

    #A stable order: namespaces and objects alphabetically, globals last.
    api.scopes.sort(key=lambda s: (
        {'root': 3, 'object': 2, 'class': 2, 'namespace': 1}.get(s.kind, 1),
        s.path.lower()))
    return api


def _buildFunction(binding, functionBlocks, usedKeys, api):
    from typemask import formatSignature

    signature = formatSignature(binding.name, binding.numParams, binding.typemask)
    fn = Function(binding.name, signature, binding.sourceFile, binding.line)
    fn.viaMacro = binding.viaMacro
    fn.cFunction = binding.cFunction

    key = (binding.sourceFile, binding.name)
    blocks = functionBlocks.get(key, [])
    if blocks:
        usedKeys.add(key)
        fn.documented = True
        fn.isOverloaded = len(blocks) > 1

        first = blocks[0]
        fn.desc = first.desc
        fn.returns = first.returns
        fn.returnType = first.returnType
        fn.examples = list(first.examples)
        fn.see = list(first.see)
        fn.params = _buildParams(binding, first)

        for block in blocks[1:]:
            fn.overloads.append(Overload(
                desc=block.desc,
                params=[Param(p["index"], ['any'], False, p["label"], p["desc"])
                        for p in block.orderedParams()],
                returns=block.returns,
                returnType=block.returnType,
                examples=list(block.examples)))
    else:
        fn.params = _buildParams(binding, None)

    return fn


def _attachScopeDoc(apiScope, scope, scopeBlocks):
    """
    Attach a namespace or object doc block to its scope. Namespace blocks match
    by their @name; object blocks match by type tag or by their @name.
    """
    candidates = scopeBlocks.get(_primarySourceFile(scope), [])
    for block in candidates:
        if block.kind == 'Namespace' and block.name == scope.name:
            apiScope.desc = block.desc
            return
        if block.kind == 'Object':
            if (block.typeTag and block.typeTag == scope.typeTag) or block.name == scope.name:
                apiScope.desc = block.desc
                apiScope.returnedBy = list(block.returnedBy)
                if block.typeTag:
                    apiScope.typeTag = block.typeTag
                return

    #A namespace block may live in any of the scope's files.
    for relPath in scope.sourceFiles:
        for block in scopeBlocks.get(relPath, []):
            if block.kind == 'Namespace' and block.name == scope.name:
                apiScope.desc = block.desc
                return
            if block.kind == 'Object' and block.name == scope.name:
                apiScope.desc = block.desc
                apiScope.returnedBy = list(block.returnedBy)
                return


def _primarySourceFile(scope):
    return next(iter(scope.sourceFiles), None) if scope.sourceFiles else None


def _reportOrphans(api, functionBlocks, usedFunctionKeys,
                   constantBlocks, usedConstantNames):
    for key, blocks in functionBlocks.items():
        if key not in usedFunctionKeys:
            relPath, name = key
            for block in blocks:
                api.orphanedBlocks.append(
                    "%s:%d documents function '%s' but no such binding was found "
                    "in that file" % (relPath, block.startLine, name))

    for (relPath, name), blocks in constantBlocks.items():
        if name not in usedConstantNames:
            for block in blocks:
                api.orphanedBlocks.append(
                    "%s:%d documents constant '%s' but no such binding was found"
                    % (relPath, block.startLine, name))
