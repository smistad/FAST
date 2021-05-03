#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>

namespace fast {

/**
 * @brief A purely static class to dynamically load inference engines at runtime
 */
class FAST_EXPORT InferenceEngineManager {
    public:
        static std::vector<std::string> getEngineList();
        static void loadAll();
        static std::shared_ptr<InferenceEngine> loadEngine(std::string name);
        static std::shared_ptr<InferenceEngine> loadBestAvailableEngine();
        static std::shared_ptr<InferenceEngine> loadBestAvailableEngine(ModelFormat modelFormat);
        static bool isEngineAvailable(std::string name);
    private:
        static bool m_loaded;
        static std::unordered_map<std::string, std::function<InferenceEngine*()>> m_engines;
};

}