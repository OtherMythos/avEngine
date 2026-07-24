#!/usr/bin/env python3
"""
Generate documentation for the engine's Squirrel API.

The generator reads the engine source, extracts every script binding and its
documentation comment, and writes a Markdown reference, a JSON sidecar and a
coverage report.

    python3 generate.py --src ../../src --out ../../docs

Run with --check to verify the committed output is up to date, which is what CI
uses to stop the documentation drifting from the code.
"""

import argparse
import os
import sys

import emit_json
import emit_markdown
import report
from bindings import extract
from doccomments import parseTree
from manifest import parseManifest
from model import build

SCRIPT_VM = "Scripting/ScriptVM.cpp"


#Deliberately no commit stamp in the output. Embedding one would make the
#committed documentation differ from a fresh run after every unrelated commit,
#so --check would fail constantly instead of only when the API actually moved.


def buildApi(srcRoot):
    manifestPath = os.path.join(srcRoot, SCRIPT_VM)
    if not os.path.isfile(manifestPath):
        raise SystemExit("Could not find %s under %s" % (SCRIPT_VM, srcRoot))

    registrations = parseManifest(manifestPath)
    extractor = extract(srcRoot, registrations)
    blocksByFile, docIssues = parseTree(srcRoot)
    api = build(extractor, blocksByFile, docIssues)
    return api


def _write(path, content, check, changed):
    if check:
        existing = None
        if os.path.isfile(path):
            with open(path, 'r') as f:
                existing = f.read()
        if existing != content:
            changed.append(path)
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(content)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    here = os.path.dirname(os.path.abspath(__file__))
    parser.add_argument("--src", default=os.path.normpath(os.path.join(here, "..", "..", "src")),
                        help="Path to the engine src directory.")
    parser.add_argument("--out", default=os.path.normpath(os.path.join(here, "..", "..", "docs")),
                        help="Directory to write the generated files into.")
    parser.add_argument("--check", action="store_true",
                        help="Do not write; exit non-zero if committed output is stale.")
    parser.add_argument("--fail-on-issues", action="store_true",
                        help="Exit non-zero if any documentation problems are found.")
    parser.add_argument("--quiet", action="store_true", help="Only print on error.")
    args = parser.parse_args()

    srcRoot = os.path.abspath(args.src)
    outDir = os.path.abspath(args.out)

    api = buildApi(srcRoot)
    meta = {}

    markdown = emit_markdown.render(api, meta)
    jsonOut = emit_json.render(api, meta)
    coverage = report.render(api)

    changed = []
    _write(os.path.join(outDir, "squirrelApi.md"), markdown, args.check, changed)
    _write(os.path.join(outDir, "squirrelApi.json"), jsonOut, args.check, changed)
    _write(os.path.join(outDir, "squirrelApiCoverage.md"), coverage, args.check, changed)

    if not args.quiet:
        print(report.summarise(api))

    if args.check and changed:
        print("Generated documentation is out of date. Re-run the generator and "
              "commit the result. Stale files:", file=sys.stderr)
        for path in changed:
            print("  " + os.path.relpath(path, outDir), file=sys.stderr)
        return 1

    if args.fail_on_issues and (api.issues or api.orphanedBlocks):
        print("Documentation problems were found; see the coverage report.",
              file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
