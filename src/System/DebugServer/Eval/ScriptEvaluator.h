#ifdef DEBUG_SERVER

#pragma once

#include <squirrel.h>
#include <rapidjson/document.h>
#include <string>

namespace AV{
    /**
    Compiles and runs a Squirrel snippet against the engine's VM, serialising the
    result to JSON for the debug server's /api/eval endpoint.

    Must run on the main thread (via the MainThreadQueue): the snippet executes in the
    root table with full access to every engine namespace, exactly like project scripts.

    Error handling deliberately avoids the VM's installed handlers: compilation and
    calls pass raiseerror=SQFalse and read sq_getlasterror instead. This matters beyond
    tidiness — the engine's compiler error handler raises a test-failure event under
    TEST_MODE, which would kill a test run over an agent's typo.
    */
    class ScriptEvaluator{
    public:
        /**
        Evaluate code and populate doc:
          { "ok": true,  "result": <json>, "prints": ["..."] }
          { "ok": false, "error": "compile|runtime error: ...", "prints": ["..."] }

        The snippet is first compiled as an expression ("return (<code>);") so bare
        expressions like "1+1" yield a result; if that fails to parse it is compiled
        as raw statements, where an explicit return produces the result.
        */
        static void eval(HSQUIRRELVM vm, const std::string& code, rapidjson::Document& doc);
    };
}

#endif
