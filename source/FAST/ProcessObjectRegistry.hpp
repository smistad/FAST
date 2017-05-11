#ifndef FAST_PROCESS_OBJECT_REGISTRY_HPP_
#define FAST_PROCESS_OBJECT_REGISTRY_HPP_

#include <string>
#include <unordered_map>
#include <functional>
#include "FAST/ProcessObject.hpp"
#include "FAST/Exception.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

/**
 * This class implements the C++ registry pattern which is used to keep
 * a registry of process objects so that we can instantiate process objects
 * using only a string of its name.
 */
class FAST_EXPORT  ProcessObjectRegistry {
    public:
        using ctor_t = std::function<SharedPointer<ProcessObject>()>;
        using map_t = std::unordered_map<std::string, ctor_t>;

        static SharedPointer<ProcessObject> create(const std::string& class_name) {
            if (ctors().count(class_name) == 1) {
                return ctors()[class_name]();
            }
            throw Exception("Process object " + class_name + " not registered in ProcessObjectRegistry.");
        }

        static bool registerPO(const std::string& class_name, const ctor_t& ctor) {
            ctors()[class_name] = ctor;
            return true;
        }

        static bool isPORegistered(const std::string& class_name) {
            return ctors().count(class_name) == 1;
        }

        static void unregisterPO(const std::string& class_name) {
            ctors().erase(class_name);
        }

    private:
        static map_t& ctors() {
            static map_t ctor_map;
            return ctor_map;
        }

        ProcessObjectRegistry();
        ProcessObjectRegistry(const ProcessObjectRegistry& other);
        ProcessObjectRegistry& operator=(const ProcessObjectRegistry& other);
};

#define FAST_REGISTER_DERIVED(Derived) \
[]() -> SharedPointer<ProcessObject> { return Derived::New(); }

#define FAST_REGISTER_PO(Derived)         \
static bool _registered_##Derived = ProcessObjectRegistry::registerPO(#Derived, FAST_REGISTER_DERIVED(Derived));


}

#endif
