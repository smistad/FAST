#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <set>
#include "InferenceEngine.hpp"
#include "FAST/Exception.hpp"


namespace fast {

/**
 * This class implements the C++ registry pattern which is used to keep
 * a registry of inference engine objects so that we can instantiate these objects
 * using only a string of its name.
 */
class FAST_EXPORT InferenceEngineRegistry {
    public:
        using ctor_t = std::function<SharedPointer<InferenceEngine>()>;
        using map_t = std::unordered_map<std::string, ctor_t>;

        static SharedPointer<InferenceEngine> create(const std::string& class_name) {
            if (ctors().count(class_name) == 1) {
                return ctors()[class_name]();
            }
            throw Exception("Inference engine object " + class_name + " not registered in InferenceEngineRegistry.");
        }

        /**
         * Create and return the engine that is preferred and available on this build.
         * @return
         */
        static SharedPointer<InferenceEngine> createPreferredEngine() {
            if(getList().empty()) {
                throw Exception("No inference engines were compiled with FAST, unable to run neural network inference.");
            }
            std::vector<std::string> preferred = {"TensorFlow", "TensorRT"}; // Fastest
            for(const auto& class_name : preferred) {
                if(ctors().count(class_name) == 1) {
                    return ctors()[class_name]();
                }
            }
            // If none of the preferred are available, just use the first one
            return ctors()[*getList().begin()]();
        }

        static bool registerIE(const std::string& class_name, const ctor_t& ctor) {
            ctors()[class_name] = ctor;
            return true;
        }

        static bool isIERegistered(const std::string& class_name) {
            return ctors().count(class_name) == 1;
        }

        static void unregisterIE(const std::string& class_name) {
            ctors().erase(class_name);
        }

        static std::set<std::string> getList() {
            std::set<std::string> keys;
            auto map = ctors();
            for(const auto &entry : map)
                keys.insert(entry.first);
            return keys;
        }

    private:
        static map_t& ctors() {
            static map_t ctor_map;
            return ctor_map;
        }

        InferenceEngineRegistry() = delete;
        InferenceEngineRegistry(const InferenceEngineRegistry& other) = delete;
        InferenceEngineRegistry& operator=(const InferenceEngineRegistry& other) = delete;
};

#define FAST_REGISTER_DERIVED_IE(Derived) \
[]() -> SharedPointer<InferenceEngine> { return Derived::New(); }

#define FAST_REGISTER_IE(Derived)         \
static bool _registered_##Derived = InferenceEngineRegistry::registerIE(#Derived, FAST_REGISTER_DERIVED_IE(Derived##Engine));


}

