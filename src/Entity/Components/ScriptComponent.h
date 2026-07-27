#pragma once

namespace AV{
    struct ScriptComponent {
        ScriptComponent(int id, bool hasUpdate = false)
            : scriptId(id), scriptHasUpdate(hasUpdate) {}
        int scriptId = -1;

        //Whether this script component contains a script with an update component.
        bool scriptHasUpdate = false;
    };
}
