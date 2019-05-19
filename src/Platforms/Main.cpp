#include "Logger/Log.h"
#include "System/Base.h"
#include "System/SystemSetup/SystemSetup.h"

#include <stdlib.h>
#include <vector>
#include <string>

#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <shellapi.h>
#include <atlstr.h>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
#else
int main(int argc, char **argv){
#endif

    AV::Log::Init();

    std::vector<std::string> args;

#ifdef WIN32
    //Windows makes things nice and complicated, and this was the first way I found to get arguments from the user.
    LPWSTR *szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL == szArglist)
    {
        wprintf(L"CommandLineToArgvW failed\n");
        return 0;
    }
    else for (i = 0; i < nArgs; i++) {
        args.push_back(std::string(CW2A(szArglist[i])));
    }

    LocalFree(szArglist);
#else
	for(int i = 0; i < argc; i++){
        args.push_back(std::string(argv[i]));
    }
#endif

    AV::SystemSetup::setup(args);

    AV::Base base;
    while(base.isOpen()){
        base.update();
    }

    base.shutdown();

    return 0;
}
