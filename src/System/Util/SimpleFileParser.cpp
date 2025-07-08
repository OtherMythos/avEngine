#include "SimpleFileParser.h"

namespace AV{

    SimpleFileParser::SimpleFileParser(const std::string& filePath){
        #if defined(TARGET_ANDROID)
            mFile = sds::fstreamApk();
        #else
            mFile = sds::fstream();
        #endif

        mFile.open(filePath, sds::fstream::Input);
    }

    SimpleFileParser::~SimpleFileParser(){
        mFile.close();
    }

    bool SimpleFileParser::isOpen(){
        return mFile.is_open();
    }

    bool SimpleFileParser::getLine(std::string& line){
        if(mFile.is_eof()) return false;

        line.clear();
        char c;
        while(mFile.read<char>(c) == sizeof(char)){
            if(c == '\r') {
                // ignore Windows CR
                continue;
            }else if (c == '\n'){
                break;
            }else{
                line += c;
            }
        }

        if(line.empty()){
            return false;
        }

        return true;
    }

}
