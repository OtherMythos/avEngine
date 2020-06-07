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

const char* defaultSkin =
"{"
"    \"skins\" :"
"    {"
"        \"ButtonSkin\" :"
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
"        \"ButtonSkinDisabled\" :"
"        {"
"            \"copy_from\" : \"ButtonSkin\","
"            \"material\" : \"internal/ButtonSkinDisabled\""
"        },"
"        \"ButtonSkinHighlightedCursor\" :"
"        {"
"            \"copy_from\" : \"ButtonSkin\","
"            \"material\" : \"internal/ButtonSkinBrighter\""
"        },"
"        \"ButtonSkinHighlightedButton\" :"
"        {"
"            \"copy_from\" : \"ButtonSkin\","
"            \"grid_uv\" :"
"            {"
"                \"enclosing\" : [[0, 32, 64, 32], [4, 4]]"
"            }"
"        },"
"        \"ButtonSkinHighlightedButtonAndCursor\" :"
"        {"
"            \"copy_from\" : \"ButtonSkinHighlightedButton\","
"            \"material\" : \"internal/ButtonSkinBrighter\""
"        },"
"        \"ButtonSkinPressed\" :"
"        {"
"            \"copy_from\" : \"ButtonSkin\","
"            \"material\" : \"internal/ButtonSkin\","
"            \"grid_uv\" :"
"            {"
"                \"enclosing\" : [[0, 0, 64, 32], [4, 4]]"
"            }"
"        },"
""
"        \"WindowSkin\" :"
"        {"
""
"            \"material\" : \"internal/WindowMaterial\","
"            \"borders\" :"
"            {"
"                \"all\" : [10, 10]"
"            }"
"        },"
""
"        \"CheckboxTickmarkChecked\" :"
"        {"
"            \"copy_from\" : \"ButtonSkin\","
"            \"grid_uv\" : { \"enclosing\" : [[0, 96, 32, 32], [4, 4]] }"
"        }"
"    },"
""
"    \"skin_packs\" :"
"    {"
"        \"ButtonSkin\" :"
"        {"
"            \"all\" : \"ButtonSkin\","
"            \"disabled\" : \"ButtonSkinDisabled\","
"            \"idle\" : \"ButtonSkin\","
"            \"highlighted_cursor\" : \"ButtonSkinHighlightedCursor\","
"            \"highlighted_button\" : \"ButtonSkinHighlightedButton\","
"            \"highlighted_button_and_cursor\" : \"ButtonSkinHighlightedButtonAndCursor\","
"            \"pressed\" : \"ButtonSkinPressed\""
"        },"
"        \"WindowSkin\" :"
"        {"
"            \"all\" : \"WindowSkin\""
"        },"
""
"        \"CheckboxTicked\" :"
"        {"
"            \"all\" : \"CheckboxTickmarkChecked\""
"        }"
"    },"
""
"    \"default_skin_packs\" :"
"    {"
"        \"Window\"                : \"WindowSkin\","
"        \"Button\"                : \"ButtonSkin\","
"        \"Spinner\"               : \"ButtonSkin\","
"        \"SpinnerBtnDecrement\"   : \"ButtonSkin\","
"        \"SpinnerBtnIncrement\"   : \"ButtonSkin\","
"        \"Checkbox\"              : \"ButtonSkin\","
"        \"CheckboxTickmarkUnchecked\"     : \"ButtonSkin\","
"        \"CheckboxTickmarkChecked\"       : \"CheckboxTicked\","
"        \"CheckboxTickmarkThirdState\"    : \"ButtonSkin\","
"        \"Editbox\"               : \"ButtonSkin\","
"        \"ProgressbarLayer0\"     : \"ButtonSkin\","
"        \"ProgressbarLayer1\"     : \"ButtonSkin\","
"        \"SliderLine\"            : \"ButtonSkin\","
"        \"SliderHandle\"          : \"ButtonSkin\""
"    }"
"}"
;

namespace AV{

    class GuiLogListener : public Colibri::LogListener{
        void log(const char *text, Colibri::LogSeverity::LogSeverity severity) {
            AV_ERROR(text);
        }
    };

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

        { //Process shapers
            Colibri::ShaperManager *shaperManager = mColibriManager->getShaperManager();

            const std::vector<SystemSettings::FontSettingEntry>& fontList = SystemSettings::getFontSettings();

            bool shaperAdded = false;
            for(const SystemSettings::FontSettingEntry& e : fontList){
                const filesystem::path defaultFontPath = filesystem::path(e.fontPath);
                if(!defaultFontPath.exists() || !defaultFontPath.is_file()) continue;
                //Latin for now.
                shaperManager->addShaper(HB_SCRIPT_LATIN, e.fontPath.c_str(), e.locale.c_str());
                shaperAdded = true;
            }
            if(!shaperAdded){
                //None are provided or valid, so use the default font.
                const Ogre::String& masterPath = SystemSettings::getMasterPath();
                const filesystem::path defaultFontPath = filesystem::path(masterPath) / filesystem::path("essential/font/ProggyClean.ttf");
                assert(defaultFontPath.exists() && defaultFontPath.is_file());

                shaperManager->addShaper( HB_SCRIPT_LATIN, defaultFontPath.str().c_str(), "en");
            }


        }

        { //Load skins
            const std::vector<std::string>& skinList = SystemSettings::getGuiSkins();
            for(const std::string& s : skinList){
                const filesystem::path defaultFontPath = filesystem::path(s);
                if(!defaultFontPath.exists() || !defaultFontPath.is_file()) continue;

                mColibriManager->loadSkins(s.c_str());
            }

            //Check if any skins were loaded correctly.
            Colibri::SkinManager* skinManager = mColibriManager->getSkinManager();
            if(skinManager->getSkins().size() <= 0){
                //Load the default skin if no actual skins were loaded.
                _loadDefaultSkin();
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
        mColibriManager->update(timeSinceLast);
    }

    void GuiManager::shutdown(){
        //TODO it might be nice if this call was somewhere else, for instance in the script manager.
        GuiNamespace::destroyStoredWidgets();

        delete mColibriManager;
    }

    void GuiManager::_loadDefaultSkin(){
        Colibri::SkinManager* skinManager = mColibriManager->getSkinManager();

        //Create the unlit datablocks.
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

        Ogre::HlmsBlendblock bb;
        bb.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);

        const char* blockNames[] = {
            "internal/ButtonSkin",
            "internal/ButtonSkinDisabled",
            "internal/ButtonSkinBrighter",
            "internal/WindowMaterial"
        };
        const Ogre::ColourValue colourValues[] = {
            Ogre::ColourValue(0, 0.67, 0.81, 1),
            Ogre::ColourValue(0, 0.67, 0.81, 0.2),
            Ogre::ColourValue(0, 1.34, 1.62, 1),
            Ogre::ColourValue(0.2, 0.2, 0.2, 0.9)
        };
        for(int i = 0; i < 4; i++){
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
