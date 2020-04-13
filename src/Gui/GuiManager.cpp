#include "GuiManager.h"

#include "ColibriGui/ColibriManager.h"
#include "ColibriGui/Text/ColibriShaperManager.h"
#include "ColibriGui/Text/ColibriShaper.h"
#include "ColibriGui/Ogre/CompositorPassColibriGuiProvider.h"
#include "ColibriGui/ColibriWindow.h"
#include "ColibriGui/ColibriButton.h"
#include "ColibriGui/ColibriLabel.h"
#include "ColibriGui/Layouts/ColibriLayoutLine.h"
#include "hb.h"

#include "System/BaseSingleton.h"
#include "OgreRoot.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Window/Window.h"

namespace AV{
    GuiManager::GuiManager(){

    }

    GuiManager::~GuiManager(){

    }

    void GuiManager::setupColibriManager(){
        static Colibri::LogListener colibriLogListener;
        static Colibri::ColibriListener colibriListener;
        mColibriManager = new Colibri::ColibriManager( &colibriLogListener, &colibriListener );
    }

    void GuiManager::setupCompositorProvider(Ogre::CompositorManager2* compMan){
        Ogre::CompositorPassColibriGuiProvider *compoProvider = OGRE_NEW Ogre::CompositorPassColibriGuiProvider( mColibriManager );
        compMan->setCompositorPassProvider( compoProvider );
    }

    void GuiManager::setup(Ogre::Root* root, Ogre::SceneManager* sceneManager){
        struct ShaperSettings{
            const char *locale;
            const char *fullpath;
            hb_script_t script;
            Colibri::HorizReadingDir::HorizReadingDir horizReadingDir;
            bool useKerning;
            bool allowsVerticalLayout;
            ShaperSettings( const char *_locale, const char *_fullpath, hb_script_t _script,
                            bool _useKerning=false,
                            Colibri::HorizReadingDir::HorizReadingDir _horizReadingDir=
                    Colibri::HorizReadingDir::LTR,
                            bool _allowsVerticalLayout=false ) :
                locale( _locale ),
                fullpath( _fullpath ),
                script( _script ),
                horizReadingDir( _horizReadingDir ),
                useKerning( _useKerning ),
                allowsVerticalLayout( _allowsVerticalLayout )
            {

            }
        };


        #ifdef __linux__
            ShaperSettings shaperSettings[3] =
            {
                //ShaperSettings( "en", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/DejaVuSerif.ttf", HB_SCRIPT_LATIN, true ),
                ShaperSettings( "en", "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", HB_SCRIPT_LATIN, true ),
                ShaperSettings( "ar", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/amiri-0.104/amiri-regular.ttf", HB_SCRIPT_ARABIC, false,
                Colibri::HorizReadingDir::RTL ),
                ShaperSettings( "ch", "/home/edward/Documents/avDeps/colibrigui/bin/Data/Fonts/fireflysung-1.3.0/fireflysung.ttf", HB_SCRIPT_HAN, false,
                Colibri::HorizReadingDir::LTR, true )
            };
        #elif _WIN32
            ShaperSettings shaperSettings[3] =
            {
                ShaperSettings( "en", "C:\\Users\\edward\\Documents\\avDeps\\colibrigui\\bin\\Data\\Fonts\\DejaVuSerif.ttf", HB_SCRIPT_LATIN, true ),
                ShaperSettings( "ar", "C:\\Users\\edward\\Documents\\avDeps\\colibrigui\\bin\\Data\\Fonts\\amiri-0.104\\amiri-regular.ttf", HB_SCRIPT_ARABIC, false,
                Colibri::HorizReadingDir::RTL ),
                ShaperSettings( "ch", "C:\\Users\\edward\\Documents\\avDeps\\colibrigui\\bin\\Data\\Fonts\\fireflysung-1.3.0\\fireflysung.ttf", HB_SCRIPT_HAN, false,
                Colibri::HorizReadingDir::LTR, true )
            };
        #endif

        Colibri::ShaperManager *shaperManager = mColibriManager->getShaperManager();

        for( size_t i=0; i<sizeof( shaperSettings ) / sizeof( shaperSettings[0] ); ++i )
        {
            Colibri::Shaper *shaper;
            shaper = shaperManager->addShaper( shaperSettings[i].script, shaperSettings[i].fullpath,
                                               shaperSettings[i].locale );
            if( shaperSettings[i].useKerning )
                shaper->addFeatures( Colibri::Shaper::KerningOn );
        }

        size_t defaultFont = 0; //"en"
        shaperManager->setDefaultShaper( defaultFont + 1u,
                                         shaperSettings[defaultFont].horizReadingDir,
                                         shaperSettings[defaultFont].allowsVerticalLayout );

        if( defaultFont == 1 ){
            mColibriManager->setSwapRTLControls( true );
        }

        mColibriManager->setOgre( root,
                     root->getRenderSystem()->getVaoManager(),
                     sceneManager );
        #ifdef __linux__
            mColibriManager->loadSkins("/home/edward/Documents/avDeps/colibrigui/bin/Data/Materials/ColibriGui/Skins/Debug/Skins.colibri.json");
        #elif _WIN32
            mColibriManager->loadSkins("C:\\Users\\edward\\Documents\\avDeps\\colibrigui\\bin\\Data\\Materials\\ColibriGui\\Skins\\DarkGloss\\Skins.colibri.json");
        #endif

        //The fist value represents virtual screen coordinates. The second is the actual resolution of the screen.
        //TODO in future I'd perfer not to have the singleton approach. Also remove the includes.
        mColibriManager->setCanvasSize( Ogre::Vector2( 1920.0f, 1080.0f ), Ogre::Vector2( BaseSingleton::getWindow()->getWidth(), BaseSingleton::getWindow()->getHeight() ) );



        //TODO temporary just to see something
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
        mColibriManager->update(10);
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
