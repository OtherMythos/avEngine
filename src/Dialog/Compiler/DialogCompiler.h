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

        /**
        Scan a piece of dialog string for variables.
        Variables would be represented like this:
            'This is a global name $nameVariable$ and this is a local name #nameVariable#'
        The two different delimiters need to be properly terminated, so if a matching $ or # is not found then an error will be thrown.
        Another $ or # cannot appear inside a variable declaration.

        @return
        Positive int of size n depending on how many valid declarations are found. 0 if none.
        A maximum of 4 variables is allowed in a string.
        -1 if the declaration is malformed.
        -2 if more than 4 variables are provided.
        */
        int _scanStringForVariables(const char* c);

        const char* mErrorReason = 0;
    };
}
