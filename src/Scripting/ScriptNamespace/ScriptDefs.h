#pragma once

#define SCRIPT_CHECK_WORLD() \
    World *world = WorldSingleton::getWorld(); \
    if(!world) return sq_throwerror(vm, "The world does not exist.");

#define SCRIPT_CHECK_RESULT(x) \
    if(x != USER_DATA_GET_SUCCESS) return sq_throwerror(vm, ScriptUtils::checkResultErrorMessage(x) );

#define SCRIPT_ASSERT_RESULT(XX) \
    UserDataGetResult __scriptError = XX; \
    assert(__scriptError == USER_DATA_GET_SUCCESS);

#define ASSERT_SQ_RESULT(xx) \
    bool __SQresult = SQ_SUCCEEDED(xx); \
    assert(__SQresult);

#define WRAP_OGRE_ERROR(__xx__) \
    try { __xx__ } catch(Ogre::Exception& e){ \
        return sq_throwerror(vm, e.getDescription().c_str()); \
    }

#define WRAP_STD_ERROR(__xx__) \
    try { __xx__ } catch(std::exception& e){ \
        return sq_throwerror(vm, e.what()); \
    }

#define CHECK_PHYSICS() \
    if(SystemSettings::getPhysicsCompletelyDisabled()) return sq_throwerror(vm, "Physics is disabled.");

#define CHECK_DYNAMIC_PHYSICS() \
    if(SystemSettings::getDynamicPhysicsDisabled()) return sq_throwerror(vm, "Physics is disabled.");

#define ASSERT_DYNAMIC_PHYSICS() \
    assert(!SystemSettings::getDynamicPhysicsDisabled());

#define SCRIPT_RETURN_OGRE_ERROR(z, x) \
    std::string s(z); \
    s += x.getDescription(); \
    return sq_throwerror(vm, s.c_str()); \

namespace AV{
    //Specifies the result of functions which retrieve values from user data.
    enum UserDataGetResult{
        USER_DATA_GET_SUCCESS,
        USER_DATA_GET_TYPE_MISMATCH, //The type tags did not match up, i.e a user data was provided but it didn't have a matching tag.
        USER_DATA_GET_INCORRECT_TYPE, //Something other than a user data was found.
        USER_DATA_ERROR //A generic error
    };
}
