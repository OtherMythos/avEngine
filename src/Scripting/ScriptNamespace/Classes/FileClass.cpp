#include "FileClass.h"

#define NOMINMAX
#include "System/Util/PathUtils.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include <fstream>

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
        ScriptUtils::addFunction(vm, writeToFile, "write", 2, ".s");
        ScriptUtils::addFunction(vm, writeLineToFile, "writeLine", 2, ".s");

        sq_newslot(vm, -3, false);
    }

    SQInteger FileClass::fileClassConstructor(HSQUIRRELVM vm){
        std::fstream* stream = new std::fstream();
        sq_setinstanceup(vm, 1, (SQUserPointer*)stream);

        sq_setreleasehook(vm, 1, fileReleaseHook);

        return 0;
    }

    SQInteger FileClass::openFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);

        std::string outString;
        formatResToPath(filePath, outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);

        std::fstream* f = (std::fstream*)p;
        f->close();
        f->open(outString);
        if(f->fail()){
            return sq_throwerror(vm, "Error opening file.");
        }

        return 0;
    }

    SQInteger FileClass::closeFile(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);

        std::fstream* f = (std::fstream*)p;
        assert(f);
        f->close();

        return 0;
    }

    SQInteger FileClass::getLine(HSQUIRRELVM vm){
        std::string outString;

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        std::fstream* f = (std::fstream*)p;
        assert(f);

        std::getline(*f, outString);

        sq_pushstring(vm, outString.c_str(), -1);

        return 1;
    }

    SQInteger FileClass::writeToFile(HSQUIRRELVM vm){
        const SQChar *outString;
        sq_getstring(vm, 2, &outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        std::fstream* f = (std::fstream*)p;
        assert(f);

        f->write(outString, strlen(outString));

        return 0;
    }

    SQInteger FileClass::writeLineToFile(HSQUIRRELVM vm){
        const SQChar *outString;
        sq_getstring(vm, 2, &outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        std::fstream* f = (std::fstream*)p;
        assert(f);

        *f << outString;

        return 0;
    }

    SQInteger FileClass::getEOF(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        std::fstream* f = (std::fstream*)p;
        assert(f);

        sq_pushbool(vm, f->fail() || f->eof());

        return 1;
    }

    SQInteger FileClass::fileReleaseHook(SQUserPointer p, SQInteger size){
        std::fstream* f = (std::fstream*)p;
        assert(f);

        f->close();

        delete f;

        return 0;
    }
}
