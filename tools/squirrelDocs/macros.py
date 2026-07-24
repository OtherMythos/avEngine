"""
Expansion of the parameterless concatenation macros the engine uses to share
binding lists between delegate tables.

GuiWidgetDelegate and friends define blocks such as BASIC_WIDGET_FUNCTIONS and
paste them into several setup functions. Without expansion every widget method
would be invisible to the extractor.

Only parameterless macros are expanded. The function-like macros in the engine
(COLLISION_FUNCTIONS, COMP_FUNCTIONS) build SQFUNCTION arrays and register
nothing, so they are deliberately ignored.
"""

import re

DEFINE_START = re.compile(r'^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s*(\(|\s|$)')


class Macro:
    def __init__(self, name, lines, line):
        self.name = name
        #The body as (text, definitionLine) pairs. The definition line matters
        #because a doc comment for a shared binding belongs beside the macro
        #body, not beside each of its expansion sites.
        self.lines = lines
        #Where the macro was defined, for attribution.
        self.line = line


def collectMacros(lines):
    """
    Find every object-like macro whose body may span continuation lines.

    Returns (macros, definitionLines) where macros maps name -> Macro for the
    parameterless ones, and definitionLines is the set of 0-indexed line
    numbers occupied by any #define. Callers must suppress those lines, since
    a macro body containing bindings would otherwise be counted both at its
    definition and at each expansion.
    """
    macros = {}
    definitionLines = set()

    i = 0
    while i < len(lines):
        line = lines[i]
        match = DEFINE_START.match(line)
        if not match:
            i += 1
            continue

        name = match.group(1)
        isFunctionLike = match.group(2) == '('

        body = []
        startLine = i

        #Strip everything up to and including the macro name from the first line.
        headEnd = line.find(name) + len(name)
        current = line[headEnd:]

        if isFunctionLike:
            #Skip past the parameter list; these macros are not expanded.
            closing = current.find(')')
            if closing >= 0:
                current = current[closing + 1:]

        while True:
            definitionLines.add(i)
            continues = current.rstrip().endswith('\\')
            body.append((current.rstrip().rstrip('\\'), i + 1))
            if not continues:
                break
            i += 1
            if i >= len(lines):
                break
            current = lines[i]

        if not isFunctionLike:
            macros[name] = Macro(name, body, startLine + 1)

        i += 1

    return macros, definitionLines


def expandLine(line, macros, depth=0):
    """
    Replace a bare macro invocation with the macro body.
    Returns a list of (text, viaMacro, definitionLine) tuples; viaMacro is None
    when the text came from the original line rather than a macro body.
    """
    if depth > 8:
        return [(line, None, None)]

    stripped = line.strip().rstrip(';')
    macro = macros.get(stripped)
    if macro is None:
        return [(line, None, None)]

    result = []
    for bodyLine, defLine in macro.lines:
        if not bodyLine.strip():
            continue
        #Macro bodies may themselves reference other macros.
        for text, origin, originLine in expandLine(bodyLine, macros, depth + 1):
            if origin is None:
                result.append((text, macro.name, defLine))
            else:
                result.append((text, origin, originLine))

    return result


def expandFile(lines):
    """
    Expand every parameterless macro invocation in a file.

    Returns a list of (text, sourceLine, viaMacro) tuples. sourceLine is the
    1-indexed line to attribute the text to: the line in the original file for
    ordinary code, or the line inside the macro body for expanded text, so doc
    comments written beside a shared binding still find it. viaMacro names the
    macro when the text came from one.
    """
    macros, definitionLines = collectMacros(lines)

    out = []
    for i, line in enumerate(lines):
        if i in definitionLines:
            #Preserve line alignment without re-registering the macro body.
            out.append(('', i + 1, None))
            continue

        expanded = expandLine(line, macros)
        if len(expanded) == 1 and expanded[0][1] is None:
            out.append((line, i + 1, None))
            continue
        for text, viaMacro, defLine in expanded:
            out.append((text, defLine if defLine is not None else i + 1, viaMacro))

    return out
