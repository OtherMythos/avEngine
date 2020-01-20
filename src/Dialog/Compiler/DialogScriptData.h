#pragma once

#include <string>
#include <vector>
#include <map>

namespace AV{
    typedef unsigned int BlockId;

    enum class TagType{
        TEXT,
        TEXT_STRING,

        JMP,
        SLEEP
    };

    struct TagEntry{
        TagType type;

        //General purpose integer.
        //Tags which require only one value (like jmp) can just use this. For other tags this will be an index into another list.
        int i;
    };

    typedef std::vector<TagEntry> BlockContentList;
    typedef std::map<BlockId, BlockContentList*> BlockMapType;
    typedef std::vector<std::string> StringListType;

    struct CompiledDialog{
         BlockMapType* blockMap;

         StringListType* stringList;

         bool empty() const {
             return !(blockMap && stringList);
         }

         /**
         Destroy the pointers of this dialog. This does not reset the actual values however.
         */
         void destroy(){
             if(blockMap){
                 for (const auto& i : *blockMap){
                     if(i.second) delete i.second;
                 }
                 delete blockMap;
             }
             if(stringList) delete stringList;
         }

         bool operator ==(const CompiledDialog &d) const{
             return blockMap == d.blockMap && stringList == d.stringList;
         }
         bool operator !=(const CompiledDialog &d) const{
             return !(d == *this);
         }
    };
    static CompiledDialog EMPTY_DIALOG = {0, 0};
}
