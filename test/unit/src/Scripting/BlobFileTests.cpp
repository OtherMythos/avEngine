#include <gtest/gtest.h>
#include <squirrel.h>
#include <sqstdblob.h>
#include <filesystem>
#include <chrono>
#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptNamespace/SystemNamespace.h"

namespace AV{
    class BlobFileTest : public testing::Test{
    protected:
        HSQUIRRELVM vm;
        std::filesystem::path path;
        void SetUp() override{
            vm = sq_open(128);
            sq_pushroottable(vm);
            sqstd_register_bloblib(vm);
            ScriptVM::setupNamespace(vm, "_system", SystemNamespace::setupNamespace);
            path = std::filesystem::temp_directory_path() / ("av-blob-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
            sq_pushstring(vm, "path", -1);
            sq_pushstring(vm, path.string().c_str(), -1);
            sq_newslot(vm, -3, false);
            sq_pop(vm, 1);
        }
        void TearDown() override { sq_close(vm); std::filesystem::remove(path); }
        bool run(const std::string& source){
            if(SQ_FAILED(sq_compilebuffer(vm, source.c_str(), source.size(), "blob-file-test", false))) return false;
            sq_pushroottable(vm);
            if(SQ_FAILED(sq_call(vm, 1, true, false))) return false;
            SQBool result = false;
            sq_getbool(vm, -1, &result);
            sq_pop(vm, 2);
            return result;
        }
    };
    TEST_F(BlobFileTest, RoundTripsBinaryZerosAndHighBytes){
        EXPECT_TRUE(run("local b=blob(4); b[0]=0; b[1]=255; b[2]=13; b[3]=10; _system.writeBlob(path,b); local r=_system.readBlob(path); return r.len()==4 && r[0]==0 && r[1]==255 && r[2]==13 && r[3]==10;"));
    }
    TEST_F(BlobFileTest, WritesOnlyRequestedPrefix){
        EXPECT_TRUE(run("local b=blob(8); b[0]=42; _system.writeBlob(path,b,1); local r=_system.readBlob(path); return r.len()==1 && r[0]==42;"));
    }
    TEST_F(BlobFileTest, RoundTripsEmptyFile){
        EXPECT_TRUE(run("_system.writeBlob(path,blob(0)); return _system.readBlob(path).len()==0;"));
    }
    TEST_F(BlobFileTest, RejectsInvalidLengthsBeforeOpeningOutput){
        EXPECT_TRUE(run("local count=0; foreach(n in [-1,5]){ try{ _system.writeBlob(path,blob(4),n); }catch(e){ count++; } } return count==2;"));
        EXPECT_FALSE(std::filesystem::exists(path));
    }
    TEST_F(BlobFileTest, ReportsMissingInput){
        EXPECT_TRUE(run("try{ _system.readBlob(path); }catch(e){ return true; } return false;"));
    }
    TEST_F(BlobFileTest, ReportsUnwritableOutput){
        EXPECT_TRUE(run("try{ _system.writeBlob(path+\"/missing/file\",blob(4)); }catch(e){ return true; } return false;"));
    }
}
