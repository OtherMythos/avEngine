#include "AsyncTextureTicketUserData.h"

#include "TextureBoxUserData.h"
#include "TextureUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreAsyncTextureTicket.h"
#include "OgrePixelFormatGpuUtils.h"
#include "OgreRenderSystem.h"
#include "OgreRoot.h"
#include "OgreTextureGpu.h"
#include "OgreTextureGpuManager.h"

#include <memory>

namespace AV{

    SQObject AsyncTextureTicketUserData::asyncTextureTicketDelegateTableObject;

    //Ogre keeps a ticket's status to itself and only asserts on a script's mistakes, so
    //what the binding has to refuse is tracked here.
    struct AsyncTextureTicketUserData::TicketContents{
        Ogre::AsyncTextureTicket* ticket;
        bool downloaded;
        //Set while mapped, and shared with the box that mapping handed out.
        std::shared_ptr<bool> mapped;
    };

    void AsyncTextureTicketUserData::asyncTextureTicketToUserData(HSQUIRRELVM vm, Ogre::AsyncTextureTicket* ticket){
        TicketContents* contents = (TicketContents*)sq_newuserdata(vm, sizeof(TicketContents));
        new(contents)TicketContents({ticket, false, nullptr});

        sq_pushobject(vm, asyncTextureTicketDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AsyncTextureTicketTypeTag);
        sq_setreleasehook(vm, -1, asyncTextureTicketReleaseHook);
    }

    UserDataGetResult AsyncTextureTicketUserData::_readTicketFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TicketContents** outContents){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AsyncTextureTicketTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outContents = static_cast<TicketContents*>(pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger AsyncTextureTicketUserData::asyncTextureTicketReleaseHook(SQUserPointer p, SQInteger size){
        TicketContents* contents = static_cast<TicketContents*>(p);

        if(contents->mapped) *contents->mapped = false;
        Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()->destroyAsyncTextureTicket(contents->ticket);
        contents->~TicketContents();

        return 0;
    }

    const char* AsyncTextureTicketUserData::_checkTexture(Ogre::AsyncTextureTicket* ticket, Ogre::TextureGpu* texture, Ogre::uint8 mipLevel){
        //All of these are only asserts in Ogre.
        const Ogre::TextureBox box = texture->getEmptyBox(mipLevel);
        if(box.width != ticket->getWidth() || box.height != ticket->getHeight()){
            return "Texture does not match the size of this ticket.";
        }
        if(Ogre::PixelFormatGpuUtils::getFamily(texture->getPixelFormat()) != ticket->getPixelFormatFamily()){
            return "Texture does not match the format of this ticket.";
        }
        if(texture->getTextureType() != Ogre::TextureTypes::Type2D){
            return "Only 2D textures can be downloaded.";
        }
        if(texture->isMultisample()){
            return "Multisampled textures can't be downloaded.";
        }
        //Handed a texture still streaming in, Ogre would hold the download until it
        //arrived and silently drop it if it never did, so the transfer would never finish.
        if(!texture->isDataReady()){
            return "Texture is not resident and ready.";
        }
        return 0;
    }

    SQInteger AsyncTextureTicketUserData::download(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        Ogre::TextureGpu* texture;
        bool userOwned, isValid;
        SCRIPT_CHECK_RESULT(TextureUserData::readTextureFromUserData(vm, 2, &texture, &userOwned, &isValid));
        if(!isValid) return sq_throwerror(vm, "Texture is invalid.");
        //A window's texture only exists for the frame it's presented in, and on Metal
        //its drawable can't be copied from at all.
        if(!userOwned) return sq_throwerror(vm, "Only textures created by script can be downloaded.");

        SQInteger mipLevel;
        SQBool accurateTracking;
        sq_getinteger(vm, 3, &mipLevel);
        sq_getbool(vm, 4, &accurateTracking);

        if(mipLevel < 0 || mipLevel >= texture->getNumMipmaps()) return sq_throwerror(vm, "Texture has no such mip level.");
        const Ogre::uint8 mip = static_cast<Ogre::uint8>(mipLevel);

        const char* problem = _checkTexture(contents->ticket, texture, mip);
        if(problem) return sq_throwerror(vm, problem);

        if(contents->mapped) return sq_throwerror(vm, "Ticket is mapped: call unmap first.");
        if(!contents->ticket->queryIsTransferDone()) return sq_throwerror(vm, "A download is still in flight.");

        WRAP_OGRE_ERROR(
            contents->ticket->download(texture, mip, accurateTracking != 0);
        )
        contents->downloaded = true;

        return 0;
    }

    SQInteger AsyncTextureTicketUserData::queryIsTransferDone(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        sq_pushbool(vm, contents->ticket->queryIsTransferDone());

        return 1;
    }

    SQInteger AsyncTextureTicketUserData::map(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        SQInteger slice;
        sq_getinteger(vm, 2, &slice);

        //A ticket never downloaded into maps happily, as whatever its buffer holds.
        if(!contents->downloaded) return sq_throwerror(vm, "Nothing has been downloaded.");
        if(contents->mapped) return sq_throwerror(vm, "Ticket is already mapped.");
        if(slice < 0 || slice >= static_cast<SQInteger>(contents->ticket->getNumSlices())){
            return sq_throwerror(vm, "Ticket has no such slice.");
        }

        Ogre::TextureBox box;
        WRAP_OGRE_ERROR(
            box = contents->ticket->map(static_cast<Ogre::uint32>(slice));
        )
        contents->mapped = std::make_shared<bool>(true);
        TextureBoxUserData::mappedTextureBoxToUserData(vm, &box, contents->mapped);

        return 1;
    }

    SQInteger AsyncTextureTicketUserData::unmap(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        if(!contents->mapped) return sq_throwerror(vm, "Ticket is not mapped.");

        *contents->mapped = false;
        contents->mapped.reset();
        WRAP_OGRE_ERROR(
            contents->ticket->unmap();
        )

        return 0;
    }

    SQInteger AsyncTextureTicketUserData::getWidth(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        sq_pushinteger(vm, static_cast<SQInteger>(contents->ticket->getWidth()));

        return 1;
    }

    SQInteger AsyncTextureTicketUserData::getHeight(HSQUIRRELVM vm){
        TicketContents* contents;
        SCRIPT_ASSERT_RESULT(_readTicketFromUserData(vm, 1, &contents));

        sq_pushinteger(vm, static_cast<SQInteger>(contents->ticket->getHeight()));

        return 1;
    }

    void AsyncTextureTicketUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        /**SQFunction
        @name download
        @desc Start copying a texture to the CPU. Use queryIsTransferDone to learn when it has finished. Usually returns at once,
        but on Vulkan it has been seen to wait for the whole copy. @see docs/texture-readback.md
        The texture must have been created by script, and match this ticket's size and format.
        Throws if a download is in flight or the ticket is mapped.
        @param1:Texture: The texture to copy.
        @param2:Integer: The mip level to copy.
        @param3:Bool: Accurate tracking. True flushes the GPU work queued so far, so the transfer can finish as soon as possible
        and be reported exactly. False costs nothing extra, and the transfer is reported some frames later.
        */
        ScriptUtils::addFunction(vm, download, "download", 4, ".uib");
        /**SQFunction
        @name queryIsTransferDone
        @desc Whether the last download has finished, so map won't have to wait. Never blocks.
        True until the next download, and also on a ticket which has never had one.
        @return: True if no download is in flight.
        */
        ScriptUtils::addFunction(vm, queryIsTransferDone, "queryIsTransferDone");
        /**SQFunction
        @name map
        @desc Make the last download readable. Waits for it if it hasn't finished, stalling the game until the GPU catches up.
        Throws if nothing has been downloaded or the ticket is already mapped.
        @param1:Integer: The slice to map.
        @return: A read only TextureBox, which throws if read after unmap.
        */
        ScriptUtils::addFunction(vm, map, "map", 2, ".i");
        /**SQFunction
        @name unmap
        @desc End a map. Required before the next download.
        */
        ScriptUtils::addFunction(vm, unmap, "unmap");
        /**SQFunction
        @name getWidth
        @desc The width of texture this ticket downloads.
        */
        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        /**SQFunction
        @name getHeight
        @desc The height of texture this ticket downloads.
        */
        ScriptUtils::addFunction(vm, getHeight, "getHeight");

        sq_resetobject(&asyncTextureTicketDelegateTableObject);
        sq_getstackobj(vm, -1, &asyncTextureTicketDelegateTableObject);
        sq_addref(vm, &asyncTextureTicketDelegateTableObject);
        sq_pop(vm, 1);
    }
}
