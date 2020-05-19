#ifdef false

#pragma once

namespace AV {
    class ImguiEntityView{
    public:
        ImguiEntityView();

        static void prepareGui(bool* pOpen);

    private:
        static void _drawChunkCanvas();
    };
}

#endif
