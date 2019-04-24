#pragma once

namespace AV{
    struct ScriptComponent {
        ScriptComponent(int id)
            : scriptId(id) {}
        int scriptId = -1;
    };
}
