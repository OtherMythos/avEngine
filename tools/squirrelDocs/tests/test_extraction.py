"""
Extraction and doc-comment tests driven by small inline fixtures.

These do not need the engine tree; each writes a tiny source file that mirrors a
real engine idiom, so a regression in the walker fails here with a clear cause.
"""

import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import macros
import doccomments
from manifest import parseManifest
from bindings import extract, stripLineComment


def _write(root, relPath, content):
    path = os.path.join(root, relPath)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(content)


SCRIPT_VM = """
namespace AV {
    void ScriptVM::_setupVM(HSQUIRRELVM vm){
        sq_pushroottable(vm);
        const std::vector<NamespaceEntry> namespaces = {
            {"_camera", CameraNamespace::setupNamespace},
        };
        for(const NamespaceEntry& e : namespaces){ setupNamespace(e.first, e.second); }
        WidgetDelegate::setupDelegateTable(vm);
        sq_pop(vm,1);
    }
}
"""

CAMERA = """
namespace AV {
    /**SQNamespace
    @name _camera
    @desc Controls the camera.
    */
    void CameraNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name setPosition
        @desc Set position with three floats.
        @param1:x: x position
        */
        /**SQFunction
        @name setPosition
        @desc Set position with a vector.
        @param1:Vec3: the position
        */
        ScriptUtils::addFunction(vm, setPos, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, getPos, "getPosition");
    }
}
"""

#A delegate built from a shared macro, mirroring GuiWidgetDelegate.
WIDGET = """
namespace AV {
    #define SHARED_FUNCTIONS \\
        ScriptUtils::addFunction(vm, a, "shared1"); \\
        ScriptUtils::addFunction(vm, b, "shared2");

    void WidgetDelegate::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);
        SHARED_FUNCTIONS
        ScriptUtils::addFunction(vm, c, "own");
        //ScriptUtils::addFunction(vm, dead, "commentedOut");
        sq_resetobject(&widgetDelegateTableObject);
        sq_getstackobj(vm, -1, &widgetDelegateTableObject);
        sq_settypetag(vm, -1, WidgetTypeTag);
        sq_pop(vm, 1);
    }
}
"""


class ExtractionTests(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.mkdtemp()
        _write(self.dir, "Scripting/ScriptVM.cpp", SCRIPT_VM)
        _write(self.dir, "Scripting/ScriptNamespace/CameraNamespace.cpp", CAMERA)
        _write(self.dir, "Scripting/ScriptNamespace/Classes/WidgetDelegate.cpp", WIDGET)

    def _extract(self):
        regs = parseManifest(os.path.join(self.dir, "Scripting/ScriptVM.cpp"))
        ex = extract(self.dir, regs)
        return {s.name: s for s in ex.scopes + list(ex.roots.values())
                if s.name is not None}, ex

    def test_namespace_and_functions_found(self):
        scopes, _ = self._extract()
        self.assertIn("_camera", scopes)
        names = [f.name for f in scopes["_camera"].functions]
        self.assertEqual(names, ["setPosition", "getPosition"])

    def test_macro_expansion_reveals_shared_bindings(self):
        scopes, _ = self._extract()
        widget = scopes["Widget"]
        names = [f.name for f in widget.functions]
        self.assertEqual(names, ["shared1", "shared2", "own"])

    def test_commented_binding_is_ignored(self):
        scopes, _ = self._extract()
        names = [f.name for f in scopes["Widget"].functions]
        self.assertNotIn("commentedOut", names)

    def test_type_tag_captured(self):
        scopes, _ = self._extract()
        self.assertEqual(scopes["Widget"].typeTag, "WidgetTypeTag")

    def test_object_named_from_capture_variable(self):
        scopes, _ = self._extract()
        #widgetDelegateTableObject -> Widget
        self.assertIn("Widget", scopes)


class DocCommentTests(unittest.TestCase):

    def test_overloads_kept_separate(self):
        with tempfile.TemporaryDirectory() as d:
            _write(d, "Camera.cpp", CAMERA)
            blocks, issues = doccomments.parseTree(d)
            camBlocks = [b for v in blocks.values() for b in v
                         if b.kind == 'Function' and b.name == 'setPosition']
            self.assertEqual(len(camBlocks), 2)
            self.assertEqual(camBlocks[0].params[1]["label"], "x")
            self.assertEqual(camBlocks[1].params[1]["label"], "Vec3")

    def test_glued_param_description(self):
        #The engine writes @param1:label:description with no space after label.
        blocks, _ = doccomments.parseFile(None, "x.cpp", lines=[
            "/**SQFunction\n", "@name f\n",
            "@param1:Vec3:Vector containing velocity.\n", "*/\n"])
        self.assertEqual(blocks[0].params[1]["label"], "Vec3")
        self.assertEqual(blocks[0].params[1]["desc"], "Vector containing velocity.")


class CommentStripTests(unittest.TestCase):

    def test_strips_line_comment(self):
        self.assertEqual(stripLineComment("code(); //note").strip(), "code();")

    def test_keeps_slashes_in_string(self):
        self.assertEqual(stripLineComment('f("http://x");'), 'f("http://x");')


if __name__ == "__main__":
    unittest.main()
