"""
Parsing of the /**SQ... documentation comments.

A block opens with /**SQFunction, /**SQNamespace, /**SQConstant or /**SQObject,
runs until the closing */, and contains @tag values. A tag's value continues
across following lines until the next tag or the end of the block.

Several SQFunction blocks may precede a single binding. That is how overloads
are written, and each block describes one accepted call shape.

Recognised tags:
    @name           the script visible name
    @desc           prose description
    @param<N>       a parameter, optionally as @param<N>:label: description
    @returns        what the call evaluates to, optionally @returns:type:
    @example        a Squirrel snippet
    @see            a cross reference
    @property       a field reached through _get/_set, as @property name:type: desc
    @typeTag        the C++ type tag for an object, for cross referencing
    @returnedBy     a call which produces an object of this type
"""

import re

BLOCK_START = re.compile(r'/\*\*SQ(Function|Namespace|Constant|Object)\b')
BLOCK_END = '*/'

#The leading identifier of a tag. Everything after it, colons included, is the
#value, since params carry their label and description as :label:description.
TAG_HEAD = re.compile(r'^([A-Za-z_][A-Za-z0-9_]*)(.*)$', re.DOTALL)

PARAM_TAG = re.compile(r'^param(\d+)$')
#:label:description, where the description may follow the colon with no space,
#as most of the engine writes it.
LABELLED_VALUE = re.compile(r'^:([^:]*):(.*)$', re.DOTALL)
#@property x:float: The x component
PROPERTY_VALUE = re.compile(r'^([A-Za-z_][A-Za-z0-9_]*)\s*:\s*([^:]*?)\s*:\s*(.*)$', re.DOTALL)

KNOWN_TAGS = ('name', 'desc', 'example', 'see', 'typeTag', 'returnedBy', 'property')

#Common near misses, so the linter can suggest the intended tag.
TAG_SUGGESTIONS = {
    'return': 'returns',
    'ret': 'returns',
    'returrun': 'returns',
    'description': 'desc',
    'descripton': 'desc',
    'parameter': 'param1',
    'params': 'param1',
    'examples': 'example',
    'note': 'desc',
}


class DocIssue:
    def __init__(self, message, sourceFile, line):
        self.message = message
        self.sourceFile = sourceFile
        self.line = line

    def __str__(self):
        return "%s:%d %s" % (self.sourceFile, self.line, self.message)


class DocBlock:
    def __init__(self, kind, sourceFile, startLine, endLine):
        #Function | Namespace | Constant | Object
        self.kind = kind
        self.sourceFile = sourceFile
        self.startLine = startLine
        self.endLine = endLine

        self.name = None
        self.desc = None
        self.returns = None
        self.returnType = None
        self.typeTag = None
        #index -> {"label": str|None, "desc": str}
        self.params = {}
        self.examples = []
        self.see = []
        self.returnedBy = []
        #name -> {"type": str, "desc": str}
        self.properties = {}

    def orderedParams(self):
        return [self.params[i] | {"index": i} for i in sorted(self.params)]


def _splitTags(lines):
    """
    Turn the body of a block into an ordered list of (tag, value) pairs.
    A value runs until the next line that starts a tag.
    """
    tags = []
    currentTag = None
    currentValue = []

    for line in lines:
        stripped = line.strip()
        #Tolerate the javadoc style leading asterisk.
        if stripped.startswith('*') and not stripped.startswith('*/'):
            stripped = stripped[1:].strip()

        if stripped.startswith('@'):
            if currentTag is not None:
                tags.append((currentTag, '\n'.join(currentValue).strip()))
            head = TAG_HEAD.match(stripped[1:])
            if head:
                currentTag = head.group(1)
                rest = head.group(2)
                #A space after the tag separates it from the value; a colon is
                #part of the value and must be kept.
                if rest.startswith(' '):
                    rest = rest[1:]
                currentValue = [rest] if rest else []
            else:
                currentTag = ''
                currentValue = []
        elif currentTag is not None:
            currentValue.append(stripped)

    if currentTag is not None:
        tags.append((currentTag, '\n'.join(currentValue).strip()))

    return tags


def _labelAndDesc(value):
    """Split a :label:description value; either part may be empty."""
    match = LABELLED_VALUE.match(value)
    if not match:
        return None, value.strip()
    label = match.group(1).strip() or None
    return label, match.group(2).strip()


def _applyTag(block, tag, value, issues):
    param = PARAM_TAG.match(tag)
    if param:
        index = int(param.group(1))
        label, desc = _labelAndDesc(value)
        if index in block.params:
            issues.append(DocIssue(
                "duplicate @param%d; the later one overwrites the earlier" % index,
                block.sourceFile, block.startLine))
        block.params[index] = {"label": label, "desc": desc}
        return

    if tag == 'returns':
        returnType, desc = _labelAndDesc(value)
        block.returns = desc
        block.returnType = returnType
        return

    if tag == 'name':
        block.name = value
    elif tag == 'desc':
        block.desc = value
    elif tag == 'example':
        block.examples.append(value)
    elif tag == 'see':
        block.see.append(value)
    elif tag == 'typeTag':
        block.typeTag = value
    elif tag == 'returnedBy':
        block.returnedBy.append(value)
    elif tag == 'property':
        match = PROPERTY_VALUE.match(value)
        if match:
            block.properties[match.group(1)] = {
                "type": match.group(2) or None,
                "desc": match.group(3),
            }
        else:
            issues.append(DocIssue(
                "@property should read 'name:type: description', got '%s'" % value,
                block.sourceFile, block.startLine))
    else:
        suggestion = TAG_SUGGESTIONS.get(tag)
        hint = (" (did you mean @%s?)" % suggestion) if suggestion else ""
        issues.append(DocIssue(
            "unknown tag @%s%s" % (tag, hint), block.sourceFile, block.startLine))


def parseFile(path, relPath, lines=None):
    """
    Returns (blocks, issues) for one source file.
    """
    if lines is None:
        with open(path, 'r', errors='replace') as f:
            lines = f.readlines()

    blocks = []
    issues = []

    i = 0
    while i < len(lines):
        match = BLOCK_START.search(lines[i])
        if not match:
            i += 1
            continue

        #A marker sitting inside a line comment is a disabled block, not a real
        #one; the engine keeps a few of these around.
        commentPos = lines[i].find('//')
        if commentPos != -1 and commentPos < match.start():
            i += 1
            continue

        kind = match.group(1)
        startLine = i + 1

        #The opening line may carry content after the marker.
        body = [lines[i][match.end():]]
        j = i
        closed = False
        while j < len(lines):
            if BLOCK_END in (body[-1] if j == i else lines[j]):
                if j > i:
                    body.append(lines[j][:lines[j].find(BLOCK_END)])
                else:
                    body[-1] = body[-1][:body[-1].find(BLOCK_END)]
                closed = True
                break
            if j > i:
                body.append(lines[j])
            j += 1

        if not closed:
            issues.append(DocIssue(
                "unterminated /**SQ%s block" % kind, relPath, startLine))
            i += 1
            continue

        block = DocBlock(kind, relPath, startLine, j + 1)
        for tag, value in _splitTags(body):
            _applyTag(block, tag, value, issues)

        if block.name is None and kind != 'Object':
            issues.append(DocIssue(
                "/**SQ%s block has no @name and will be ignored" % kind,
                relPath, startLine))
        else:
            blocks.append(block)

        i = j + 1

    return blocks, issues


def parseTree(srcRoot):
    """
    Parse every source file under srcRoot.
    Returns (blocksByFile, issues).
    """
    import os

    blocksByFile = {}
    issues = []

    for dirPath, _, files in os.walk(srcRoot):
        for name in files:
            if not name.endswith(('.cpp', '.h')):
                continue
            fullPath = os.path.join(dirPath, name)
            relPath = os.path.relpath(fullPath, srcRoot)
            blocks, fileIssues = parseFile(fullPath, relPath)
            if blocks:
                blocksByFile[relPath] = blocks
            issues.extend(fileIssues)

    return blocksByFile, issues
