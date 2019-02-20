#include "Logger/Log.h"
#include "System/Base.h"
#include "System/SystemSetup/SystemSetup.h"

#ifdef WIN32
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
#else
int main(int argc, char **argv){
#endif

    AV::Log::Init();

#ifdef WIN32
    AV::SystemSetup::setup(__argc, __argv);
#else
	AV::SystemSetup::setup(argc, argv);
#endif

    AV::Base base;
    while(base.isOpen()){
        base.update();
    }

    base.shutdown();

    return 0;
}
