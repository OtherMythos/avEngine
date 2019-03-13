#pragma once

#include "Scripting/ScriptNamespace/ScriptNamespace.h"
#include "Scripting/ScriptNamespace/EntityNamespace.h"

namespace AV{
    class ComponentNamespace : public ScriptNamespace{
    public:

        template <typename T, typename ... Args>
        static T* _addComponent(HSQUIRRELVM v, Args && ... args){
          //entityx::Entity e = getEntityHandle(v);
          eId eid = EntityNamespace::_getEID(v, -1);

          // if(e.valid() && !e.has_component<T>()){
          //   T* comp = (e.assign<T>(std::forward<Args>(args) ...)).get();
          //   sq_pushbool(v, true);
          //   return comp;
          // }else{
          //   if(!e.valid()) std::cerr << "That entity is not valid." << '\n';
          //   else std::cerr << "Could not create the component. That entity already has that component." << '\n';
          //   sq_pushbool(v, false);
          // }

          //No component was created, so return 0 as the pointer.
          return 0;
        }
    };
}
