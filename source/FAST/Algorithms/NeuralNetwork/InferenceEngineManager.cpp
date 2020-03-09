#include <FAST/Config.hpp>
#include <FAST/Utility.hpp>
#ifndef WIN32
#include <dlfcn.h>
#endif
#include "InferenceEngineManager.hpp"

namespace fast {

bool InferenceEngineManager::m_loaded = false;
std::unordered_map<std::string, std::function<InferenceEngine*()>> InferenceEngineManager::m_engines;

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

std::vector<std::string> InferenceEngineManager::getEngineList() {
    loadAll();
    std::vector<std::string> list;
    for(auto&& engine : m_engines)
        list.push_back(engine.first);
    return list;
}

void InferenceEngineManager::loadAll() {
    if(m_loaded) {
        return;
    }
    std::string prefix = "InferenceEngine";
#ifndef WIN32
    prefix = "lib" + prefix;
#endif
    Reporter::info() << "Loading inference engines in folder " << Config::getLibraryPath() << Reporter::end();
    for(auto&& item : getDirectoryList(Config::getLibraryPath(), true, false)) {
        if(item.substr(0, prefix.size()) == prefix) {
            std::string name = item.substr(prefix.size(), item.rfind('.') - prefix.size());
            Reporter::info() << "Loading inference engine " << name << " from shared library " << item << Reporter::end();
#ifdef WIN32
            SetErrorMode(SEM_FAILCRITICALERRORS); // TODO To avoid diaglog box, when not able to load a DLL
            auto handle = LoadLibrary(item.c_str());
            if(!handle) {
                Reporter::warning() << "Failed to load plugin because " << GetLastErrorAsString() << Reporter::end();
                continue;
            }
            auto load = (InferenceEngine* (*)())GetProcAddress(handle, "load");
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
                Reporter::warning() << "Failed to get address of load function because " << Reporter::end();// dlerror() << Reporter::end();
                continue;
            }
#endif
            m_engines[name] = load;
        }
    }
    m_loaded = true;
}


std::shared_ptr<InferenceEngine> InferenceEngineManager::loadBestAvailableEngine() {
    loadAll();
    if(m_engines.empty())
        throw Exception("No inference engines available on the system");

    if(isEngineAvailable("TensorFlowCUDA"))
        return loadEngine("TensorFlowCUDA");

    if(isEngineAvailable("TensorRT"))
        return loadEngine("TensorRT");

    return loadEngine(getEngineList().front());
}

bool InferenceEngineManager::isEngineAvailable(std::string name) {
    loadAll();
    return m_engines.count(name) > 0;
}

std::shared_ptr<InferenceEngine> InferenceEngineManager::loadEngine(std::string name) {
    loadAll();
    if(m_engines.count(name) == 0)
        throw Exception("Inference engine with name " + name + " is not available");
    // Call the load function which the map stores a handle to
    return std::shared_ptr<InferenceEngine>(m_engines.at(name)());
}

}