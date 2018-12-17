#include "Logger/Log.h"
#include "System/Base.h"

int main(){
    AV::Log::Init();
    
    AV::Base base;
    while(base.isOpen()){
        base.update();
    }

    return 0;
}
