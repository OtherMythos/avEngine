#include "PlaneUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Vector3UserData.h"
#include "OgrePlane.h"

#include <sstream>

namespace AV{

    SQObject PlaneUserData::planeDelegateTableObject;

    void PlaneUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 2);

        ScriptUtils::addFunction(vm, planeToString, "_tostring");
        ScriptUtils::addFunction(vm, planeCompare, "_cmp");

        sq_resetobject(&planeDelegateTableObject);
        sq_getstackobj(vm, -1, &planeDelegateTableObject);
        sq_addref(vm, &planeDelegateTableObject);
        sq_pop(vm, 1);

        //Create the creation functions.
        sq_pushroottable(vm);

        {
            ScriptUtils::addFunction(vm, createPlane, "Plane", -1, ".uu|nu");
        }

        sq_pop(vm, 1);
    }

    SQInteger PlaneUserData::createPlane(HSQUIRRELVM vm){
        SQInteger size = sq_gettop(vm);

        Ogre::Plane p;
        if(size == 3){
            Ogre::Vector3 first;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &first));

            SQObjectType t = sq_gettype(vm, 3);
            if(t == OT_FLOAT || t == OT_INTEGER){
                SQFloat floatVal;
                sq_getfloat(vm, 3, &floatVal);

                p = Ogre::Plane(first, floatVal);
            }else{
                Ogre::Vector3 second;
                SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &second));

                p = Ogre::Plane(first, second);
            }
        }
        else if(size == 4){
            Ogre::Vector3 p0, p1, p2;
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &p0));
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &p1));
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 4, &p2));

            p = Ogre::Plane(p0, p1, p2);
        }
        else if(size == 1){
            //Do nothing, this is just an empty plane.
        }
        else{
            return sq_throwerror(vm, "Invalid number of parameters");
        }

        planeToUserData(vm, p);

        return 1;
    }

    SQInteger PlaneUserData::planeToString(HSQUIRRELVM vm){
        Ogre::Plane* outPlane;
        SCRIPT_ASSERT_RESULT(_readPlanePtrFromUserData(vm, -1, &outPlane));

        std::ostringstream stream;
        stream << *outPlane;
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    SQInteger PlaneUserData::planeCompare(HSQUIRRELVM vm){
        Ogre::Plane* first;
        Ogre::Plane* second;

        SCRIPT_ASSERT_RESULT(_readPlanePtrFromUserData(vm, -2, &first));
        SCRIPT_CHECK_RESULT(_readPlanePtrFromUserData(vm, -1, &second));

        if(*first == *second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushinteger(vm, 2);
        }
        return 1;
    }

    void PlaneUserData::planeToUserData(HSQUIRRELVM vm, const Ogre::Plane& vec){
        Ogre::Plane* pointer = (Ogre::Plane*)sq_newuserdata(vm, sizeof(Ogre::Plane));
        *pointer = vec;

        sq_pushobject(vm, planeDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, PlaneTypeTag);
    }

    UserDataGetResult PlaneUserData::readPlaneFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Plane* outPlane){
        Ogre::Plane* planePtr;
        UserDataGetResult result = _readPlanePtrFromUserData(vm, stackInx, &planePtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outPlane = *planePtr;
        return result;
    }

    UserDataGetResult PlaneUserData::_readPlanePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Plane** outPlane){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != PlaneTypeTag){
            *outPlane = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outPlane = (Ogre::Plane*)pointer;

        return USER_DATA_GET_SUCCESS;
    }
}
