"""
Reads ScriptVM.cpp, which is the authoritative manifest of the script API.

Everything reachable from Squirrel is registered from _setupVM (the main VM) or
_setupSetupFunctionVM (the reduced VM available to the project setup function).
The script visible name of a namespace lives here rather than in the namespace's
own source file, so this is the only place the mapping can be read from.
"""

import re

#{"_camera", CameraNamespace::setupNamespace},
NAMESPACE_ENTRY = re.compile(
    r'\{\s*"([^"]+)"\s*,\s*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\}')

#setupNamespace("_window", WindowNamespace::setupSetupFuncNamespace);
DIRECT_NAMESPACE = re.compile(
    r'setupNamespace\(\s*"([^"]+)"\s*,\s*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)\s*\)')

#DatablockUserData::setupDelegateTable(vm);
SETUP_CALL = re.compile(
    r'^\s*([A-Za-z_][A-Za-z0-9_]*)::(setupDelegateTable|setupTable|setupClass|setupConstants|setupFunctions)\s*\(\s*vm\s*\)\s*;')

IFDEF = re.compile(r'^\s*#\s*(ifdef|ifndef|if|else|elif|endif)\b\s*(.*)')

FUNCTION_START = re.compile(
    r'^\s*(?:void|SQInteger)\s+ScriptVM::(_setupVM|_setupSetupFunctionVM)\s*\(')


class Registration:
    """One entry point registered against a VM."""

    def __init__(self, cls, function, scriptName, kind, vm, availability, line):
        #C++ class owning the setup function, e.g. CameraNamespace.
        self.cls = cls
        #C++ setup function name, e.g. setupNamespace.
        self.function = function
        #Script visible name where one is known, e.g. _camera.
        self.scriptName = scriptName
        #namespace | object | constants | globals
        self.kind = kind
        #main | setup
        self.vm = vm
        #Preprocessor guards active at the registration site.
        self.availability = availability
        self.line = line

    def key(self):
        return (self.cls, self.function)


def _kindForFunction(function):
    if function == 'setupNamespace' or function == 'setupSetupFuncNamespace':
        return 'namespace'
    if function in ('setupDelegateTable', 'setupTable'):
        return 'object'
    if function == 'setupClass':
        return 'class'
    if function == 'setupConstants':
        return 'constants'
    return 'globals'


def parseManifest(path):
    """
    Returns a list of Registration objects in registration order.
    """
    with open(path, 'r') as f:
        lines = f.readlines()

    registrations = []
    guards = []
    currentVm = None
    depth = 0

    for i, line in enumerate(lines):
        start = FUNCTION_START.match(line)
        if start:
            currentVm = 'main' if start.group(1) == '_setupVM' else 'setup'
            depth = line.count('{') - line.count('}')
            guards = []
            continue

        if currentVm is None:
            continue

        depth += line.count('{') - line.count('}')
        if depth <= 0:
            #The setup function has closed.
            currentVm = None
            continue

        #Track the preprocessor guards in force at this point.
        guard = IFDEF.match(line)
        if guard:
            directive, condition = guard.group(1), guard.group(2).strip()
            if directive in ('ifdef', 'if'):
                guards.append(condition)
            elif directive == 'ifndef':
                guards.append('!' + condition)
            elif directive == 'else' and guards:
                guards[-1] = '!' + guards[-1]
            elif directive == 'endif' and guards:
                guards.pop()

        availability = list(guards)

        for match in NAMESPACE_ENTRY.finditer(line):
            registrations.append(Registration(
                match.group(2), match.group(3), match.group(1),
                'namespace', currentVm, availability, i + 1))

        for match in DIRECT_NAMESPACE.finditer(line):
            registrations.append(Registration(
                match.group(2), match.group(3), match.group(1),
                'namespace', currentVm, availability, i + 1))

        setupCall = SETUP_CALL.match(line)
        if setupCall:
            cls, function = setupCall.group(1), setupCall.group(2)
            registrations.append(Registration(
                cls, function, None,
                _kindForFunction(function), currentVm, availability, i + 1))

    return registrations
