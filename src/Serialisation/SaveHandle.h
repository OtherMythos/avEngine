#pragma once

#include <string>

namespace AV{
    class SaveHandle{
    public:
        SaveHandle();
        ~SaveHandle();

        std::string saveName;
    };
}
