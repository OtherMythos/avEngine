"""
Extraction of the script binding surface from the engine source.

Every binding in the engine goes through ScriptUtils::addFunction, and every
constant through ScriptUtils::declareConstant. Starting from the manifest in
ScriptVM.cpp, this walks the body of each registered setup function, follows
calls into nested setup functions, and attributes each binding to the table it
is being registered against.

Table attribution is done by tracking the Squirrel stack the same way the C++
does: sq_newtable and sq_pushroottable push a scope, sq_newslot and sq_pop take
one off, and a binding lands on whatever is currently on top.
"""

import os
import re

import macros
from typemask import maskProblems

ADD_FUNCTION = re.compile(
    r'ScriptUtils::addFunction\s*\(\s*vm\s*,\s*(.+?)\s*,\s*"([^"]*)"\s*'
    r'(?:,\s*(-?\d+)\s*(?:,\s*"([^"]*)"\s*)?)?\)')

DECLARE_CONSTANT = re.compile(
    r'ScriptUtils::declareConstant\s*\(\s*vm\s*,\s*(?:"([^"]*)"|([A-Za-z_][A-Za-z0-9_]*(?:\[[^\]]*\])?))\s*,\s*(.+?)\s*\)\s*;')

#functionMap["assertTrue"] = {".b", 2, assertTrue};
#Test mode namespaces register through a map so the bindings can be swapped for
#a stub when test mode is off.
FUNCTION_MAP_ENTRY = re.compile(
    r'functionMap\s*\[\s*"([^"]*)"\s*\]\s*=\s*\{\s*"([^"]*)"\s*,\s*(-?\d+)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\}')

#{TestModeTextureNamespace::setupTestNamespace, "texture"}
TEST_NAMESPACE_ENTRY = re.compile(
    r'\{\s*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*,\s*"([^"]*)"\s*\}')

PUSH_STRING = re.compile(r'sq_pushstring\s*\(\s*vm\s*,\s*_SC\(\s*"([^"]*)"\s*\)')
#A Squirrel class is built the same way a delegate table is, from the point of
#view of what gets registered onto it.
NEW_TABLE = re.compile(r'sq_(?:newtable(?:ex)?|newclass)\s*\(\s*vm')
PUSH_ROOT = re.compile(r'sq_pushroottable\s*\(\s*vm\s*\)')
NEW_SLOT = re.compile(r'sq_newslot\s*\(\s*vm')
POP = re.compile(r'sq_pop\s*\(\s*vm\s*,\s*(\d+)\s*\)')
SET_TYPETAG = re.compile(r'sq_settypetag\s*\(\s*vm\s*,\s*-?\d+\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')

#sq_getstackobj(vm, -1, &lightDelegateTableObject);
#The captured variable is the only authoritative name for a delegate table, and
#one setup function often builds several of them.
GET_STACK_OBJ = re.compile(
    r'sq_getstackobj\s*\(\s*vm\s*,\s*-?\d+\s*,\s*&\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')

TABLE_VAR_SUFFIXES = (
    'DelegateTableObject', 'DelegateTable', 'TableObject', 'Table', 'Object')

#void CameraNamespace::setupNamespace(HSQUIRRELVM vm){
#Setup functions may take more than the VM, as the test namespaces do.
FUNCTION_DEF = re.compile(
    r'^\s*(?:static\s+)?void\s+([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*HSQUIRRELVM\s+\w+\s*[,)]')

#Direct static call into another setup function: SomeClass::setupThing(vm);
STATIC_SETUP_CALL = re.compile(
    r'^\s*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*vm\s*\)\s*;')

#Instance call: meshComponent.setupNamespace(vm);
INSTANCE_SETUP_CALL = re.compile(
    r'^\s*([A-Za-z_][A-Za-z0-9_]*)\.([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*vm\s*\)\s*;')

#Local declaration used to resolve the above: MeshComponentNamespace meshComponent;
LOCAL_DECL = re.compile(
    r'^\s*([A-Za-z_][A-Za-z0-9_]*Namespace)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;')

#ScriptUtils::setupDelegateTable(vm, &windowDelegateTable, GuiWidgetDelegate::setupWindow);
DELEGATE_SETUP = re.compile(
    r'ScriptUtils::setupDelegateTable\s*\(\s*vm\s*,\s*&\s*([A-Za-z_][A-Za-z0-9_]*)\s*,\s*'
    r'([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\)')

#const char* const InputNamesKeys[] = { "_K_UNKNOWN", ... };
STRING_ARRAY = re.compile(
    r'(?:static\s+)?const\s+char\s*\*\s*(?:const\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*\]\s*=\s*\{')

#const char* c = PauseTypeStrs[i];
#Some constant loops read the name into a local first.
ARRAY_ALIAS = re.compile(
    r'const\s+char\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*([A-Za-z_][A-Za-z0-9_]*)\s*\[')

OBJECT_SUFFIXES = ('UserData', 'Class', 'Delegate')


def stripLineComment(text):
    """
    Remove a // line comment, leaving string literals intact. Commented out
    calls must not be mistaken for real registrations.
    """
    inString = False
    escaped = False
    for i, char in enumerate(text):
        if escaped:
            escaped = False
            continue
        if char == '\\':
            escaped = True
            continue
        if char == '"':
            inString = not inString
            continue
        if not inString and char == '/' and i + 1 < len(text) and text[i + 1] == '/':
            return text[:i]
    return text


class Binding:
    def __init__(self, name, cFunction, numParams, typemask, sourceFile, line, viaMacro):
        self.name = name
        self.cFunction = cFunction
        self.numParams = numParams
        self.typemask = typemask
        self.sourceFile = sourceFile
        self.line = line
        self.viaMacro = viaMacro


class Constant:
    def __init__(self, name, value, sourceFile, line, fromArray=None):
        self.name = name
        self.value = value
        self.sourceFile = sourceFile
        self.line = line
        self.fromArray = fromArray


class Scope:
    def __init__(self, name, kind, owner=None, parent=None, vm='main'):
        self.name = name
        #namespace | object | class | root | anonymous
        self.kind = kind
        #C++ class that defines it.
        self.owner = owner
        self.parent = parent
        #Which VM this scope is reachable from: main, or the reduced setup VM.
        self.vm = vm
        self.functions = []
        self.constants = []
        self.typeTag = None
        self.availability = []
        self.sourceFiles = set()

    def path(self):
        if self.parent is not None and self.parent.kind != 'root':
            return self.parent.path() + '.' + self.name
        return self.name


class SourceIndex:
    """Every C++ function in the tree that takes an HSQUIRRELVM."""

    def __init__(self, root):
        self.root = root
        #(class, function) -> (relPath, startIndex, expandedLines)
        self.functions = {}
        #arrayName -> [strings]
        self.stringArrays = {}
        self._build()

    def _build(self):
        for dirPath, _, files in os.walk(self.root):
            for name in files:
                if not name.endswith(('.cpp', '.h')):
                    continue
                fullPath = os.path.join(dirPath, name)
                relPath = os.path.relpath(fullPath, self.root)
                try:
                    with open(fullPath, 'r', errors='replace') as f:
                        rawLines = f.readlines()
                except OSError:
                    continue

                self._indexStringArrays(rawLines, relPath)

                expanded = macros.expandFile(rawLines)
                self._indexFunctions(expanded, relPath)

    def _indexStringArrays(self, rawLines, relPath):
        i = 0
        while i < len(rawLines):
            match = STRING_ARRAY.search(rawLines[i])
            if not match:
                i += 1
                continue
            values = []
            j = i
            while j < len(rawLines):
                values.extend(re.findall(r'"([^"]*)"', rawLines[j]))
                if '}' in rawLines[j] and j > i:
                    break
                if '}' in rawLines[j] and j == i and rawLines[i].count('}'):
                    break
                j += 1
            self.stringArrays[match.group(1)] = values
            i = j + 1

    def _indexFunctions(self, expanded, relPath):
        i = 0
        while i < len(expanded):
            text = expanded[i][0]
            match = FUNCTION_DEF.match(text)
            if not match:
                i += 1
                continue

            cls, function = match.group(1), match.group(2)

            #Walk to the closing brace of the body.
            depth = 0
            started = False
            j = i
            body = []
            while j < len(expanded):
                lineText = expanded[j][0]
                body.append(expanded[j])
                depth += lineText.count('{') - lineText.count('}')
                if '{' in lineText:
                    started = True
                if started and depth <= 0:
                    break
                j += 1

            self.functions[(cls, function)] = (relPath, i, body)
            i = j + 1

    def lookup(self, cls, function):
        return self.functions.get((cls, function))


def _nameFromTableVariable(variable):
    """lightDelegateTableObject -> Light, SceneNodeDelegateTableObject -> SceneNode."""
    name = variable
    for suffix in TABLE_VAR_SUFFIXES:
        if name.endswith(suffix) and len(name) > len(suffix):
            name = name[:-len(suffix)]
            break
    if not name:
        return None
    return name[0].upper() + name[1:]


def _objectNameFor(cls, function):
    """Derive a script visible type name for a delegate table."""
    if function.startswith('setup') and function not in (
            'setupDelegateTable', 'setupTable', 'setupClass',
            'setupNamespace', 'setupConstants', 'setupFunctions'):
        #GuiWidgetDelegate::setupWindow describes a Window, not a GuiWidget.
        return function[len('setup'):]

    name = cls
    for suffix in OBJECT_SUFFIXES:
        if name.endswith(suffix):
            return name[:-len(suffix)]
    return name


class Extractor:
    def __init__(self, index):
        self.index = index
        #The main VM and the reduced setup VM are separate surfaces which
        #happen to share some namespace names, so they get separate roots.
        self.roots = {'main': Scope('', 'root', vm='main'),
                      'setup': Scope('', 'root', vm='setup')}
        self.scopes = []
        self.warnings = []
        self._visiting = set()
        self._vm = 'main'
        #Every (class, function) pair actually reached from a root.
        self.visited = set()

    @property
    def root(self):
        return self.roots[self._vm]

    def addScope(self, scope):
        self.scopes.append(scope)
        return scope

    def run(self, registrations):
        for vm in ('main', 'setup'):
            self._vm = vm
            for reg in registrations:
                if reg.vm == vm:
                    self._runRegistration(reg)
        self._vm = 'main'
        self._reportUnregistered()
        return self.roots

    def _runRegistration(self, reg):
        entry = self.index.lookup(reg.cls, reg.function)
        if entry is None:
            self.warnings.append(
                "No body found for %s::%s (registered at ScriptVM.cpp:%d)"
                % (reg.cls, reg.function, reg.line))
            return

        key = (reg.cls, reg.function)

        if reg.kind == 'namespace':
            scope = self._findOrCreate(reg.scriptName, 'namespace', reg.cls, self.root)
            scope.availability = reg.availability
            self._walk(entry, scope, False, key)
        elif reg.kind in ('object', 'class'):
            name = _objectNameFor(reg.cls, reg.function)
            scope = self._findOrCreate(name, reg.kind, reg.cls, None)
            scope.availability = reg.availability
            self._walk(entry, scope, True, key)
        else:
            #Constants and root globals land on the root table.
            self._walk(entry, self.root, False, key)

    def _reportUnregistered(self):
        """
        Setup functions which build a table but are never called from either
        VM. Their bindings are unreachable from script.
        """
        for (cls, function) in sorted(self.index.functions):
            if function not in ('setupDelegateTable', 'setupTable', 'setupClass'):
                continue
            if (cls, function) in self.visited:
                continue
            relPath, _, body = self.index.functions[(cls, function)]
            count = sum(len(ADD_FUNCTION.findall(text)) for text, _, _ in body)
            if count == 0:
                continue
            self.warnings.append(
                "%s::%s defines %d binding(s) but is never called from a VM setup; "
                "unreachable from script (%s)" % (cls, function, count, relPath))

    def _findOrCreate(self, name, kind, owner, parent):
        for scope in self.scopes:
            if (scope.name == name and scope.kind == kind
                    and scope.parent is parent and scope.vm == self._vm):
                return scope
        return self.addScope(Scope(name, kind, owner, parent, vm=self._vm))

    def _findExisting(self, name, kind):
        for scope in self.scopes:
            if scope.name == name and scope.kind == kind and scope.vm == self._vm:
                return scope
        return None

    def _merge(self, target, source):
        target.functions.extend(source.functions)
        target.constants.extend(source.constants)
        target.sourceFiles |= source.sourceFiles
        if target.typeTag is None:
            target.typeTag = source.typeTag
        if not target.availability:
            target.availability = source.availability

    def _walk(self, entry, baseScope, claimFirstTable, functionKey=None):
        relPath, _, body = entry
        if functionKey is not None:
            self.visited.add(functionKey)

        key = (self._vm, relPath, baseScope.name, baseScope.kind)
        if key in self._visiting:
            return
        self._visiting.add(key)
        try:
            self._walkBody(relPath, body, baseScope, claimFirstTable)
        finally:
            self._visiting.discard(key)

    def _walkBody(self, relPath, body, baseScope, claimFirstTable):
        stack = [baseScope]
        pendingName = None
        locals_ = {}

        #An object setup function may build several sibling delegate tables.
        #Each starts life unnamed and adopts the name of the variable it is
        #captured into; anything never captured falls back to the base scope's
        #identity, which is the common single table case.
        buildsObjects = claimFirstTable and baseScope.kind in ('object', 'class')
        #A class or delegate setup often names its table with sq_pushstring
        #before creating it, e.g. FileClass pushes "File" then sq_newclass. That
        #string names the object, so the first table must not be read as a
        #nested namespace.
        objectTableClaimed = not buildsObjects
        unnamedTables = []
        arrayAliases = {}

        baseScope.sourceFiles.add(relPath)

        for rawText, lineNo, viaMacro in body:
            current = stack[-1] if stack else baseScope
            #Structural detection must ignore anything commented out.
            text = stripLineComment(rawText)

            decl = LOCAL_DECL.match(text)
            if decl:
                locals_[decl.group(2)] = decl.group(1)

            alias = ARRAY_ALIAS.search(text)
            if alias:
                arrayAliases[alias.group(1)] = alias.group(2)

            pushString = PUSH_STRING.search(text)
            if pushString:
                pendingName = pushString.group(1)

            if PUSH_ROOT.search(text):
                stack.append(self.root)
                pendingName = None
                continue

            if NEW_TABLE.search(text):
                if buildsObjects and not objectTableClaimed:
                    #The first table of a delegate/class setup is the object
                    #itself; a pending string is its script name.
                    objectTableClaimed = True
                    table = self.addScope(
                        Scope(None, baseScope.kind, baseScope.owner, None, vm=self._vm))
                    table.availability = baseScope.availability
                    table.sourceFiles.add(relPath)
                    unnamedTables.append({"scope": table, "pushName": pendingName,
                                          "captureVar": None})
                    stack.append(table)
                elif pendingName:
                    nested = self._findOrCreate(pendingName, 'namespace', baseScope.owner, current)
                    nested.sourceFiles.add(relPath)
                    stack.append(nested)
                elif buildsObjects:
                    #A further delegate table; its identity is resolved once the
                    #whole body has been seen.
                    table = self.addScope(
                        Scope(None, baseScope.kind, baseScope.owner, None, vm=self._vm))
                    table.availability = baseScope.availability
                    table.sourceFiles.add(relPath)
                    unnamedTables.append({"scope": table, "pushName": None,
                                          "captureVar": None})
                    stack.append(table)
                else:
                    anon = self.addScope(
                        Scope(None, 'anonymous', baseScope.owner, current, vm=self._vm))
                    anon.sourceFiles.add(relPath)
                    stack.append(anon)
                pendingName = None
                continue

            captured = GET_STACK_OBJ.search(text)
            if captured and current.name is None:
                #Record the capture, but hold off naming: a variable name is
                #only a better identity than the registration's own when the
                #body builds more than one table.
                for entry in unnamedTables:
                    if entry["scope"] is current:
                        entry["captureVar"] = captured.group(1)
                        break

            typeTag = SET_TYPETAG.search(text)
            if typeTag and current is not self.root:
                current.typeTag = typeTag.group(1)

            for match in ADD_FUNCTION.finditer(text):
                cFunction, name = match.group(1), match.group(2)
                numParams = int(match.group(3)) if match.group(3) else None
                typemask = match.group(4)
                current.functions.append(Binding(
                    name, cFunction, numParams, typemask, relPath, lineNo, viaMacro))
                current.sourceFiles.add(relPath)

                bad = maskProblems(typemask)
                if bad:
                    self.warnings.append(
                        "%s:%d '%s' has an invalid typemask '%s' (bad character%s %s)"
                        % (relPath, lineNo, name, typemask,
                           's' if len(bad) > 1 else '',
                           ', '.join("'%s'" % c for c in bad)))

            for match in FUNCTION_MAP_ENTRY.finditer(text):
                name, typemask, numParams, cFunction = match.groups()
                current.functions.append(Binding(
                    name, cFunction, int(numParams), typemask or None,
                    relPath, lineNo, viaMacro))
                current.sourceFiles.add(relPath)

            #The test namespaces are declared as a table of setup function and
            #name, then registered in a loop, so the names never reach a
            #sq_pushstring the walker could see.
            testEntry = TEST_NAMESPACE_ENTRY.search(text)
            if testEntry:
                cls, function, name = testEntry.groups()
                nestedEntry = self.index.lookup(cls, function)
                if nestedEntry is not None:
                    nested = self._findOrCreate(name, 'namespace', cls, current)
                    nested.availability = current.availability
                    nested.sourceFiles.add(relPath)
                    self._walk(nestedEntry, nested, False, (cls, function))
                continue

            self._readConstants(text, relPath, lineNo, current, arrayAliases)

            delegate = DELEGATE_SETUP.search(text)
            if delegate:
                cls, function = delegate.group(2), delegate.group(3)
                nestedEntry = self.index.lookup(cls, function)
                if nestedEntry is None:
                    self.warnings.append("No body found for %s::%s" % (cls, function))
                else:
                    name = _objectNameFor(cls, function)
                    scope = self._findOrCreate(name, 'object', cls, None)
                    self._walk(nestedEntry, scope, True, (cls, function))
                continue

            staticCall = STATIC_SETUP_CALL.match(text)
            if staticCall:
                cls, function = staticCall.group(1), staticCall.group(2)
                if cls != 'ScriptUtils':
                    nestedEntry = self.index.lookup(cls, function)
                    if nestedEntry is not None:
                        self._walk(nestedEntry, current, False, (cls, function))
                continue

            instanceCall = INSTANCE_SETUP_CALL.match(text)
            if instanceCall:
                varName, function = instanceCall.group(1), instanceCall.group(2)
                cls = locals_.get(varName)
                if cls:
                    nestedEntry = self.index.lookup(cls, function)
                    if nestedEntry is not None:
                        self._walk(nestedEntry, current, False, (cls, function))
                continue

            if NEW_SLOT.search(text):
                if len(stack) > 1:
                    stack.pop()
                pendingName = None
                continue

            popMatch = POP.search(text)
            if popMatch:
                for _ in range(int(popMatch.group(1))):
                    if len(stack) > 1:
                        stack.pop()
                pendingName = None

        self._nameTables(unnamedTables, baseScope)

    def _nameTables(self, tables, baseScope):
        """
        Resolve the identity of the delegate tables a setup function built.

        An explicit sq_pushstring name is authoritative; it is the script name.
        Otherwise a body that builds one table is describing the type it was
        registered as, and the registration name is the better one:
        NavMeshUserData captures into meshDelegateTableObject but the type is
        NavMesh, not Mesh. A body that builds several distinct types can only be
        told apart by their capture variables.
        """
        tables = [t for t in tables if t["scope"].functions or t["scope"].constants]
        if not tables:
            return

        useVariableNames = len(tables) > 1

        for entry in tables:
            table = entry["scope"]
            name = entry["pushName"]
            if name is None and useVariableNames and entry["captureVar"]:
                name = _nameFromTableVariable(entry["captureVar"])
            if name is None:
                name = baseScope.name

            existing = self._findExisting(name, table.kind)
            if existing is not None and existing is not table:
                self._merge(existing, table)
                if table in self.scopes:
                    self.scopes.remove(table)
            else:
                table.name = name

    def _readConstants(self, text, relPath, lineNo, scope, arrayAliases=None):
        for match in DECLARE_CONSTANT.finditer(text):
            literal, identifier, value = match.group(1), match.group(2), match.group(3)
            if literal is not None:
                scope.constants.append(Constant(literal, value, relPath, lineNo))
                continue

            #Loop registered constants, e.g. declareConstant(vm, InputNamesKeys[i], ...).
            arrayName = identifier.split('[')[0]
            if arrayAliases:
                arrayName = arrayAliases.get(arrayName, arrayName)
            values = self.index.stringArrays.get(arrayName)
            if values:
                for name in values:
                    scope.constants.append(
                        Constant(name, None, relPath, lineNo, fromArray=arrayName))
            else:
                self.warnings.append(
                    "Could not resolve constant names from '%s' at %s:%d"
                    % (identifier, relPath, lineNo))


def extract(srcRoot, registrations):
    index = SourceIndex(srcRoot)
    extractor = Extractor(index)
    extractor.run(registrations)
    return extractor
