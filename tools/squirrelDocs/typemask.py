"""
Translation of Squirrel typemasks into readable parameter types.

A typemask is the string passed to sq_setparamscheck, for instance ".n|unn".
Each position describes one parameter, and the first position is always the
invisible 'this' parameter. Alternatives are separated with |.

The parameter count passed alongside the mask is the total number of
parameters including 'this'. A negative count means "at least this many",
which is how the engine marks trailing optional parameters.
"""

#Squirrel type mask characters, as understood by sq_setparamscheck.
TYPE_CHARS = {
    'o': 'null',
    'i': 'int',
    'f': 'float',
    'n': 'number',
    's': 'string',
    't': 'table',
    'a': 'array',
    'u': 'userdata',
    'c': 'closure',
    'g': 'generator',
    'p': 'userpointer',
    'v': 'thread',
    'x': 'instance',
    'y': 'class',
    'b': 'bool',
    '.': 'any',
}


class TypemaskError(Exception):
    pass


def parseTypemask(mask, badChars=None):
    """
    Split a typemask into a list of slots, one per parameter.
    Each slot is a list of accepted type names.
    The leading 'this' slot is included, so callers should drop it.

    An unrecognised character is tolerated and rendered as 'unknown' so a single
    malformed mask never stops the whole run; pass a list as badChars to collect
    the offending characters for the linter to report.
    """
    if mask is None:
        return []

    slots = []
    current = []
    #True immediately after a '|', when the next type joins the current slot
    #rather than starting a new one.
    continuingSlot = True

    for char in mask:
        if char == '|':
            if not current:
                if badChars is not None:
                    badChars.append('|')
                continue
            continuingSlot = True
            continue

        if char.isspace():
            #Squirrel allows spaces in a mask to visually group a parameter's
            #alternatives; they simply end the current slot.
            continue

        if char in TYPE_CHARS:
            typeName = TYPE_CHARS[char]
        else:
            typeName = 'unknown'
            if badChars is not None:
                badChars.append(char)

        if not continuingSlot and current:
            #A type which did not follow a '|' begins the next parameter.
            slots.append(current)
            current = []

        current.append(typeName)
        continuingSlot = False

    if current:
        slots.append(current)

    return slots


def maskProblems(mask):
    """Return the list of characters in a mask Squirrel would not recognise."""
    badChars = []
    parseTypemask(mask, badChars)
    return badChars


def buildParams(numParams, typemask):
    """
    Produce a list of parameter descriptors from a binding's declared
    parameter count and typemask. The 'this' parameter is stripped.

    Each descriptor is {"types": [...], "optional": bool}.
    """
    slots = parseTypemask(typemask)

    #Drop the leading 'this' slot.
    if slots:
        slots = slots[1:]

    if numParams is None or numParams == 0:
        #No parameter check was registered, so nothing is known about arity.
        return [{"types": s, "optional": False} for s in slots], True

    if numParams < 0:
        #Negative means "at least abs(numParams)", including 'this'.
        required = abs(numParams) - 1
    else:
        required = numParams - 1

    params = []
    for i, types in enumerate(slots):
        params.append({"types": types, "optional": i >= required})

    #A mask may describe fewer slots than the count requires, in which case the
    #remaining parameters accept anything.
    while len(params) < required:
        params.append({"types": ['any'], "optional": False})

    return params, False


def formatSignature(name, numParams, typemask):
    """
    Render a call signature, for instance setMargin(userdata|number, [number]).
    """
    params, unchecked = buildParams(numParams, typemask)

    if unchecked and not params:
        return "%s(...)" % name

    rendered = []
    for p in params:
        text = '|'.join(p["types"])
        if p["optional"]:
            text = "[%s]" % text
        rendered.append(text)

    if unchecked:
        rendered.append("...")

    return "%s(%s)" % (name, ', '.join(rendered))
