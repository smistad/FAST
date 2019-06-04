#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>

namespace fast {

/**
 * A purely static class to dynamically load inference engines at runtime
 */
class InferenceEngineManager {
    public:
        static void loadAll();
        static std::shared_ptr<InferenceEngine> getEngine(std::string name);
        static std::shared_ptr<InferenceEngine> getBestAvailableEngine();
        static bool isEngineAvailable(std::string name);
    private:
        static std::unordered_map<std::string, std::shared_ptr<InferenceEngine>> m_engines;
        static bool m_loaded;
};

}