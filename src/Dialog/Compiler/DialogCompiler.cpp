#include "DialogCompiler.h"

#include "tinyxml2.h"

#include "Logger/Log.h"

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
            mErrorReason = "The first entry of a dialog script should be named Dialog_Script.";
            return false;
        }

        d.blockMap = new BlockMapType();
        d.stringList = new StringListType();

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
            int targetPos = d.stringList->size();
            d.stringList->push_back(t);
            blockList->push_back({TagType::TEXT_STRING, targetPos});
        }
        else if(strcmp(n, "jmp") == 0){
            //At some point I need to figure out if there actually is a block with that id.
            int target = item->IntAttribute("id", -1);
            if(target < 0){
                mErrorReason = "jmp tags should include an attribute named id which refers to a valid dialog block.";
                return false;
            }

            blockList->push_back({TagType::JMP, target});
        }
        else if(strcmp(n, "sleep") == 0){
            int out = -1;
            tinyxml2::XMLError e = item->QueryIntAttribute("l", &out);
            if(e != tinyxml2::XML_SUCCESS){
                mErrorReason = "Include a positive time value in milliseconds for a sleep command.";
                return false;
            }
            blockList->push_back({TagType::SLEEP, out});
        }

        return true;
    }

}
