#include "GarbageCollector.hpp"

void oul::GarbageCollector::addMemoryObject(cl::Memory* object) {
    memoryObjects.insert(object);
}

void oul::GarbageCollector::deleteMemoryObject(cl::Memory* object) {
    memoryObjects.erase(object);
    delete object;
    object = NULL;
}

void oul::GarbageCollector::deleteAllMemoryObjects() {
    std::set<cl::Memory *>::iterator it;
    for(it = memoryObjects.begin(); it != memoryObjects.end(); it++) {
        cl::Memory * mem = *it;
        delete (mem);
        mem = NULL;
    }
    memoryObjects.clear();
}
