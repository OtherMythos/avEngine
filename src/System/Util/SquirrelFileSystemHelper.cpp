#include "SquirrelFileSystemHelper.h"

#include <cstdio>

#ifdef TARGET_ANDROID
    #include <android/asset_manager.h>
    #include <android/asset_manager_jni.h>
    #include "System/FileSystem/FilePath.h"
#endif

namespace AV{

    typedef void* SQFILE;

    #define SQ_SEEK_CUR 0
    #define SQ_SEEK_END 1
    #define SQ_SEEK_SET 2

    #define IO_BUFFER_SIZE 2048
    struct IOBuffer {
        unsigned char buffer[IO_BUFFER_SIZE];
        SQInteger size;
        SQInteger ptr;
        SQFILE file;
    };

    SQFILE sqstd_fopen(const SQChar *filename ,const SQChar *mode)
    {
        #ifdef TARGET_ANDROID
            AAsset* asset = AAssetManager_open(FilePath::mAssetManager, filename, AASSET_MODE_UNKNOWN);
            if (!asset) {
                AAsset_close(asset);
                return 0;
            }
            return (SQFILE)asset;
        #else
            return (SQFILE)fopen(filename,mode);
        #endif
    }

    SQInteger sqstd_fread(void* buffer, SQInteger size, SQInteger count, SQFILE file)
    {
        #ifdef TARGET_ANDROID
            return (SQInteger)AAsset_read((AAsset*)file, buffer, size * count) / size;
        #else
            SQInteger ret = (SQInteger)fread(buffer,size,count,(FILE *)file);
            return ret;
        #endif
    }

    SQInteger sqstd_fwrite(const SQUserPointer buffer, SQInteger size, SQInteger count, SQFILE file)
    {
        return (SQInteger)fwrite(buffer,size,count,(FILE *)file);
    }

    SQInteger sqstd_fseek(SQFILE file, SQInteger offset, SQInteger origin)
    {
        SQInteger realorigin;
        switch(origin) {
            case SQ_SEEK_CUR: realorigin = SEEK_CUR; break;
            case SQ_SEEK_END: realorigin = SEEK_END; break;
            case SQ_SEEK_SET: realorigin = SEEK_SET; break;
            default: return -1; //failed
        }

        #ifdef TARGET_ANDROID
            // AAsset_seek returns new offset or -1 on failure
            return AAsset_seek((AAsset*)file, (off_t)offset, (int)realorigin) == -1 ? -1 : 0;
        #else
            return fseek((FILE *)file,(long)offset,(int)realorigin);
        #endif
    }

    SQInteger sqstd_ftell(SQFILE file)
    {
        #ifdef TARGET_ANDROID
            off_t pos = AAsset_seek((AAsset*)file, 0, SEEK_CUR);
            return pos;
        #else
            return ftell((FILE *)file);
        #endif
    }

    SQInteger sqstd_fflush(SQFILE file)
    {
        return fflush((FILE *)file);
    }

    SQInteger sqstd_fclose(SQFILE file)
    {
        #ifdef TARGET_ANDROID
            AAsset_close((AAsset*)file);
            return 0;
        #else
            return fclose((FILE *)file);
        #endif
    }

    SQInteger sqstd_feof(SQFILE file)
    {
        return feof((FILE *)file);
    }


    SQInteger _read_byte(IOBuffer *iobuffer)
    {
        if(iobuffer->ptr < iobuffer->size) {

            SQInteger ret = iobuffer->buffer[iobuffer->ptr];
            iobuffer->ptr++;
            return ret;
        }
        else {
            if( (iobuffer->size = sqstd_fread(iobuffer->buffer,1,IO_BUFFER_SIZE,iobuffer->file )) > 0 )
            {
                SQInteger ret = iobuffer->buffer[0];
                iobuffer->ptr = 1;
                return ret;
            }
        }

        return 0;
    }

    SQInteger _read_two_bytes(IOBuffer *iobuffer)
    {
        if(iobuffer->ptr < iobuffer->size) {
            if(iobuffer->size < 2) return 0;
            SQInteger ret = *((const wchar_t*)&iobuffer->buffer[iobuffer->ptr]);
            iobuffer->ptr += 2;
            return ret;
        }
        else {
            if( (iobuffer->size = sqstd_fread(iobuffer->buffer,1,IO_BUFFER_SIZE,iobuffer->file )) > 0 )
            {
                if(iobuffer->size < 2) return 0;
                SQInteger ret = *((const wchar_t*)&iobuffer->buffer[0]);
                iobuffer->ptr = 2;
                return ret;
            }
        }

        return 0;
    }

    static SQInteger _io_file_lexfeed_PLAIN(SQUserPointer iobuf)
    {
        IOBuffer *iobuffer = (IOBuffer *)iobuf;
        return _read_byte(iobuffer);

    }

#ifdef SQUNICODE
    static SQInteger _io_file_lexfeed_UTF8(SQUserPointer iobuf)
{
    IOBuffer *iobuffer = (IOBuffer *)iobuf;
#define READ(iobuf) \
    if((inchar = (unsigned char)_read_byte(iobuf)) == 0) \
        return 0;

    static const SQInteger utf8_lengths[16] =
    {
        1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
        0,0,0,0,                /* 1000 to 1011 : not valid */
        2,2,                    /* 1100, 1101 : 2 bytes */
        3,                      /* 1110 : 3 bytes */
        4                       /* 1111 :4 bytes */
    };
    static const unsigned char byte_masks[5] = {0,0,0x1f,0x0f,0x07};
    unsigned char inchar;
    SQInteger c = 0;
    READ(iobuffer);
    c = inchar;
    //
    if(c >= 0x80) {
        SQInteger tmp;
        SQInteger codelen = utf8_lengths[c>>4];
        if(codelen == 0)
            return 0;
            //"invalid UTF-8 stream";
        tmp = c&byte_masks[codelen];
        for(SQInteger n = 0; n < codelen-1; n++) {
            tmp<<=6;
            READ(iobuffer);
            tmp |= inchar & 0x3F;
        }
        c = tmp;
    }
    return c;
}
#endif

    static SQInteger _io_file_lexfeed_UCS2_LE(SQUserPointer iobuf)
    {
        SQInteger ret;
        IOBuffer *iobuffer = (IOBuffer *)iobuf;
        if( (ret = _read_two_bytes(iobuffer)) > 0 )
            return ret;
        return 0;
    }

    static SQInteger _io_file_lexfeed_UCS2_BE(SQUserPointer iobuf)
    {
        SQInteger c;
        IOBuffer *iobuffer = (IOBuffer *)iobuf;
        if( (c = _read_two_bytes(iobuffer)) > 0 ) {
            c = ((c>>8)&0x00FF)| ((c<<8)&0xFF00);
            return c;
        }
        return 0;
    }

    SQInteger file_read(SQUserPointer file,SQUserPointer buf,SQInteger size)
    {
        SQInteger ret;
        if( ( ret = sqstd_fread(buf,1,size,(SQFILE)file ))!=0 )return ret;
        return -1;
    }



    SQRESULT SquirrelFileSystemHelper::sqstd_loadfile(HSQUIRRELVM v, const SQChar *filename, SQBool printerror){
        SQFILE file = sqstd_fopen(filename,_SC("rb"));

        SQInteger ret;
        unsigned short us;
        unsigned char uc;
        SQLEXREADFUNC func = _io_file_lexfeed_PLAIN;
        if(file){
            ret = sqstd_fread(&us,1,2,file);
            if(ret != 2) {
                //probably an empty file
                us = 0;
            }
            if(us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
                sqstd_fseek(file,0,SQ_SEEK_SET);
                if(SQ_SUCCEEDED(sq_readclosure(v,file_read,file))) {
                    sqstd_fclose(file);
                    return SQ_OK;
                }
            }
            else { //SCRIPT
    
                switch(us)
                {
                    //gotta swap the next 2 lines on BIG endian machines
                    case 0xFFFE: func = _io_file_lexfeed_UCS2_BE; break;//UTF-16 little endian;
                    case 0xFEFF: func = _io_file_lexfeed_UCS2_LE; break;//UTF-16 big endian;
                    case 0xBBEF:
                        if(sqstd_fread(&uc,1,sizeof(uc),file) == 0) {
                            sqstd_fclose(file);
                            return sq_throwerror(v,_SC("io error"));
                        }
                        if(uc != 0xBF) {
                            sqstd_fclose(file);
                            return sq_throwerror(v,_SC("Unrecognized encoding"));
                        }
    #ifdef SQUNICODE
                        func = _io_file_lexfeed_UTF8;
    #else
                        func = _io_file_lexfeed_PLAIN;
    #endif
                        break;//UTF-8 ;
                    default: sqstd_fseek(file,0,SQ_SEEK_SET); break; // ascii
                }
                IOBuffer buffer;
                buffer.ptr = 0;
                buffer.size = 0;
                buffer.file = file;
                if(SQ_SUCCEEDED(sq_compile(v,func,&buffer,filename,printerror))){
                    sqstd_fclose(file);
                    return SQ_OK;
                }
            }
            sqstd_fclose(file);
            return SQ_ERROR;
        }
        return sq_throwerror(v,_SC("cannot open the file"));
    }

    SQRESULT SquirrelFileSystemHelper::sqstd_dofile(HSQUIRRELVM v, const SQChar *filename, SQBool retval, SQBool printerror)
    {
        //at least one entry must exist in order for us to push it as the environment
        if(sq_gettop(v) == 0)
            return sq_throwerror(v,_SC("environment table expected"));

        if(SQ_SUCCEEDED(sqstd_loadfile(v,filename,printerror))) {
            sq_push(v,-2);
            if(SQ_SUCCEEDED(sq_call(v,1,retval,SQTrue))) {
                sq_remove(v,retval?-2:-1); //removes the closure
                return 1;
            }
            sq_pop(v,1); //removes the closure
        }
        return SQ_ERROR;
    }

}