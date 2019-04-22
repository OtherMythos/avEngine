#pragma once

namespace AV{
    class JobDispatcher;

    //Id of a job.
    //This is to encapsulate the actual id of the job within a private scope.
    struct JobId{
        friend JobDispatcher;
    public:
        //0 represents an invalid id.
        JobId() : _id(0) {}

        bool operator == (const JobId &other) const { return _id == other.id(); }

        uint64_t id() const { return _id; }
        static const JobId INVALID;
    private:
        JobId(uint64_t i) : _id(i) {}

        uint64_t _id;
    };
}
