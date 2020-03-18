#pragma once

#include <string>
#include <vector>
#include <map>
#include "OgrePrerequisites.h"

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
        HIDE_DIALOG_WINDOW,

        SCRIPT, //Script declaration tag
        SET
    };

    static const char* tagTypeString(TagType t){
        switch (t) {
            case TagType::TEXT: return "ts";
            case TagType::TEXT_STRING: return "t";
            case TagType::JMP: return "jmp";
            case TagType::SLEEP: return "sleep";
            case TagType::ACTOR_MOVE_TO: return "actorMoveTo";
            case TagType::ACTOR_CHANGE_DIRECTION: return "actorChangeDirection";
            case TagType::HIDE_DIALOG_WINDOW: return "hideDialogWindow";
            case TagType::SCRIPT: return "script";
            case TagType::SET: return "set";
            default: return "unknown";
        }
    }

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

    //Represents attributes which use variables.
    struct VariableAttribute{
        //The system determines which tags contain variables and sections them off.
        //If a tag with a variable is found this data type is used to represent its attributes, rather than just plain values.
        //This char is expected to be used for metadata.
        //The first bit means whether this attribute is a variable or not.
        //The second specifies global or local variables.
        //The rest of the char is used for remembering the expected variable type.
        char _varData;
        union{
            Ogre::uint32 mVarHash;
            int i;
            float f;
            bool b;
        };
    };

    enum class AttributeType : char{
        STRING = 0,
        FLOAT = 1 << 0,
        INT = 1 << 1,
        BOOLEAN = 1 << 2,

        NUMBER = FLOAT | INT
    };

    static const char* attributeTypeString(AttributeType t){
        switch(t){
            case AttributeType::STRING: return "String";
            case AttributeType::FLOAT: return "Float";
            case AttributeType::INT: return "Int";
            case AttributeType::BOOLEAN: return "Boolean";
            case AttributeType::NUMBER: return "Number";
            default: return "Unknown";
        }
    }

    struct vEntry2{
        VariableAttribute x;
        VariableAttribute y;
    };

    struct vEntry4{
        VariableAttribute x;
        VariableAttribute y;
        VariableAttribute z;
        VariableAttribute w;
    };

    typedef std::vector<TagEntry> HeaderInformation;
    typedef std::vector<TagEntry> BlockContentList;
    typedef std::vector<Entry2> Entry2List;
    typedef std::vector<Entry4> Entry4List;
    typedef std::vector<VariableAttribute> VEntry1List;
    typedef std::vector<vEntry2> VEntry2List;
    typedef std::vector<vEntry4> VEntry4List;
    typedef std::map<BlockId, BlockContentList*> BlockMapType;
    typedef std::vector<std::string> StringListType;

    struct CompiledDialog{
         BlockMapType* blockMap;
         HeaderInformation* headerInformation;

         StringListType* stringList;
         Entry2List* entry2List;
         Entry4List* entry4List;
         VEntry1List* vEntry1List;
         VEntry2List* vEntry2List;
         VEntry4List* vEntry4List;

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
             if(headerInformation) delete headerInformation;
             if(stringList) delete stringList;
             if(entry2List) delete entry2List;
             if(entry4List) delete entry4List;
             if(vEntry2List) delete vEntry2List;
             if(vEntry1List) delete vEntry1List;
             if(vEntry4List) delete vEntry4List;
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
