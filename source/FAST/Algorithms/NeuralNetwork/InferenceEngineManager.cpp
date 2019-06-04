#include <FAST/Config.hpp>
#include <FAST/Utility.hpp>
#include <dlfcn.h>
#include "InferenceEngineManager.hpp"

namespace fast {

bool InferenceEngineManager::m_loaded = false;
std::unordered_map<std::string, std::shared_ptr<InferenceEngine>> InferenceEngineManager::m_engines;

#ifdef WIN32


//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif

void InferenceEngineManager::loadAll() {
    if(m_loaded) {
        return;
    }
    std::string prefix = "InferenceEngine";
#ifndef WIN32
    prefix = "lib" + prefix;
#endif
    std::cout << "Loading inference engines in folder " << Config::getLibraryPath() << std::endl;
    for(auto&& item : getDirectoryList(Config::getLibraryPath(), true, false)) {
        if(item.substr(0, prefix.size()) == prefix) {
            std::string name = item.substr(prefix.size(), item.rfind('.'));
            Reporter::info() << "Loading inference engine " << name << " from shared library " << item << Reporter::end();
#ifdef WIN32
            auto handle = LoadLibrary(item.c_str());
            if(!handle) {
                Reporter::warning() << "Failed to load plugin because " << GetLastErrorAsString() << Reporter::end();
                continue;
            }
            auto load = (Plugin* (*)())GetProcAddress(handle, "load");
            if(!load) {
                FreeLibrary(handle);
                Reporter::warning() << "Failed to get adress to load function because " << GetLastErrorAsString() << Reporter::end();
                continue;
            }
#else
            auto handle = dlopen(item.c_str(), RTLD_LAZY);
            if(!handle) {
                Reporter::warning() << "Failed to load plugin because " << dlerror() << Reporter::end();
                continue;
            }
            auto load = (InferenceEngine* (*)())dlsym(handle, "load");
            if(!load) {
                dlclose(handle);
                Reporter::warning() << "Failed to get address of load function because " << dlerror() << Reporter::end();
                continue;
            }
#endif
            auto object = load();
            m_engines[object->getName()] = std::shared_ptr<InferenceEngine>(object);
        }
    }
    m_loaded = true;
}

std::shared_ptr<InferenceEngine> InferenceEngineManager::getEngine(std::string name) {
    loadAll();
    if(!isEngineAvailable(name))
        throw Exception("Inference engine " + name + " is not available.");
    return m_engines[name];
}

std::shared_ptr<InferenceEngine> InferenceEngineManager::getBestAvailableEngine() {
    loadAll();
    if(m_engines.size() == 0)
        throw Exception("No inference engines available on the system");

    if(isEngineAvailable("TensorFlow"))
        return m_engines["TensorFlow"];

    if(isEngineAvailable("TensorRT"))
        return m_engines["TensorRT"];

    return m_engines.begin()->second;
}

bool InferenceEngineManager::isEngineAvailable(std::string name) {
    loadAll();
    return m_engines.count(name) > 0;
}

}