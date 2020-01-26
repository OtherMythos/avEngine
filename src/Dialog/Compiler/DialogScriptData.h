#pragma once

#include <string>
#include <vector>
#include <map>

namespace AV{
    typedef unsigned int BlockId;

    /**
    Enum for available tag types.
    Eight bits are used here, although the final bit is used as a flag of whether this tag contains a variable in any of its attributes.
    Variables require more memory so are treated differently.
    This flag means the data for variables can be stored differently, so not all entries have to pay the price for them.
    */
    enum class TagType : unsigned char{
        TEXT,
        TEXT_STRING,

        JMP,
        SLEEP,
        ACTOR_MOVE_TO,
        ACTOR_CHANGE_DIRECTION,
        HIDE_DIALOG_WINDOW
    };

    struct TagEntry{
        TagType type;

        //General purpose integer.
        //Tags which require only one value (like jmp) can just use this. For other tags this will be an index into another list.
        int i;
    };

    struct Entry2{
        int x;
        int y;
    };

    struct Entry4{
        int x;
        int y;
        int z;
        int w;
    };

    typedef std::vector<TagEntry> BlockContentList;
    typedef std::vector<Entry2> Entry2List;
    typedef std::vector<Entry4> Entry4List;
    typedef std::map<BlockId, BlockContentList*> BlockMapType;
    typedef std::vector<std::string> StringListType;

    struct CompiledDialog{
         BlockMapType* blockMap;

         StringListType* stringList;
         Entry2List* entry2List;
         Entry4List* entry4List;

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


    inline bool _tagContainsVariable(TagType t){
        return (unsigned char)t & (1 << 7);
    }
    inline TagType _stripVariableFlag(TagType t){
        return static_cast<TagType>((unsigned char)t & 0x7F);
    }
    inline TagType _setVariableFlag(TagType t){
        return static_cast<TagType>((unsigned char)t | 0x80);
    }
}
