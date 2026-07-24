#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>
#include <string>

namespace AV{
    /**
    Serialises the Colibri GUI: the window/widget hierarchy, visible labels, and hit
    testing. Read-only; driving the GUI (clicks etc.) is the input API's job.

    Runs on the main thread via the MainThreadQueue. Coordinates are reported both in
    Colibri canvas units and normalised 0-1 (canvas-relative), the latter matching the
    space /api/input/mouse and render captures use.
    */
    class GuiInspector{
    public:
        /**
        The window/widget hierarchy.
        @param windowId Optional widget id to root the dump at; empty dumps all top-level windows.
        @param status 404 if windowId is given but not found.
        */
        static void writeTree(rapidjson::Document& doc, int& status, const std::string& windowId,
                              int maxDepth, int maxNodes, bool visibleOnly);

        /**
        A flat list of every text-bearing widget (Label, and the labels inside
        Button/Editbox) with its text and position.
        */
        static void writeLabels(rapidjson::Document& doc, bool visibleOnly);

        /**
        Which widgets contain a normalised (0-1) point, outermost first.
        */
        static void writeHitTest(rapidjson::Document& doc, int& status, float normX, float normY);

        /**
        Single-widget deep dive by id.
        @param status 404 if the id is unknown/stale.
        */
        static void writeWidget(rapidjson::Document& doc, int& status, const std::string& widgetId);
    };
}

#endif
