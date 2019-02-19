#include "Logger/Log.h"
#include "System/Base.h"
#include "System/SystemSetup/SystemSetup.h"

//int main(int argc, char **argv){
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    AV::Log::Init();
    //AV::SystemSetup::setup(argc, argv);

    AV::Base base;
    while(base.isOpen()){
        base.update();
    }

    base.shutdown();

    return 0;
}
