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
        ScriptUtils::addFunction(vm, openFile, "open", -2, ".s");
        ScriptUtils::addFunction(vm, getLine, "getLine");
        ScriptUtils::addFunction(vm, getEOF, "eof");
        ScriptUtils::addFunction(vm, closeFile, "close");
        ScriptUtils::addFunction(vm, writeToFile, "write", 2, ".s");
        ScriptUtils::addFunction(vm, writeLineToFile, "writeLine", 2, ".s");

        sq_newslot(vm, -3, false);

        ScriptUtils::declareConstant(vm, "_FILE_READ", static_cast<SQInteger>(FileOpenMode::READ));
        ScriptUtils::declareConstant(vm, "_FILE_WRITE", static_cast<SQInteger>(FileOpenMode::WRITE));
        ScriptUtils::declareConstant(vm, "_FILE_READ_WRITE", static_cast<SQInteger>(FileOpenMode::READ_WRITE));
    }

    SQInteger FileClass::fileClassConstructor(HSQUIRRELVM vm){
        FileClassData* data = new FileClassData();
        sq_setinstanceup(vm, 1, (SQUserPointer*)data);

        sq_setreleasehook(vm, 1, fileReleaseHook);

        return 0;
    }

    SQInteger FileClass::openFile(HSQUIRRELVM vm){
        const SQChar *filePath;
        sq_getstring(vm, 2, &filePath);

        //Determine the requested open mode, defaulting to read/write for backwards compatibility.
        SQInteger mode = static_cast<SQInteger>(FileOpenMode::READ_WRITE);
        if(sq_gettop(vm) >= 3){
            sq_getinteger(vm, 3, &mode);
        }

        std::ios_base::openmode openMode;
        bool readable;
        bool writable;
        switch(static_cast<FileOpenMode>(mode)){
            case FileOpenMode::READ:
                openMode = std::ios_base::in;
                readable = true;
                writable = false;
                break;
            case FileOpenMode::WRITE:
                openMode = std::ios_base::out;
                readable = false;
                writable = true;
                break;
            case FileOpenMode::READ_WRITE:
                openMode = std::ios_base::in | std::ios_base::out;
                readable = true;
                writable = true;
                break;
            default:
                return sq_throwerror(vm, "Invalid file open mode.");
        }

        std::string outString;
        formatResToPath(filePath, outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);

        FileClassData* f = (FileClassData*)p;
        f->stream.close();
        f->stream.clear();
        f->stream.open(outString, openMode);
        if(f->stream.fail()){
            return sq_throwerror(vm, "Error opening file.");
        }

        f->readable = readable;
        f->writable = writable;

        return 0;
    }

    SQInteger FileClass::closeFile(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);

        FileClassData* f = (FileClassData*)p;
        assert(f);
        f->stream.close();

        return 0;
    }

    SQInteger FileClass::getLine(HSQUIRRELVM vm){
        std::string outString;

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        FileClassData* f = (FileClassData*)p;
        assert(f);

        if(!f->readable){
            return sq_throwerror(vm, "File was not opened for reading.");
        }

        std::getline(f->stream, outString);

        sq_pushstring(vm, outString.c_str(), -1);

        return 1;
    }

    SQInteger FileClass::writeToFile(HSQUIRRELVM vm){
        const SQChar *outString;
        sq_getstring(vm, 2, &outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        FileClassData* f = (FileClassData*)p;
        assert(f);

        if(!f->writable){
            return sq_throwerror(vm, "File was not opened for writing.");
        }

        f->stream.write(outString, strlen(outString));

        return 0;
    }

    SQInteger FileClass::writeLineToFile(HSQUIRRELVM vm){
        const SQChar *outString;
        sq_getstring(vm, 2, &outString);

        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        FileClassData* f = (FileClassData*)p;
        assert(f);

        if(!f->writable){
            return sq_throwerror(vm, "File was not opened for writing.");
        }

        f->stream << outString;

        return 0;
    }

    SQInteger FileClass::getEOF(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, 1, &p, 0, false);
        FileClassData* f = (FileClassData*)p;
        assert(f);

        sq_pushbool(vm, f->stream.fail() || f->stream.eof());

        return 1;
    }

    SQInteger FileClass::fileReleaseHook(SQUserPointer p, SQInteger size){
        FileClassData* f = (FileClassData*)p;
        assert(f);

        f->stream.close();

        delete f;

        return 0;
    }
}
