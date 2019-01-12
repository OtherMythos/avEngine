#include "Logger/Log.h"
#include "System/Base.h"

int main(int argc, char **argv){
    AV::Log::Init();

    AV::Base base;
    while(base.isOpen()){
        base.update();
    }

    base.shutdown();

    return 0;
}
