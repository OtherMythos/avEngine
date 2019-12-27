#pragma once

#include <string>
#include "DialogScriptData.h"

namespace tinyxml2{
    class XMLDocument;
    class XMLElement;
};

namespace AV{

    struct CompiledDialog;

    /**
    A class which is capable of compiling a dialog script file.

    The files are compiled from their original xml state to a machine recognisable form.
    */
    class DialogCompiler{
    public:
        DialogCompiler();
        ~DialogCompiler();

        /**
        Compile a dialog xml file into the contents of outData.

        @returns
        True or false depending on whether any fatal errors occurred.
        */
        bool compileScript(const std::string& filePath, CompiledDialog& outData);

    private:
        bool _initialScanDocument(tinyxml2::XMLDocument &xmlDoc, CompiledDialog& d);
        bool _parseBlock(tinyxml2::XMLElement *e, CompiledDialog& d);
        bool _parseDialogTag(tinyxml2::XMLElement *item, CompiledDialog& d, BlockContentList* blockList);

        const char* mErrorReason = 0;
    };
}
