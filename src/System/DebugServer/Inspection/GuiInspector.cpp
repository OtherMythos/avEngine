#ifdef DEBUG_SERVER

#include "GuiInspector.h"

#include "System/BaseSingleton.h"
#include "Gui/GuiManager.h"
#include "Scripting/ScriptNamespace/GuiNamespace.h"

#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriEditbox.h"

#include <vector>
#include <set>
#include <cstdlib>

namespace AV{
    //---- helpers ---------------------------------------------------------------------

    static const char* stateToString(Colibri::States::States state){
        switch(state){
            case Colibri::States::Disabled: return "Disabled";
            case Colibri::States::Idle: return "Idle";
            case Colibri::States::HighlightedCursor: return "HighlightedCursor";
            case Colibri::States::HighlightedButton: return "HighlightedButton";
            case Colibri::States::HighlightedButtonAndCursor: return "HighlightedButtonAndCursor";
            case Colibri::States::Pressed: return "Pressed";
            default: return "Unknown";
        }
    }

    static const char* widgetTypeToString(GuiNamespace::WidgetType type){
        switch(type){
            case GuiNamespace::WidgetType::Button: return "Button";
            case GuiNamespace::WidgetType::Label: return "Label";
            case GuiNamespace::WidgetType::AnimatedLabel: return "AnimatedLabel";
            case GuiNamespace::WidgetType::Editbox: return "Editbox";
            case GuiNamespace::WidgetType::Slider: return "Slider";
            case GuiNamespace::WidgetType::Checkbox: return "Checkbox";
            case GuiNamespace::WidgetType::Panel: return "Panel";
            case GuiNamespace::WidgetType::Spinner: return "Spinner";
            case GuiNamespace::WidgetType::Window: return "Window";
            default: return "Unknown";
        }
    }

    //Only widgets the engine created carry a valid m_userId. Internal Colibri sub-widgets
    //(e.g. a Button's own Label) default m_userId to 0, which collides with the first real
    //widget — so getWidgetData alone gives false positives. Verify the id round-trips back
    //to this exact pointer.
    static bool registeredWidgetData(Colibri::Widget* widget, GuiNamespace::GuiWidgetUserData** outData,
                                     GuiNamespace::WidgetId* outId){
        const GuiNamespace::WidgetId id = widget->m_userId;
        if(!GuiNamespace::getWidgetData(widget, outData)) return false;
        if(GuiNamespace::getDebugWidget(id) != widget) return false;
        *outId = id;
        return true;
    }

    //Best-effort type name: the engine's own widget type when registered, else the
    //Colibri render class.
    static std::string classifyType(Colibri::Widget* widget){
        GuiNamespace::GuiWidgetUserData* data;
        GuiNamespace::WidgetId id;
        if(registeredWidgetData(widget, &data, &id)){
            return widgetTypeToString(data->type);
        }
        if(widget->isWindow()) return "Window";
        if(widget->isLabel()) return "Label";
        return "Widget";
    }

    //Colibri derived coordinates are clip space [-1,1] with (-1,-1) at the top-left.
    static inline float clipToUnit(float v){ return (v + 1.0f) * 0.5f; }

    //Text of a text-bearing widget, without mutating it (Button::getLabel() would
    //allocate a label, so guard with hasLabel()).
    static bool widgetText(Colibri::Widget* widget, std::string& outText){
        if(Colibri::Label* label = dynamic_cast<Colibri::Label*>(widget)){
            outText = label->getText();
            return true;
        }
        if(Colibri::Button* button = dynamic_cast<Colibri::Button*>(widget)){
            if(button->hasLabel()){
                outText = button->getLabel()->getText();
                return true;
            }
            return false;
        }
        if(Colibri::Editbox* editbox = dynamic_cast<Colibri::Editbox*>(widget)){
            outText = editbox->getText();
            return true;
        }
        return false;
    }

    static Ogre::Vector2 canvasSize(){
        Colibri::ColibriManager* mgr = BaseSingleton::getGuiManager()->getColibriManager();
        Ogre::Vector2 c = mgr->getCanvasSize();
        if(c.x <= 0.0f) c.x = 1.0f;
        if(c.y <= 0.0f) c.y = 1.0f;
        return c;
    }

    //Writes the normalised (0-1) rect + center plus raw canvas-unit size into obj.
    static void writeGeometry(rapidjson::Value& obj, Colibri::Widget* widget,
                              rapidjson::Document::AllocatorType& allocator){
        const Ogre::Vector2 tl = widget->getDerivedTopLeft();
        const Ogre::Vector2 br = widget->getDerivedBottomRight();
        const Ogre::Vector2 size = widget->getSize();

        const float nx = clipToUnit(tl.x), ny = clipToUnit(tl.y);
        //A clip-space extent of 2.0 spans the whole canvas, so half the clip delta is
        //the 0-1 fraction.
        const float nw = (br.x - tl.x) * 0.5f, nh = (br.y - tl.y) * 0.5f;

        //Normalised [x, y, w, h] — same space as /api/input/mouse and render captures.
        rapidjson::Value rect(rapidjson::kArrayType);
        rect.PushBack(nx, allocator); rect.PushBack(ny, allocator);
        rect.PushBack(nw, allocator); rect.PushBack(nh, allocator);
        obj.AddMember("rect", rect, allocator);

        rapidjson::Value center(rapidjson::kArrayType);
        center.PushBack(nx + nw * 0.5f, allocator); center.PushBack(ny + nh * 0.5f, allocator);
        obj.AddMember("center", center, allocator);

        //Raw size in Colibri canvas units, for reference.
        rapidjson::Value sz(rapidjson::kArrayType);
        sz.PushBack(size.x, allocator); sz.PushBack(size.y, allocator);
        obj.AddMember("sizeCanvas", sz, allocator);
    }

    //Common per-widget fields shared by tree and widget-detail.
    static void writeWidgetCommon(rapidjson::Value& obj, Colibri::Widget* widget,
                                  bool effectiveVisible, rapidjson::Document::AllocatorType& allocator){
        GuiNamespace::GuiWidgetUserData* data;
        GuiNamespace::WidgetId id;
        const bool registered = registeredWidgetData(widget, &data, &id);
        if(registered) obj.AddMember("id", static_cast<uint64_t>(id), allocator);
        obj.AddMember("userId", registered ? data->userIdx : 0, allocator);

        obj.AddMember("type", rapidjson::Value(classifyType(widget).c_str(), allocator), allocator);

        if(widget->isWindow()){
            const std::string name = GuiNamespace::getDebugWindowName(static_cast<Colibri::Window*>(widget));
            if(!name.empty()) obj.AddMember("name", rapidjson::Value(name.c_str(), allocator), allocator);
        }

        obj.AddMember("hidden", widget->isHidden(), allocator);
        obj.AddMember("disabled", widget->isDisabled(), allocator);
        obj.AddMember("visible", effectiveVisible, allocator);
        obj.AddMember("state", rapidjson::Value(stateToString(widget->getCurrentState()), allocator), allocator);

        writeGeometry(obj, widget, allocator);

        std::string text;
        if(widgetText(widget, text)){
            obj.AddMember("text", rapidjson::Value(text.c_str(), allocator), allocator);
        }
    }

    //---- tree walk -------------------------------------------------------------------

    struct GuiWalkState{
        int maxDepth;
        int remaining;
        bool truncated;
        bool visibleOnly;
        const Ogre::Vector2& canvas;
        rapidjson::Document::AllocatorType& allocator;
    };

    static rapidjson::Value serialiseWidget(Colibri::Widget* widget, int depth, bool parentVisible, GuiWalkState& state){
        const bool effectiveVisible = parentVisible && !widget->isHidden();

        rapidjson::Value obj(rapidjson::kObjectType);
        writeWidgetCommon(obj, widget, effectiveVisible, state.allocator);

        const Colibri::WidgetVec& children = widget->getChildren();
        //Count children we would actually emit (respecting visibleOnly).
        size_t emittable = 0;
        for(Colibri::Widget* child : children){
            if(state.visibleOnly && child->isHidden()) continue;
            emittable++;
        }

        if(emittable > 0){
            if(depth >= state.maxDepth){
                obj.AddMember("childCount", static_cast<uint64_t>(emittable), state.allocator);
                state.truncated = true;
            }else{
                rapidjson::Value childArray(rapidjson::kArrayType);
                size_t serialised = 0;
                for(Colibri::Widget* child : children){
                    if(state.visibleOnly && child->isHidden()) continue;
                    if(state.remaining <= 0) break;
                    state.remaining--;
                    childArray.PushBack(serialiseWidget(child, depth + 1, effectiveVisible, state), state.allocator);
                    serialised++;
                }
                obj.AddMember("children", childArray, state.allocator);
                if(serialised < emittable){
                    obj.AddMember("childCount", static_cast<uint64_t>(emittable), state.allocator);
                    state.truncated = true;
                }
            }
        }

        return obj;
    }

    //Collect every window that is a descendant of another window, so we can pick the
    //true top-level roots regardless of Colibri's parent semantics.
    static void markDescendantWindows(Colibri::Widget* widget, std::set<Colibri::Window*>& out){
        for(Colibri::Widget* child : widget->getChildren()){
            if(child->isWindow()) out.insert(static_cast<Colibri::Window*>(child));
            markDescendantWindows(child, out);
        }
    }

    void GuiInspector::writeTree(rapidjson::Document& doc, int& status, const std::string& windowId,
                                 int maxDepth, int maxNodes, bool visibleOnly){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        if(!BaseSingleton::getGuiManager() || !BaseSingleton::getGuiManager()->getColibriManager()){
            status = 503;
            doc.AddMember("error", "gui not available", allocator);
            return;
        }

        const Ogre::Vector2 canvas = canvasSize();
        rapidjson::Value canvasArr(rapidjson::kArrayType);
        canvasArr.PushBack(canvas.x, allocator); canvasArr.PushBack(canvas.y, allocator);
        doc.AddMember("canvasSize", canvasArr, allocator);

        GuiWalkState state{maxDepth, maxNodes, false, visibleOnly, canvas, allocator};
        rapidjson::Value windows(rapidjson::kArrayType);

        if(!windowId.empty()){
            const GuiNamespace::WidgetId id = std::strtoull(windowId.c_str(), nullptr, 10);
            Colibri::Widget* target = GuiNamespace::getDebugWidget(id);
            if(!target){
                status = 404;
                doc.RemoveAllMembers();
                doc.AddMember("error", rapidjson::Value(("no widget with id " + windowId).c_str(), allocator), allocator);
                return;
            }
            state.remaining--;
            windows.PushBack(serialiseWidget(target, 0, true, state), allocator);
        }else{
            std::vector<Colibri::Window*> allWindows;
            GuiNamespace::getDebugWindows(allWindows);

            std::set<Colibri::Window*> childWindows;
            for(Colibri::Window* w : allWindows) markDescendantWindows(w, childWindows);

            for(Colibri::Window* w : allWindows){
                if(childWindows.count(w)) continue; //Not a top-level window.
                if(visibleOnly && w->isHidden()) continue;
                if(state.remaining <= 0){ state.truncated = true; break; }
                state.remaining--;
                windows.PushBack(serialiseWidget(w, 0, true, state), allocator);
            }
        }

        doc.AddMember("truncated", state.truncated, allocator);
        doc.AddMember("windows", windows, allocator);
    }

    //---- labels ----------------------------------------------------------------------

    static void collectLabels(Colibri::Widget* widget, bool parentVisible, bool visibleOnly,
                              rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator){
        const bool effectiveVisible = parentVisible && !widget->isHidden();

        //Only engine-created (registered) text widgets: this is what the project authored,
        //and it excludes the internal Label a Button/Editbox owns, so no duplicate text.
        GuiNamespace::GuiWidgetUserData* data;
        GuiNamespace::WidgetId id;
        std::string text;
        if((!visibleOnly || effectiveVisible) && registeredWidgetData(widget, &data, &id) && widgetText(widget, text)){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("id", static_cast<uint64_t>(id), allocator);
            obj.AddMember("type", rapidjson::Value(classifyType(widget).c_str(), allocator), allocator);
            obj.AddMember("text", rapidjson::Value(text.c_str(), allocator), allocator);
            obj.AddMember("visible", effectiveVisible, allocator);
            writeGeometry(obj, widget, allocator);
            out.PushBack(obj, allocator);
        }

        for(Colibri::Widget* child : widget->getChildren()){
            collectLabels(child, effectiveVisible, visibleOnly, out, allocator);
        }
    }

    void GuiInspector::writeLabels(rapidjson::Document& doc, bool visibleOnly){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        if(!BaseSingleton::getGuiManager() || !BaseSingleton::getGuiManager()->getColibriManager()){
            doc.AddMember("error", "gui not available", allocator);
            return;
        }

        const Ogre::Vector2 canvas = canvasSize();
        rapidjson::Value canvasArr(rapidjson::kArrayType);
        canvasArr.PushBack(canvas.x, allocator); canvasArr.PushBack(canvas.y, allocator);
        doc.AddMember("canvasSize", canvasArr, allocator);

        std::vector<Colibri::Window*> allWindows;
        GuiNamespace::getDebugWindows(allWindows);
        std::set<Colibri::Window*> childWindows;
        for(Colibri::Window* w : allWindows) markDescendantWindows(w, childWindows);

        rapidjson::Value labels(rapidjson::kArrayType);
        for(Colibri::Window* w : allWindows){
            if(childWindows.count(w)) continue;
            collectLabels(w, true, visibleOnly, labels, allocator);
        }
        doc.AddMember("labels", labels, allocator);
    }

    //---- hit test --------------------------------------------------------------------

    static void collectHits(Colibri::Widget* widget, float px, float py,
                            rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator){
        if(widget->isHidden()) return; //A hidden widget can't be hit; skip its subtree.

        const Ogre::Vector2 tl = widget->getDerivedTopLeft();
        const Ogre::Vector2 br = widget->getDerivedBottomRight();
        const float x0 = clipToUnit(tl.x), y0 = clipToUnit(tl.y);
        const float x1 = clipToUnit(br.x), y1 = clipToUnit(br.y);

        if(px >= x0 && px <= x1 && py >= y0 && py <= y1){
            rapidjson::Value obj(rapidjson::kObjectType);
            GuiNamespace::GuiWidgetUserData* data;
            GuiNamespace::WidgetId id;
            if(registeredWidgetData(widget, &data, &id)){
                obj.AddMember("id", static_cast<uint64_t>(id), allocator);
            }
            obj.AddMember("type", rapidjson::Value(classifyType(widget).c_str(), allocator), allocator);
            std::string text;
            if(widgetText(widget, text)) obj.AddMember("text", rapidjson::Value(text.c_str(), allocator), allocator);
            out.PushBack(obj, allocator); //Parent before child => outermost first.
        }

        for(Colibri::Widget* child : widget->getChildren()){
            collectHits(child, px, py, out, allocator);
        }
    }

    void GuiInspector::writeHitTest(rapidjson::Document& doc, int& status, float normX, float normY){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        if(!BaseSingleton::getGuiManager() || !BaseSingleton::getGuiManager()->getColibriManager()){
            status = 503;
            doc.AddMember("error", "gui not available", allocator);
            return;
        }

        rapidjson::Value point(rapidjson::kArrayType);
        point.PushBack(normX, allocator); point.PushBack(normY, allocator);
        doc.AddMember("point", point, allocator);

        std::vector<Colibri::Window*> allWindows;
        GuiNamespace::getDebugWindows(allWindows);
        std::set<Colibri::Window*> childWindows;
        for(Colibri::Window* w : allWindows) markDescendantWindows(w, childWindows);

        rapidjson::Value hits(rapidjson::kArrayType);
        for(Colibri::Window* w : allWindows){
            if(childWindows.count(w)) continue;
            collectHits(w, normX, normY, hits, allocator);
        }
        doc.AddMember("hits", hits, allocator);
    }

    //---- single widget ---------------------------------------------------------------

    void GuiInspector::writeWidget(rapidjson::Document& doc, int& status, const std::string& widgetId){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        if(!BaseSingleton::getGuiManager() || !BaseSingleton::getGuiManager()->getColibriManager()){
            status = 503;
            doc.AddMember("error", "gui not available", allocator);
            return;
        }

        const GuiNamespace::WidgetId id = std::strtoull(widgetId.c_str(), nullptr, 10);
        Colibri::Widget* widget = GuiNamespace::getDebugWidget(id);
        if(!widget){
            status = 404;
            doc.AddMember("error", rapidjson::Value(("no widget with id " + widgetId).c_str(), allocator), allocator);
            return;
        }

        const Ogre::Vector2 canvas = canvasSize();
        rapidjson::Value canvasArr(rapidjson::kArrayType);
        canvasArr.PushBack(canvas.x, allocator); canvasArr.PushBack(canvas.y, allocator);
        doc.AddMember("canvasSize", canvasArr, allocator);

        //Effective visibility: walk ancestors, stopping when a widget reports itself as
        //its own parent (Colibri roots) — bounded so a cycle can't hang us.
        bool effectiveVisible = !widget->isHidden();
        Colibri::Widget* p = widget->getParent();
        for(int guard = 0; guard < 64 && p && p != widget; guard++){
            if(p->isHidden()){ effectiveVisible = false; break; }
            Colibri::Widget* next = p->getParent();
            if(next == p) break;
            p = next;
        }

        writeWidgetCommon(doc, widget, effectiveVisible, allocator);

        //Parent + child ids — registered widgets only (internal sub-widgets have no id).
        GuiNamespace::GuiWidgetUserData* idata;
        GuiNamespace::WidgetId otherId;
        Colibri::Widget* parent = widget->getParent();
        if(parent && parent != widget && registeredWidgetData(parent, &idata, &otherId)){
            doc.AddMember("parentId", static_cast<uint64_t>(otherId), allocator);
        }
        rapidjson::Value childIds(rapidjson::kArrayType);
        for(Colibri::Widget* child : widget->getChildren()){
            if(registeredWidgetData(child, &idata, &otherId)){
                childIds.PushBack(static_cast<uint64_t>(otherId), allocator);
            }
        }
        doc.AddMember("childIds", childIds, allocator);
    }
}

#endif
