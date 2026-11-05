#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class AsyncTextureTicket;
    class TextureGpu;
}

namespace AV{
    class AsyncTextureTicketUserData{
    private:
        struct TicketContents;
    public:
        AsyncTextureTicketUserData() = delete;
        ~AsyncTextureTicketUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        //Takes ownership of the ticket.
        static void asyncTextureTicketToUserData(HSQUIRRELVM vm, Ogre::AsyncTextureTicket* ticket);

    private:
        static SQObject asyncTextureTicketDelegateTableObject;

        static SQInteger download(HSQUIRRELVM vm);
        static SQInteger queryIsTransferDone(HSQUIRRELVM vm);
        static SQInteger map(HSQUIRRELVM vm);
        static SQInteger unmap(HSQUIRRELVM vm);
        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);

        //Why this mip of the texture can't be downloaded into the ticket, or null if it can.
        static const char* _checkTexture(Ogre::AsyncTextureTicket* ticket, Ogre::TextureGpu* texture, Ogre::uint8 mipLevel);
        static UserDataGetResult _readTicketFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TicketContents** outContents);
        static SQInteger asyncTextureTicketReleaseHook(SQUserPointer p, SQInteger size);
    };
}
