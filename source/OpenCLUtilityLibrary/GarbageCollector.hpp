#ifndef GARBAGECOLLECTOR_HPP_
#define GARBAGECOLLECTOR_HPP_

#include <boost/shared_ptr.hpp>
#include "CL/OpenCL.hpp"
#include <set>

namespace oul {
/**
 * The purpose of the garbage collector object is to allow the explicit deletion
 * of OpenCL memory objects such as buffers and images. This is useful when you
 * want to delete an OpenCL memory object at a specific location in your code
 * instead of waiting for the object to reach the end of its scope.
 */
class GarbageCollector {
    public:
        void addMemoryObject(cl::Memory *);
        void deleteMemoryObject(cl::Memory *);
        void deleteAllMemoryObjects();
    private:
        std::set<cl::Memory *> memoryObjects;

};
typedef boost::shared_ptr<class GarbageCollector> GarbageCollectorPtr;

}

#endif /* GARBAGECOLLECTOR_HPP_ */
