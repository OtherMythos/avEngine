#include "DialogCompiler.h"

#include "tinyxml2.h"

#include "Logger/Log.h"
#include "OgreIdString.h"

namespace AV{
    DialogCompiler::DialogCompiler(){

    }

    DialogCompiler::~DialogCompiler(){

    }

    bool DialogCompiler::compileScript(const std::string& filePath, CompiledDialog& outData){
        tinyxml2::XMLDocument xmlDoc;

        if(xmlDoc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
            AV_ERROR("Error when opening the dialog script: {}", filePath);
            AV_ERROR(xmlDoc.ErrorStr());
            return false;
        }

        //At this point the xml is valid, but there could be errors in the content.
        bool success = true;
        success &= _initialScanDocument(xmlDoc, outData);
        if(!success){
            AV_ERROR("Error during dialog script compilation");
            AV_ERROR(mErrorReason);
            return false;
        }

        return true;
    }

    bool DialogCompiler::_initialScanDocument(tinyxml2::XMLDocument &xmlDoc, CompiledDialog& d){
        tinyxml2::XMLNode *root = xmlDoc.FirstChild();
        if(!root) {
            mErrorReason = "Script file empty!";
            return false;
        }

        if(strcmp(root->Value(), "Dialog_Script") != 0){
            mErrorReason = "The first tag of a dialog script should be named Dialog_Script.";
            return false;
        }

        d.blockMap = new BlockMapType();
        d.stringList = new StringListType();
        d.entry2List = new Entry2List();
        d.entry4List = new Entry4List();
        d.vEntry2List = new VEntry2List();
        d.vEntry1List = new VEntry1List();

        for(tinyxml2::XMLElement *e = root->FirstChildElement("b"); e != NULL; e = e->NextSiblingElement("b")){
            if(e){
                if(!_parseBlock(e, d)){
                    return false;
                }
            }
        }

        return true;
    }

    bool DialogCompiler::_parseBlock(tinyxml2::XMLElement *e, CompiledDialog& d){
        int blockId = e->IntAttribute("id", -1);
        if(blockId < 0){
            mErrorReason = "A block was provided with an incorrect or missing id.";
            return false;
        }

        if(d.blockMap->find(blockId) != d.blockMap->end()){
            mErrorReason = "Two blocks were found with the same id.";
            return false;
        }

        //Optimisation
        //I could have a single list which contains all the various block steps.
        //The map would then contain indexes as to where each block starts. You'd determine the end by some sort of terminator entry.
        //This would mean only one list ever has to be created.
        BlockContentList* blockList = new BlockContentList();
        int reserveCount = e->IntAttribute("sz", -1);
        if(reserveCount >= 0){
            blockList->reserve(reserveCount);
        }

        for(tinyxml2::XMLElement *item = e->FirstChildElement(); item != NULL; item = item->NextSiblingElement()){
            if(!_parseDialogTag(item, d, blockList)){
                return false;
            }
        }

        (*d.blockMap)[blockId] = blockList;

        return true;
    }

    bool DialogCompiler::_parseDialogTag(tinyxml2::XMLElement *item, CompiledDialog& d, BlockContentList* blockList){
        const char* n = item->Value();
        const char* t = item->GetText();

        if(strcmp(n, "ts") == 0){
            int contentResult = _scanStringForVariables(t);
            {
                if(contentResult == -1){
                    mErrorReason = "Malformed dialog string";
                    return false;
                }
                if(contentResult == -2){
                    mErrorReason = "No more than 4 variables were provided in a single piece of dialog.";
                    return false;
                }
                if(contentResult == -3){
                    mErrorReason = "A variable was specified without any content between the tags.";
                    return false;
                }
            }
            int targetPos = d.stringList->size();
            d.stringList->push_back(t);
            TagType resultTag = contentResult > 0 ? _setVariableFlag(TagType::TEXT_STRING) : TagType::TEXT_STRING;
            blockList->push_back({resultTag, targetPos});
        }
        else if(strcmp(n, "jmp") == 0){
            //At some point I need to figure out if there actually is a block with that id.
            AttributeOutput o;
            GetAttributeResult r = _getAttribute(item, "id", AttributeType::INT, o);
            if(r != GET_SUCCESS){
                mErrorReason = "jmp tags should include an attribute named id which refers to a valid dialog block.";
                return false;
            }

            if(o.isVariable){
                blockList->push_back({_setVariableFlag(TagType::JMP), static_cast<int>(d.vEntry1List->size())});
                VariableAttribute a;
                a._varData = _attributeOutputToChar(o, AttributeType::INT);
                a.mVarHash = o.vId;
                d.vEntry1List->push_back(a);
            }else{
                blockList->push_back({TagType::JMP, o.i});
            }
        }
        else if(strcmp(n, "sleep") == 0){
            AttributeOutput o;
            GetAttributeResult r = _getAttribute(item, "l", AttributeType::INT, o);
            if(r != GET_SUCCESS){
                mErrorReason = "Include a positive integer time value in milliseconds for a sleep tag with the label 'l'";
                return false;
            }
            if(o.isVariable){
                blockList->push_back({_setVariableFlag(TagType::SLEEP), static_cast<int>(d.vEntry1List->size())});
                VariableAttribute a;
                a._varData = _attributeOutputToChar(o, AttributeType::INT);
                a.mVarHash = o.vId;
                d.vEntry1List->push_back(a);
            }else{
                blockList->push_back({TagType::SLEEP, o.i});
            }
        }
        else if(strcmp(n, "actorMoveTo") == 0){
            int actorId = -1;
            tinyxml2::XMLError e = item->QueryIntAttribute("a", &actorId);
            if(e != tinyxml2::XML_SUCCESS){
                mErrorReason = "Include an actor id with an actorMoveTo tag";
                return false;
            }

            int x, y, z;
            x = item->IntAttribute("x", 0);
            y = item->IntAttribute("y", 0);
            z = item->IntAttribute("z", 0);

            blockList->push_back({TagType::ACTOR_MOVE_TO, static_cast<int>(d.entry4List->size())});
            d.entry4List->push_back({x, y, z, actorId});
        }
        else if(strcmp(n, "actorChangeDirection") == 0){
            int actorId = -1;
            tinyxml2::XMLError e = item->QueryIntAttribute("a", &actorId);
            if(e != tinyxml2::XML_SUCCESS){
                mErrorReason = "Include an actor id with an actorChangeDirection tag";
                return false;
            }
            int direction;
            direction = item->IntAttribute("d", 0);

            blockList->push_back({TagType::ACTOR_CHANGE_DIRECTION, static_cast<int>(d.entry2List->size())});
            d.entry2List->push_back({actorId, direction});
        }
        else if(strcmp(n, "hideDialogWindow") == 0){
            blockList->push_back({TagType::HIDE_DIALOG_WINDOW, 0});
        }

        return true;
    }

    DialogCompiler::GetAttributeResult DialogCompiler::_getAttribute(tinyxml2::XMLElement *item, const char* name, AttributeType t, AttributeOutput& o){
        const tinyxml2::XMLAttribute* attrib = item->FindAttribute(name);
        if(!attrib) return GET_NOT_FOUND;

        tinyxml2::XMLError error;
        switch(t){
            case AttributeType::INT:{
                error = attrib->QueryIntValue( &(o.i) );
                break;
            }
            case AttributeType::BOOLEAN:{
                error = attrib->QueryBoolValue( &(o.b) );
                break;
            }
            case AttributeType::FLOAT:{
                error = attrib->QueryFloatValue( &(o.f) );
                break;
            }
            case AttributeType::STRING:{
                //Defer this until a later date, as this might be a variable.
                //Populate error with some dummy field.
                error = tinyxml2::XMLError::XML_NO_ATTRIBUTE;
                break;
            }
            default:{
                assert(false);
            }
        }
        o.isVariable = false;
        if(error == tinyxml2::XMLError::XML_SUCCESS){
            //A constant value was found so this can just be returned.
            return GET_SUCCESS;
        }
        //By this point something was wrong with the initial check.
        //That doesn't mean the attribute is invalid though.
        //This might be because a variable was provided rather than just a constant.
        //We expect the attribute to be a string now. If not then it's failed.

        const char* c = attrib->Value();
        char target = '\0';
        //To be a variable the attribute needs to have either of the terminators at the beginning and end of the string.
        if(c[0] == '$' || c[0] == '#'){
            target = c[0];
            size_t end = strlen(c);
            if(c[end - 1] == target){
                //This is a variable.

                //TODO Check if the variable string is empty.
                o.isVariable = true;
                o.globalVariable = target == '$' ? true : false;
                //Strip the terminator characters from the attribute.
                Ogre::IdString idS(std::string(c).substr(1, end - 2));
                o.vId = idS.mHash;

                return GET_SUCCESS;
            }
        }
        //Now the attribute is just a regular string.

        if(t != AttributeType::STRING) return GET_TYPE_MISMATCH;
        else assert(false);

        //TODO implement returning strings.
        //Right now I'm not using them so can't test them.

        return GET_SUCCESS;
    }

    int DialogCompiler::_scanStringForVariables(const char* c){
        char currentCheck = '\0';
        int foundVariables = 0;
        bool nextAfterFound = false;

        const char* p = &c[0];
        while(*p != '\0') {
            char c = *p;

            if(c == '#' || c == '$'){
                if(currentCheck == '\0'){
                    currentCheck = c;
                    nextAfterFound = true;
                }else{
                    if(currentCheck == c){ //This is the terminator value.

                        //If this is true then there was no string within the two terminator variables.
                        if(nextAfterFound) return -3;
                        foundVariables++;
                        currentCheck = '\0';
                        if(foundVariables > 4) return -2;
                    }else{
                        //Another variable declaration has appeared inside this one, so the string is malformed.
                        return -1;
                    }
                }
            }

            nextAfterFound = false;
            ++p;
        }

        if(currentCheck != '\0') return -1; //The string has ended but we're still trying to read a variable. Therefore it's malformed.

        return foundVariables;
    }

    char DialogCompiler::_attributeOutputToChar(const AttributeOutput& o, AttributeType t){
        char retChar = 0;
        if(o.isVariable) retChar |= 0x1;
        if(o.globalVariable) retChar |= 0x2;

        retChar |= ((char)t << 2u);

        return retChar;
    }

}
