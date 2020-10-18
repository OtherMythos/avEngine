/**
Originally taken from Ogre.
This version has the Ogre namespace removed, and the debug readable string system removed.
If possible the Ogre version should be used as the extra debug checking is useful.
*/


#pragma once

#include "OgrePrerequisites.h"
#include "Hash/MurmurHash3.h"
#include "System/EnginePrerequisites.h"
#include <stdio.h>  // sprintf
#include <string.h> // strlen
#include <string>

#define AV_HASH_FUNC Ogre::MurmurHash3_x86_32
#define AV_HASH_BITS 32
#include "assert.h"

namespace AV
{
    struct IdString
    {
        static const uint32_t Seed = 0x3A8EFA67; //It's a prime number :)

        uint32      mHash;

        IdString() : mHash( 0 )
        {
        }

        IdString( const char *string ) : mHash( 0 )
        {
            AV_HASH_FUNC( string, static_cast<int>(strlen( string )), Seed, &mHash );
        }

        IdString( const std::string &string ) : mHash( 0 )
        {
            AV_HASH_FUNC( string.c_str(), static_cast<int>(string.size()), Seed, &mHash );
        }

        IdString( uint32 value ) : mHash( 0 )
        {
            AV_HASH_FUNC( &value, sizeof( value ), Seed, &mHash );
        }

        void operator += ( IdString idString )
        {
            uint32 doubleHash[2];
            doubleHash[0] = mHash;
            doubleHash[1] = idString.mHash;

            AV_HASH_FUNC( &doubleHash, sizeof( doubleHash ), Seed, &mHash );
        }

        IdString operator + ( IdString idString ) const
        {
            IdString retVal( *this );
            retVal += idString;
            return retVal;
        }

        bool operator < ( IdString idString ) const
        {
            return mHash < idString.mHash;
        }

        bool operator == ( IdString idString ) const
        {
            return mHash == idString.mHash;
        }

        bool operator != ( IdString idString ) const
        {
            return mHash != idString.mHash;
        }

        /// Returns "[Hash 0x0a0100ef]" strings in Release mode, readable string in debug
        std::string getFriendlyText() const
        {
            return getReleaseText();
        }

        /// Always returns "[Hash 0x0a0100ef]" strings in any mode
        std::string getReleaseText() const
        {
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( push )
            #pragma warning( disable: 4996 ) //Unsecure CRT deprecation warning
        #endif

            char tmp[(AV_HASH_BITS >> 2)+10];
            sprintf( tmp, "[Hash 0x%.8x]", mHash );
            tmp[(AV_HASH_BITS >> 2)+10-1] = '\0';
            return std::string( tmp );

        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( pop )
        #endif
        }

        /** C String version. Zero allocations.
        @param outCStr
            C String to store the string.
        @param stringSize
            Size of of outCStr. Recommended size: OGRE_DEBUG_STR_SIZE
        */
        void getFriendlyText( char *outCStr, size_t stringSize ) const
        {
            getReleaseText( outCStr, stringSize );
        }

        /// C String version. Zero allocations. See getFriendlyText.
        void getReleaseText( char *outCStr, size_t stringSize ) const
        {
        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( push )
            #pragma warning( disable: 4996 ) //Unsecure CRT deprecation warning
        #endif

            if( stringSize < (AV_HASH_BITS >> 2u)+10u )
            {
                //Not big enough. Use a temp buffer and then copy + truncate.
                char tmp[(AV_HASH_BITS >> 2)+10];
                sprintf( tmp, "[Hash 0x%.8x]", mHash );
                tmp[(AV_HASH_BITS >> 2)+10-1] = '\0';

                memcpy( outCStr, tmp, stringSize );
                outCStr[stringSize - 1u] = '\0';
            }
            else
            {
                //Write directly to the output buffer. It's big enough.
                sprintf( outCStr, "[Hash 0x%.8x]", mHash );
                outCStr[(AV_HASH_BITS >> 2)+10-1] = '\0';
            }

        #if OGRE_COMPILER == OGRE_COMPILER_MSVC
            #pragma warning( pop )
        #endif
        }
    };
}

