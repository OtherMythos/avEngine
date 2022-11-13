#include "GuiSizerDelegate.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "ColibriGui/ColibriWidget.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    void GuiSizerDelegate::setupLayoutLine(HSQUIRRELVM vm){
        sq_newtableex(vm, 1);

        ScriptUtils::addFunction(vm, layout, "layout");
        ScriptUtils::addFunction(vm, addCell, "addCell", 2, ".u");
        ScriptUtils::addFunction(vm, setCellOffset, "setCellOffset", -2, ".u|nn");
        ScriptUtils::addFunction(vm, setMarginForAllCells, "setMarginForAllCells", -2, ".u|nn");
        ScriptUtils::addFunction(vm, setGridLocationForAllCells, "setGridLocationForAllCells", 2, ".i");
        ScriptUtils::addFunction(vm, setHardMaxSize, "setHardMaxSize", -2, ".u|nn");
        ScriptUtils::addFunction(vm, setCellMinSize, "setCellMinSize", 3, ".iu");
        ScriptUtils::addFunction(vm, setCellSize, "setSize", -2, ".u|nn");
        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".u|nn");
        ScriptUtils::addFunction(vm, getNumCells, "getNumCells");
        ScriptUtils::addFunction(vm, setCellExpandVertical, "setCellExpandVertical", 3, ".ib");
        ScriptUtils::addFunction(vm, setCellExpandHorizontal, "setCellExpandHorizontal", 3, ".ib");
        ScriptUtils::addFunction(vm, setCellProportionVertical, "setCellProportionVertical", 3, ".ii");
        ScriptUtils::addFunction(vm, setCellProportionHorizontal, "setCellProportionHorizontal", 3, ".ii");
        ScriptUtils::addFunction(vm, setCellPriority, "setCellPriority", 3, ".ii");
    }

    #define GET_CELL_ID \
        SQInteger cellId; \
        sq_getinteger(vm, 2, &cellId); \
        if(cellId < 0 || cellId >= cells.size()) return sq_throwerror(vm, "Invalid cell id.");

    inline const Colibri::LayoutCellVec& _getCellsFromLayout(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Colibri::LayoutLine* lineLayout = dynamic_cast<Colibri::LayoutLine*>(layout);
        assert(lineLayout);
        const Colibri::LayoutCellVec& cells = lineLayout->getCells();

        return cells;
    }

    SQInteger GuiSizerDelegate::getNumCells(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);

        sq_pushinteger(vm, cells.size());

        return 1;
    }

    SQInteger GuiSizerDelegate::setCellExpandVertical(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        SQBool expand;
        sq_getbool(vm, 3, &expand);

        cells[cellId]->m_expand[1] = expand;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellExpandHorizontal(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        SQBool expand;
        sq_getbool(vm, 3, &expand);

        cells[cellId]->m_expand[0] = expand;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellProportionVertical(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        SQInteger proportion;
        sq_getinteger(vm, 3, &proportion);
        if(proportion < 0 || proportion >= 0xFFFF) return sq_throwerror(vm, "Priority must be between 0 and 255.");

        cells[cellId]->m_proportion[1] = proportion;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellProportionHorizontal(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        SQInteger proportion;
        sq_getinteger(vm, 3, &proportion);
        if(proportion < 0 || proportion >= 0xFFFF) return sq_throwerror(vm, "Priority must be between 0 and 255.");

        cells[cellId]->m_proportion[0] = proportion;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellPriority(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        SQInteger priority;
        sq_getinteger(vm, 3, &priority);
        if(priority < 0 || priority >= 256) return sq_throwerror(vm, "Priority must be between 0 and 255.");

        cells[cellId]->m_priority = static_cast<uint8_t>(priority);

        return 0;
    }

    SQInteger GuiSizerDelegate::setHardMaxSize(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        layout->m_hardMaxSize = outVec;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellMinSize(HSQUIRRELVM vm){
        const Colibri::LayoutCellVec& cells = _getCellsFromLayout(vm);
        GET_CELL_ID

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(Vector2UserData::readVector2FromUserData(vm, 3, &outVec));

        cells[cellId]->m_minSize = outVec;

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellSize(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        layout->setCellSize(outVec);

        return 0;
    }

    SQInteger GuiSizerDelegate::setPosition(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        layout->m_topLeft = outVec;

        return 0;
    }

    SQInteger GuiSizerDelegate::setMarginForAllCells(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        Colibri::LayoutLine* lineLayout = dynamic_cast<Colibri::LayoutLine*>(layout);
        assert(lineLayout);
        layout->setMarginToAllCells(lineLayout->getCells(), outVec);

        return 0;
    }

    SQInteger GuiSizerDelegate::setGridLocationForAllCells(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        SQInteger targetLocation;
        sq_getinteger(vm, 2, &targetLocation);

        if(targetLocation < 0 || targetLocation >= Colibri::GridLocations::NumGridLocations){
            return sq_throwerror(vm, "Invalid grid location provided.");
        }
        Colibri::GridLocations::GridLocations loc = static_cast<Colibri::GridLocations::GridLocations>(targetLocation);

        Colibri::LayoutLine* lineLayout = dynamic_cast<Colibri::LayoutLine*>(layout);
        assert(lineLayout);
        const Colibri::LayoutCellVec& cells = lineLayout->getCells();
        for(Colibri::LayoutCell* cell : cells){
            cell->m_gridLocation = loc;
        }

        return 0;
    }

    SQInteger GuiSizerDelegate::setCellOffset(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, 1, &layout));

        Ogre::Vector2 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read2FloatsOrVec2(vm, &outVec));

        layout->setCellOffset(outVec);

        return 0;
    }

    SQInteger GuiSizerDelegate::addCell(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, -2, &layout));

        Colibri::LayoutCell* target = 0;

        SQUserPointer typeTag;
        sq_gettypetag(vm, -1, &typeTag);
        if(typeTag == LayoutLineTypeTag){
            Colibri::LayoutBase* newLayout = 0;
            SCRIPT_CHECK_RESULT(GuiNamespace::getLayoutFromUserData(vm, -1, &newLayout));
            target = dynamic_cast<Colibri::LayoutCell*>(newLayout);

            if(layout == target) return sq_throwerror(vm, "Attempting to add a layout to itself");
        }else{
            Colibri::Widget* widget = 0;
            void* foundType = 0;
            SCRIPT_CHECK_RESULT(GuiNamespace::getWidgetFromUserData(vm, -1, &widget, &foundType));
            if(!GuiNamespace::isTypeTagWidget(foundType)) return sq_throwerror(vm, "Unknown object passed.");
            target = dynamic_cast<Colibri::LayoutCell*>(widget);
        }

        Colibri::LayoutLine* lineLayout = dynamic_cast<Colibri::LayoutLine*>(layout);
        assert(lineLayout);
        size_t numCells = lineLayout->getCells().size();
        lineLayout->addCell(target);
        sq_pushinteger(vm, numCells);

        return 1;
    }

    SQInteger GuiSizerDelegate::layout(HSQUIRRELVM vm){
        Colibri::LayoutBase* layout = 0;
        SCRIPT_ASSERT_RESULT(GuiNamespace::getLayoutFromUserData(vm, -1, &layout));

        ((Colibri::LayoutLine*)layout)->layout();

        return 0;
    }

    void GuiSizerDelegate::setupConstants(HSQUIRRELVM vm){
        /**SQNamespace
        @name _gui
        */
        /**SQConstant
        @name _LAYOUT_VERTICAL
        */
        ScriptUtils::declareConstant(vm, "_LAYOUT_VERTICAL", 0);
        /**SQConstant
        @name _LAYOUT_HORIZONTAL
        */
        ScriptUtils::declareConstant(vm, "_LAYOUT_HORIZONTAL", 1);
        /**SQConstant
        @name _TEXT_ALIGN_NATURAL
        @desc Align text according to the specified global alignment.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_NATURAL", 0);
        /**SQConstant
        @name _TEXT_ALIGN_LEFT
        @desc Align text to the left.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_LEFT", 1);
        /**SQConstant
        @name _TEXT_ALIGN_CENTER
        @desc Align text to the center.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_CENTER", 2);
        /**SQConstant
        @name _TEXT_ALIGN_RIGHT
        @desc Align text to the right.
        */
        ScriptUtils::declareConstant(vm, "_TEXT_ALIGN_RIGHT", 3);

        /**SQConstant
        @name _GRID_LOCATION_TOP_LEFT
        @desc Top left
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_TOP_LEFT", Colibri::GridLocations::TopLeft);
        /**SQConstant
        @name _GRID_LOCATION_TOP
        @desc Top
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_TOP", Colibri::GridLocations::Top);
        /**SQConstant
        @name _GRID_LOCATION_TOP_RIGHT
        @desc Top right
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_TOP_RIGHT", Colibri::GridLocations::TopRight);
        /**SQConstant
        @name _GRID_LOCATION_CENTER_LEFT
        @desc Center left
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_CENTER_LEFT", Colibri::GridLocations::CenterLeft);
        /**SQConstant
        @name _GRID_LOCATION_CENTER
        @desc Center
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_CENTER", Colibri::GridLocations::Center);
        /**SQConstant
        @name _GRID_LOCATION_CENTER_RIGHT
        @desc Center right
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_CENTER_RIGHT", Colibri::GridLocations::CenterRight);
        /**SQConstant
        @name _GRID_LOCATION_BOTTOM_LEFT
        @desc Bottom left
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_BOTTOM_LEFT", Colibri::GridLocations::BottomLeft);
        /**SQConstant
        @name _GRID_LOCATION_BOTTOM
        @desc Bottom
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_BOTTOM", Colibri::GridLocations::Bottom);
        /**SQConstant
        @name _GRID_LOCATION_BOTTOM_RIGHT
        @desc Bottom Right
        */
        ScriptUtils::declareConstant(vm, "_GRID_LOCATION_BOTTOM_RIGHT", Colibri::GridLocations::BottomRight);
    }
}
