#pragma once

#include <string>
#include <vector>
#include <map>

namespace AV{
    typedef unsigned int BlockId;

    enum class TagType{
        TEXT,
        TEXT_STRING,

        JMP
    };

    struct TagEntry{
        TagType type;

        //General purpose integer.
        //Tags which require only one value (like jmp) can just use this. For other tags this will be an index into another list.
        int i;
    };

    typedef std::vector<TagEntry> BlockContentList;

    struct CompiledDialog{
        std::map<BlockId, BlockContentList*> blockMap;

        std::vector<std::string> stringList;
    };
}
