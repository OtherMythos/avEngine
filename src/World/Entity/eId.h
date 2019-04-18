#pragma once

#include <stdint.h>

namespace AV{
    /**
    An id of an entity.
    This is very similar to the id from entity x.
    I just copied it so I could have my own without including any entityx specifics.
    It references an entity in the entity manager.
    */
    struct eId {
    public:
        eId() : id_(0) {}
        explicit eId(uint64_t id) : id_(id) {}

        uint64_t id() const { return id_; }

        bool operator==(const eId &e) const{
            return e.id() == id_;
        }

        bool operator<(const eId &e) const{
            return e.id() < id_;
        }

    private:
        uint64_t id_;

    public:
        static const eId INVALID;
    };
}

#define _eId(x) eId(x.id().id())
