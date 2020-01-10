#include "FileClass.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    SQObject FileClass::classObject;

    void FileClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("File"), -1);
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, fileClassConstructor, "constructor");
        ScriptUtils::addFunction(vm, openFile, "open", 2, ".s");
        ScriptUtils::addFunction(vm, getLine, "getLine");
        ScriptUtils::addFunction(vm, getEOF, "eof");
        ScriptUtils::addFunction(vm, closeFile, "close");

        sq_newslot(vm, -3, false);
    }

    SQInteger FileClass::fileClassConstructor(HSQUIRRELVM vm){
        std::fstream* stream = new std::fstream();
        sq_setinstanceup(vm, -1, (SQUserPointer*)stream);

        sq_setreleasehook(vm, -1, fileReleaseHook);

        return 0;
    }

    SQInteger FileClass::openFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, -1, &filePath);

        SQUserPointer p;
        sq_getinstanceup(vm, -2, &p, 0);

        std::fstream* f = (std::fstream*)p;
        f->close();
        f->open(filePath);

        return 0;
    }

    SQInteger FileClass::closeFile(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, -1, &p, 0);

        std::fstream* f = (std::fstream*)p;
        f->close();

        return 0;
    }

    SQInteger FileClass::getLine(HSQUIRRELVM vm){
        std::string outString;

        SQUserPointer p;
        sq_getinstanceup(vm, -1, &p, 0);
        std::fstream* f = (std::fstream*)p;

        std::getline(*f, outString);

        sq_pushstring(vm, outString.c_str(), -1);

        return 1;
    }

    SQInteger FileClass::getEOF(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, -1, &p, 0);
        std::fstream* f = (std::fstream*)p;

        sq_pushbool(vm, f->fail() || f->eof());

        return 1;
    }

    SQInteger FileClass::fileReleaseHook(SQUserPointer p, SQInteger size){

        std::fstream* f = (std::fstream*)p;
        f->close();

        delete f;

        return 0;
    }
}