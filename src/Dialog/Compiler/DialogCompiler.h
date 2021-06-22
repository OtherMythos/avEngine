#pragma once

#include <string>
#include "DialogScriptData.h"
#include "OgrePrerequisites.h"

namespace tinyxml2{
    class XMLDocument;
    class XMLElement;
};

namespace AV{

    struct CompiledDialog;

    struct AttributeOutput{
        union{
            int i;
            float f;
            bool b;
            const char* s;

            Ogre::uint32 vId;
        };
        bool isVariable;
        bool globalVariable;
    };

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

        /**
        Compile a dialog script stored in a string.

        @returns
        True or false depending on whether any fatal errors occurred.
        */
        bool compileScript(const char* scriptContent, CompiledDialog& outData);

    private:
        bool _initialScanDocument(tinyxml2::XMLDocument &xmlDoc, CompiledDialog& d);
        bool _parseBlock(tinyxml2::XMLElement *e, CompiledDialog& d);
        bool _parseDialogTag(tinyxml2::XMLElement *item, CompiledDialog& d, BlockContentList* blockList);

        /**
        Parse a single script declaration tag.
        These tags are intended to be standalone and separate from actual blocks.
        */
        bool _parseScriptDeclaration(tinyxml2::XMLElement *e, CompiledDialog& d);

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

        enum GetAttributeResult{
            GET_SUCCESS,
            GET_NOT_FOUND,
            GET_TYPE_MISMATCH
        };

        /**
        Get an attribute from an xml item.
        This function will scan the attribute for variables as well as constant values.
        You are able to specify the type of the value you expect, which will be honored even if a variable is provided.
        Variable values need to be read at run-time, and if there is a type mismatch the system will throw.
        */
        GetAttributeResult _getAttribute(tinyxml2::XMLElement *item, const char* name, AttributeType t, AttributeOutput& o) const;

        /**
        Query an attribute from an xml item.
        This function performs no checks of type, and should be used if the user doesn't know what type the named attribute will be.

        @returns Either GET_SUCCESS or GET_NOT_FOUND. It will never return GET_TYPE_MISMATCH.
        */
        GetAttributeResult _queryAttribute(tinyxml2::XMLElement *item, const char* name, AttributeType* outType, AttributeOutput& o);

        char _attributeOutputToChar(const AttributeOutput& o, AttributeType t);
        char _BlankChar(AttributeType t);

        const char* mErrorReason = 0;
    };
}
