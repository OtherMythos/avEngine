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
        d.headerInformation = new HeaderInformation();
        d.stringList = new StringListType();
        d.entry2List = new Entry2List();
        d.entry4List = new Entry4List();
        d.vEntry2List = new VEntry2List();
        d.vEntry1List = new VEntry1List();
        d.vEntry4List = new VEntry4List();

        for(tinyxml2::XMLElement *e = root->FirstChildElement(); e != NULL; e = e->NextSiblingElement()){
            if(!e) continue;

            const char* itemName = e->Name();
            if(strcmp(itemName, "b") == 0){
                if(!_parseBlock(e, d)){
                    return false;
                }
            }else if (strcmp(itemName, tagTypeString(TagType::SCRIPT)) == 0){
                if(!_parseScriptDeclaration(e, d)){
                    return false;
                }
            }
        }

        return true;
    }

    bool DialogCompiler::_parseScriptDeclaration(tinyxml2::XMLElement *e, CompiledDialog& d){
        AttributeOutput pathAttribute, idAttribute;
        GetAttributeResult r = _getAttribute(e, "path", AttributeType::STRING, pathAttribute);
        if(r != GET_SUCCESS){
            mErrorReason = "Include a string path with label 'path' to a script tag.";
            return false;
        }
        r = _getAttribute(e, "id", AttributeType::INT, idAttribute);
        if(r != GET_SUCCESS){
            mErrorReason = "Include an int attribute with label 'id' to a script tag.";
            return false;
        }

        //TODO think up a way to make sure no duplicate ids are provided.
        if(pathAttribute.isVariable || idAttribute.isVariable){
            d.headerInformation->push_back({_setVariableFlag(TagType::SCRIPT), static_cast<int>(d.vEntry2List->size())});
            VariableAttribute pathAttrib, idAttrib;
            pathAttrib._varData = _attributeOutputToChar(pathAttribute, AttributeType::STRING);
            pathAttrib.mVarHash = pathAttribute.vId;
            idAttrib._varData = _attributeOutputToChar(idAttribute, AttributeType::INT);
            idAttrib.mVarHash = idAttribute.vId;

            d.vEntry2List->push_back({pathAttrib, idAttrib});
        }else{
            //
            d.headerInformation->push_back({TagType::SCRIPT, static_cast<int>(d.entry2List->size())});
            d.entry2List->push_back({
                static_cast<int>(d.stringList->size()),
                idAttribute.i
            });
            d.stringList->push_back(pathAttribute.s);
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

        if(strcmp(n, tagTypeString(TagType::TEXT)) == 0){
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
        else if(strcmp(n, tagTypeString(TagType::JMP)) == 0){
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
        else if(strcmp(n, tagTypeString(TagType::SLEEP)) == 0){
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
        else if(strcmp(n, tagTypeString(TagType::ACTOR_MOVE_TO)) == 0){
            AttributeOutput aa, ax, ay, az;
            GetAttributeResult ar = _getAttribute(item, "a", AttributeType::INT, aa);
            GetAttributeResult xr = _getAttribute(item, "x", AttributeType::INT, ax);
            GetAttributeResult yr = _getAttribute(item, "y", AttributeType::INT, ay);
            GetAttributeResult zr = _getAttribute(item, "z", AttributeType::INT, az);
            if( !(ar == GET_SUCCESS && xr == GET_SUCCESS && yr == GET_SUCCESS && zr == GET_SUCCESS) ){
                mErrorReason = "Problem getting variables.";
                return false;
            }
            if(aa.isVariable || ax.isVariable || ay.isVariable || az.isVariable){
                blockList->push_back({_setVariableFlag(TagType::ACTOR_MOVE_TO), static_cast<int>(d.vEntry4List->size())});

                VariableAttribute va, vx, vy, vz;
                va._varData = _attributeOutputToChar(aa, AttributeType::INT);
                va.mVarHash = aa.vId;
                vx._varData = _attributeOutputToChar(ax, AttributeType::INT);
                vx.mVarHash = ax.vId;
                vy._varData = _attributeOutputToChar(ay, AttributeType::INT);
                vy.mVarHash = ay.vId;
                vz._varData = _attributeOutputToChar(az, AttributeType::INT);
                vz.mVarHash = az.vId;

                d.vEntry4List->push_back({vx, vy, vz, va});
            }else{
                blockList->push_back({TagType::ACTOR_MOVE_TO, static_cast<int>(d.entry4List->size())});
                d.entry4List->push_back({ax.i, ay.i, az.i, aa.i});
            }
        }
        else if(strcmp(n, tagTypeString(TagType::ACTOR_CHANGE_DIRECTION)) == 0){
            AttributeOutput aa, ad;
            GetAttributeResult ar = _getAttribute(item, "a", AttributeType::INT, aa);
            GetAttributeResult dr = _getAttribute(item, "d", AttributeType::INT, ad);
            if(ar != GET_SUCCESS || dr != GET_SUCCESS){
                mErrorReason = "Problem getting variables.";
                return false;
            }
            if(aa.isVariable || ad.isVariable){
                blockList->push_back({_setVariableFlag(TagType::ACTOR_CHANGE_DIRECTION), static_cast<int>(d.vEntry2List->size())});

                VariableAttribute va;
                va._varData = _attributeOutputToChar(aa, AttributeType::INT);
                va.mVarHash = aa.vId;
                VariableAttribute vd;
                vd._varData = _attributeOutputToChar(ad, AttributeType::INT);
                vd.mVarHash = ad.vId;

                d.vEntry2List->push_back({va, vd});
            }else{
                blockList->push_back({TagType::ACTOR_CHANGE_DIRECTION, static_cast<int>(d.entry2List->size())});
                d.entry2List->push_back({aa.i, ad.i});
            }

        }
        else if(strcmp(n, tagTypeString(TagType::HIDE_DIALOG_WINDOW)) == 0){
            blockList->push_back({TagType::HIDE_DIALOG_WINDOW, 0});
        }
        else if(strcmp(n, tagTypeString(TagType::SCRIPT)) == 0){
            AttributeOutput aa, as;
            GetAttributeResult ar = _getAttribute(item, "id", AttributeType::INT, aa);
            GetAttributeResult sr = _getAttribute(item, "func", AttributeType::STRING, as);
            if(ar != GET_SUCCESS || sr != GET_SUCCESS){
                mErrorReason = "Script tag does not contain an id.";
                return false;
            }
            if(aa.isVariable || as.isVariable){
                blockList->push_back({_setVariableFlag(TagType::SCRIPT), static_cast<int>(d.vEntry2List->size())});

                VariableAttribute va;
                va._varData = _attributeOutputToChar(aa, AttributeType::INT);
                va.mVarHash = aa.vId;

                VariableAttribute vs;
                vs._varData = _attributeOutputToChar(as, AttributeType::STRING);
                vs.mVarHash = as.vId;
                if(!as.isVariable){
                    vs.i = d.stringList->size();
                    d.stringList->push_back(as.s);
                }

                d.vEntry2List->push_back({va, vs});
            }else{
                blockList->push_back({TagType::SCRIPT, static_cast<int>(d.entry2List->size())});
                int funcIdx = d.stringList->size();
                d.stringList->push_back(as.s);

                d.entry2List->push_back({aa.i, funcIdx});
            }
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

        o.s = c;

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
