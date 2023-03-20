#include "GuiManager.h"

#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/ColibriSkinManager.h"
#include "ColibriGui/Text/ColibriShaperManager.h"
#include "ColibriGui/Text/ColibriShaper.h"
#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"
#include "hb.h"

#include "Scripting/ScriptNamespace/GuiNamespace.h"
#include "System/BaseSingleton.h"
#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Window/Window.h"
#include "System/SystemSetup/SystemSettings.h"
#include "filesystem/path.h"

#include "OgreRoot.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"
#include "Gui/Rect2d/Compositor/CompositorPassRect2dProvider.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/DebuggerToolEvent.h"

#include "World/WorldSingleton.h"

const char* defaultSkin =
"{"
"    \"skins\" :"
"    {"
"        \"internal/ButtonSkin\" :"
"        {"
"            \"material\" : \"internal/ButtonSkin\","
"            \"tex_resolution\" : [64, 128],"
"            \"grid_uv\" :"
"            {"
"                \"center2\" : [0, 0, 64, 32],"
"                \"enclosing\" : [[0, 0, 64, 32], [4, 4]]"
"            },"
"            \"borders\" :"
"            {"
"                \"all\" : [5, 0]"
"            }"
"        },"
"        \"internal/PanelSkin\" :"
"        {"
"            \"material\" : \"internal/PanelSkin\","
"            \"borders\" :"
"            {"
"                \"all\" : [0, 0]"
"            }"
"        },"
"        \"internal/ButtonSkinDisabled\" :"
"        {"
"            \"copy_from\" : \"internal/ButtonSkin\","
"            \"material\" : \"internal/ButtonSkinDisabled\""
"        },"
"        \"internal/ButtonSkinHighlightedCursor\" :"
"        {"
"            \"copy_from\" : \"internal/ButtonSkin\","
"            \"material\" : \"internal/ButtonSkinBrighter\""
"        },"
"        \"internal/ButtonSkinHighlightedButton\" :"
"        {"
"            \"copy_from\" : \"internal/ButtonSkin\","
"            \"grid_uv\" :"
"            {"
"                \"enclosing\" : [[0, 32, 64, 32], [4, 4]]"
"            }"
"        },"
"        \"internal/ButtonSkinHighlightedButtonAndCursor\" :"
"        {"
"            \"copy_from\" : \"internal/ButtonSkinHighlightedButton\","
"            \"material\" : \"internal/ButtonSkinBrighter\""
"        },"
"        \"internal/ButtonSkinPressed\" :"
"        {"
"            \"copy_from\" : \"internal/ButtonSkin\","
"            \"material\" : \"internal/ButtonSkin\","
"            \"grid_uv\" :"
"            {"
"                \"enclosing\" : [[0, 0, 64, 32], [4, 4]]"
"            }"
"        },"
""
"        \"internal/WindowSkin\" :"
"        {"
""
"            \"material\" : \"internal/WindowMaterial\","
"            \"borders\" :"
"            {"
"                \"all\" : [10, 10]"
"            }"
"        },"
""
"        \"internal/CheckboxTickmarkChecked\" :"
"        {"
"            \"material\" : \"internal/TickMarkChecked\""
"        }"
"    },"
""
"    \"skin_packs\" :"
"    {"
"        \"internal/ButtonSkin\" :"
"        {"
"            \"all\" : \"internal/ButtonSkin\","
"            \"disabled\" : \"internal/ButtonSkinDisabled\","
"            \"idle\" : \"internal/ButtonSkin\","
"            \"highlighted_cursor\" : \"internal/ButtonSkinHighlightedCursor\","
"            \"highlighted_button\" : \"internal/ButtonSkinHighlightedButton\","
"            \"highlighted_button_and_cursor\" : \"internal/ButtonSkinHighlightedButtonAndCursor\","
"            \"pressed\" : \"internal/ButtonSkinPressed\""
"        },"
"        \"internal/WindowSkin\" :"
"        {"
"            \"all\" : \"internal/WindowSkin\""
"        },"
"        \"internal/PanelSkin\" :"
"        {"
"            \"all\" : \"internal/PanelSkin\""
"        },"
""
"        \"internal/CheckboxTicked\" :"
"        {"
"            \"all\" : \"internal/CheckboxTickmarkChecked\""
"        }"
"    },"
""
"    \"default_skin_packs\" :"
"    {"
"        \"Window\"                : \"internal/WindowSkin\","
"        \"Button\"                : \"internal/ButtonSkin\","
"        \"Spinner\"               : \"internal/ButtonSkin\","
"        \"SpinnerBtnDecrement\"   : \"internal/ButtonSkin\","
"        \"SpinnerBtnIncrement\"   : \"internal/ButtonSkin\","
"        \"Checkbox\"              : \"internal/ButtonSkin\","
"        \"CheckboxTickmarkUnchecked\"     : \"internal/WindowSkin\","
"        \"CheckboxTickmarkChecked\"       : \"internal/CheckboxTicked\","
"        \"CheckboxTickmarkThirdState\"    : \"internal/CheckboxTicked\","
"        \"Editbox\"               : \"internal/ButtonSkin\","
"        \"ProgressbarLayer0\"     : \"internal/ButtonSkin\","
"        \"ProgressbarLayer1\"     : \"internal/ButtonSkin\","
"        \"SliderLine\"            : \"internal/ButtonSkin\","
"        \"SliderHandle\"          : \"internal/ButtonSkin\""
"    }"
"}"
;

namespace AV{

    class GuiLogListener : public Colibri::LogListener{
        void log(const char *text, Colibri::LogSeverity::LogSeverity severity) {
            AV_ERROR(text);
        }
    };

    bool GuiManager::debuggerToolsReceiver(const Event &e){
        const DebuggerToolEvent& event = (DebuggerToolEvent&)e;
        if(event.eventId() == EventId::DebuggingToolToggle){
            const DebuggerToolEventToggle& toolEvent = (DebuggerToolEventToggle&)event;

            if(toolEvent.t == DebuggerToolToggle::StatsToggle){
                toggleDebugMenu();
            }
        }

        return false;
    }

    GuiManager::GuiManager(){

    }

    GuiManager::~GuiManager(){

    }

    static GuiLogListener colibriLogListener;
    static Colibri::ColibriListener colibriListener;
    void GuiManager::setupColibriManager(){
        mColibriManager = new Colibri::ColibriManager( &colibriLogListener, &colibriListener );
    }

    void GuiManager::setupCompositorProvider(Ogre::CompositorManager2* compMan){
        CompositorPassRect2dProvider *compoProvider = OGRE_NEW CompositorPassRect2dProvider(mColibriManager);
        compMan->setCompositorPassProvider(compoProvider);
    }

    void GuiManager::setup(Ogre::Root* root, Ogre::SceneManager* sceneManager){

        EventDispatcher::subscribe(EventType::DebuggerTools, AV_BIND(GuiManager::debuggerToolsReceiver));

        { //Process shapers
            Colibri::ShaperManager *shaperManager = mColibriManager->getShaperManager();

            const std::vector<SystemSettings::FontSettingEntry>& fontList = SystemSettings::getFontSettings();

            {
                //Load the default font regardless of whether any others are loaded. This is used for debug dialogs.
                const Ogre::String& masterPath = SystemSettings::getMasterPath();
                const filesystem::path defaultFontPath = filesystem::path(masterPath) / filesystem::path("essential/font/ProggyClean.ttf");
                assert(defaultFontPath.exists() && defaultFontPath.is_file());

                shaperManager->addShaper( HB_SCRIPT_LATIN, defaultFontPath.str().c_str(), "en");
            }
            bool fontAdded = false;
            for(const SystemSettings::FontSettingEntry& e : fontList){
                const filesystem::path defaultFontPath = filesystem::path(e.fontPath);
                if(!defaultFontPath.exists() || !defaultFontPath.is_file()) continue;
                //Latin for now.
                shaperManager->addShaper(HB_SCRIPT_LATIN, e.fontPath.c_str(), e.locale.c_str());
                fontAdded = true;
            }
            if(fontAdded){
                //Make the first provided font the default.
                //2 because the default font is loaded first, where it's set as font 0 (default) as well as 1.
                //After this the pointer to the first provided font is 2 and 0.
                shaperManager->setDefaultShaper(2, Colibri::HorizReadingDir::Default, false);
            }
        }

        { //Load skins
            //Load the default skin regardless as it is used for debug menus.
            _loadDefaultSkin();

            const std::vector<std::string>& skinList = SystemSettings::getGuiSkins();
            for(const std::string& s : skinList){
                const filesystem::path skinPath = filesystem::path(s);
                if(!skinPath.exists() || !skinPath.is_file()) continue;

                mColibriManager->loadSkins(s.c_str());
            }
        }


        mColibriManager->setOgre( root,
                     root->getRenderSystem()->getVaoManager(),
                     sceneManager );

        //The fist value represents virtual screen coordinates. The second is the actual resolution of the screen.
        //TODO in future I'd perfer not to have the singleton approach. Also remove the includes.
        mColibriManager->setCanvasSize( Ogre::Vector2( 1920.0f, 1080.0f ), Ogre::Vector2( BaseSingleton::getWindow()->getWidth(), BaseSingleton::getWindow()->getHeight() ) );


        /*Colibri::Window* mainWindow = mColibriManager->createWindow( 0 );
        mainWindow->setTransform( Ogre::Vector2( 0, 0 ), Ogre::Vector2( 850, 100 ) );

        Colibri::LayoutLine *layout = new Colibri::LayoutLine( mColibriManager );

        Colibri::Label *label = mColibriManager->createWidget<Colibri::Label>( mainWindow );
        //label->setText( "This is some text" );
        label->setText( "こんにちは" );
        //label->setSize( Ogre::Vector2(100, 20) );
        label->setShadowOutline( true, Ogre::ColourValue::Black, Ogre::Vector2( 1.0f ) );
        label->setTransform( Ogre::Vector2(10, 10), Ogre::Vector2(30, 10) );
        label->sizeToFit();
        label->setTopLeft(Ogre::Vector2(10, 10));
        layout->addCell(label);

        Colibri::Button* button0 = mColibriManager->createWidget<Colibri::Button>( mainWindow );
        button0->getLabel()->setText( "This is a button" );
        button0->sizeToFit();
        layout->addCell(button0);



        Colibri::Window* secondWindow = mColibriManager->createWindow( 0 );
        secondWindow->setTransform( Ogre::Vector2( 500, 500 ), Ogre::Vector2( 200, 200 ) );
        Colibri::Button* button1 = mColibriManager->createWidget<Colibri::Button>( secondWindow );
        button1->getLabel()->setText( "Another button" );
        button1->sizeToFit();
        layout->addCell(button1);

        layout->layout();*/

        //mainWindow->setHidden(true);
        //label->setHidden(true);
    }

    void GuiManager::update(float timeSinceLast){
        if(mDebugVisible){
            static int updateCounter = 0;
            updateCounter++;
            if(updateCounter % 4 == 0){
                _updateDebugMenuText();
            }
        }
        mColibriManager->update(timeSinceLast);
    }

    void GuiManager::shutdown(){
        EventDispatcher::unsubscribe(EventType::DebuggerTools, this);

        //TODO it might be nice if this call was somewhere else, for instance in the script manager.
        GuiNamespace::destroyStoredWidgets();

        if(mDebugMenuSetup){
            for(int i = 0; i < numDebugWindowLabels; i++){
                mColibriManager->destroyWidget(mDebugWindowLabels[i]);
            }
            mColibriManager->destroyWindow(mDebugWindow);
        }
        delete mColibriManager;
    }

    void GuiManager::showDebugMenu(bool show){
        mDebugVisible = show;
        if(mDebugVisible && !mDebugMenuSetup){
            _constructDebugWindow();
        }
        mDebugWindow->setHidden(!show);
        //Move it out of the way so it doesn't interfere with collision.
        mDebugWindow->setTopLeft(show ? Ogre::Vector2(0, 0) : Ogre::Vector2(-1000, -1000));
    }

    void GuiManager::_constructDebugWindow(){
        assert(mDebugWindow == 0 && !mDebugMenuSetup);
        mDebugWindow = mColibriManager->createWindow(0);
        mDebugWindow->setSkin("internal/WindowSkin");

        Colibri::LayoutLine *layout = new Colibri::LayoutLine(mColibriManager);

        for(int i = 0; i < numDebugWindowLabels; i++){
            Colibri::Label *label = mColibriManager->createWidget<Colibri::Label>( mDebugWindow );
            label->setText(" ");
            label->setSizeAndCellMinSize(Ogre::Vector2(500, 15.0f));
            label->setDefaultFontSize(15.0f);
            //Should always be the default engine font.
            label->setDefaultFont(1);
            layout->addCell(label);
            mDebugWindowLabels[i] = label;
        }
        mDebugWindow->setSize(Ogre::Vector2(400, 200));
        mDebugWindow->setZOrder(230);

        layout->layout();

        mDebugMenuSetup = true;
    }

    void GuiManager::_updateDebugMenuText(){
        const PerformanceStats& s = BaseSingleton::getPerformanceStats();
        mDebugWindowLabels[0]->setText("FPS: " + std::to_string(s.fps));
        mDebugWindowLabels[1]->setText("Average FPS: " + std::to_string(s.avgFPS));
        mDebugWindowLabels[2]->setText("Frame time: " + std::to_string(s.frameTime));

        std::ostringstream ss;
        ss << "Origin: ";
        WorldSingleton::getOrigin().printLeanStream(ss);
        mDebugWindowLabels[3]->setText(ss.str()); ss.str("");
        ss << "Player: ";
        WorldSingleton::getPlayerPosition().printLeanStream(ss);
        mDebugWindowLabels[4]->setText(ss.str()); ss.str("");
        Ogre::Vector3 oPos = WorldSingleton::getPlayerPosition().toOgre();
        ss << "Player local: " << oPos.x << ", " << oPos.y << ", " << oPos.z ;
        mDebugWindowLabels[5]->setText(ss.str());
    }

    void GuiManager::_loadDefaultSkin(){
        Colibri::SkinManager* skinManager = mColibriManager->getSkinManager();

        //Create the unlit datablocks.
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

        Ogre::HlmsBlendblock bb;
        bb.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);

        const char* blockNames[] = {
            "internal/PanelSkin",
            "internal/ButtonSkin",
            "internal/ButtonSkinDisabled",
            "internal/ButtonSkinBrighter",
            "internal/WindowMaterial",
            "internal/TickMarkChecked"
        };
        const Ogre::ColourValue colourValues[] = {
            Ogre::ColourValue(1, 1, 1, 1),
            Ogre::ColourValue(0, 0.67, 0.81, 1),
            Ogre::ColourValue(0, 0.67, 0.81, 0.2),
            Ogre::ColourValue(0, 1.34, 1.62, 1),
            Ogre::ColourValue(0.2, 0.2, 0.2, 0.9),
            Ogre::ColourValue::White
        };
        for(int i = 0; i < sizeof(blockNames) / sizeof(const char*); i++){
            Ogre::HlmsDatablock* block = unlit->createDatablock(blockNames[i], blockNames[i], Ogre::HlmsMacroblock(), bb, Ogre::HlmsParamVec(), true);
            Ogre::HlmsUnlitDatablock* unlitBlock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(block);
            unlitBlock->setUseColour(true);
            unlitBlock->setColour(colourValues[i]);
        }

        skinManager->loadSkins(defaultSkin, "internal/defaultSkin");
    }

    /*//Colibri stuff
    {

    mainWindow = colibriManager->createWindow( 0 );
    mainWindow->setTransform( Ogre::Vector2( 0, 0 ), Ogre::Vector2( 850, 100 ) );

    //The fist value represents virtual screen coordinates. The second is the actual resolution of the screen.
    colibriManager->setCanvasSize( Ogre::Vector2( 1920.0f, 1080.0f ), Ogre::Vector2( _window->getWidth(), _window->getHeight() ) );

    Colibri::LayoutLine *layout = new Colibri::LayoutLine( colibriManager );

    // button0 = colibriManager->createWidget<Colibri::Button>( mainWindow );
    // button0->m_minSize = Ogre::Vector2( 100, 64 );
    // button0->getLabel()->setText( "This is a button" );
    // button0->sizeToFit();
    // layout->addCell( button0 );
    //
    // button1 = colibriManager->createWidget<Colibri::Button>( mainWindow );
    // button1->m_minSize = Ogre::Vector2( 100, 64 );
    // button1->getLabel()->setText( "This is another fjaljlksbutton" );
    // button1->sizeToFit();
    //layout->addCell( button1 );

    Colibri::Label *label = colibriManager->createWidget<Colibri::Label>( mainWindow );
    label->setText( "This is some text" );
    //label->setText( "こんにちは" );
    //label->setSize( Ogre::Vector2(100, 20) );
    label->setShadowOutline( true, Ogre::ColourValue::Black, Ogre::Vector2( 1.0f ) );
    label->setTransform( Ogre::Vector2(10, 10), Ogre::Vector2(30, 10) );
    label->sizeToFit();
    label->setTopLeft(Ogre::Vector2(10, 10));

//layout->addCell( label );

    //layout->setAdjustableWindow( mainWindow );
    layout->m_hardMaxSize = colibriManager->getCanvasSize();

    Colibri::LayoutLine *layoutW = new Colibri::LayoutLine( colibriManager );
    layoutW->setCellSize( colibriManager->getCanvasSize() );
    layoutW->addCell( &Colibri::LayoutSpacer::c_DefaultBlankSpacer );
    layoutW->addCell( layout );
    //layoutW->addCell( label1 );
    layoutW->layout();


    {
        //This is a pointer to the parent window rather than any sort of id.
        static Colibri::Window* secondWindow = colibriManager->createWindow( 0 );
        secondWindow->setTransform( Ogre::Vector2( 500, 500 ), Ogre::Vector2( 850, 100 ) );

        static Colibri::Label *label = colibriManager->createWidget<Colibri::Label>( secondWindow );
        //label->setText( "こんにちは" );
        label->setText( "私は猫が大好きです" );
        //label->setShadowOutline( true, Ogre::ColourValue::Black, Ogre::Vector2( 10.0f ) );
        //label->setTransform( Ogre::Vector2(10, 10), Ogre::Vector2(30, 10) );
        label->setDefaultFontSize(Colibri::FontSize(16.0f));
        label->sizeToFit();
        //label->setTopLeft(Ogre::Vector2(10, 10));

        static Colibri::Label *label2 = colibriManager->createWidget<Colibri::Label>( secondWindow );
        label2->setText( "こんにちは This is some other text" );
        //label2->setText( "私は猫が大好きです" );
        //label->setShadowOutline( true, Ogre::ColourValue::Black, Ogre::Vector2( 10.0f ) );
        //label->setTransform( Ogre::Vector2(10, 10), Ogre::Vector2(30, 10) );
        label2->sizeToFit();

        button0 = colibriManager->createWidget<Colibri::Button>( secondWindow );
        //button0->m_minSize = Ogre::Vector2( 100, 64 );
        button0->getLabel()->setText( "This is a button" );
        button0->sizeToFit();

        static Colibri::LayoutLine *layoutW = new Colibri::LayoutLine( colibriManager );
        //layoutW->setCellSize( colibriManager->getCanvasSize() );
        //layoutW->setCellSize( Ogre::Vector2(100, 50) );
        layoutW->addCell( label );
        layoutW->addCell( label2 );
        layoutW->addCell( button0 );
        //layoutW->addCell( label1 );
        layoutW->layout();
    }*/
}
