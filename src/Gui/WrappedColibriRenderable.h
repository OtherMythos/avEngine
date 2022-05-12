#pragma once

//Workaround for datablock access for panels.
//I'd like to be able to set a datablock without having to change the entire material.
//Wrapping the object allows direct access to that functionality.
namespace Colibri{
    class WrappedColibriRenderable : public Renderable{
    public:
        WrappedColibriRenderable(ColibriManager *manager) : Renderable(manager) { }

        void setDatablockAll(Ogre::IdString name){
            for(int i = 0; i < States::NumStates; i++){
                m_stateInformation[i].materialName = name;
            }
        }
    };
};