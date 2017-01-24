#include "Config.hpp"
#include "Exception.hpp"
#include "Utility.hpp"
#include <fstream>

// Includes needed to get path of executable
#if defined(__APPLE__) || defined(__MACOSX)
#include <mach-o/dyld.h>
#elif _WIN32
#include <windows.h>
#else
#include <zconf.h>
#endif

namespace fast {

std::string getPath() {
    std::string path;
	std::string slash = "/";
#if defined(__APPLE__) || defined(__MACOSX)
    char exepath[1024] = {0};
    uint32_t size = sizeof(exepath);
    int ret = _NSGetExecutablePath(exepath, &size);
    if(0 != ret)
        throw Exception("Error getting executable path in getPath()");
    path = std::string(exepath);
    int lastSlashPos = path.rfind("/./");
    path = path.substr(0, lastSlashPos);
    if(path.substr(path.size()-4) == "/bin") {
        lastSlashPos = path.rfind(slash);
        path = path.substr(0, lastSlashPos);
    }
    path = path + slash;
#else
#ifdef _WIN32
    char exepath[MAX_PATH + 1] = {0};
    DWORD ret = GetModuleFileNameA(NULL, exepath, sizeof(exepath));
    if(ret == 0)
        throw Exception("Error reading module file name in getPath()");
	slash = "\\";
#else
    char exepath[PATH_MAX + 1] = {0};
    ssize_t ret = readlink("/proc/self/exe", exepath, 1024);
    if(ret == -1)
        throw Exception("Error reading /proc/self/exe in getPath()");
#endif
    // Find last / and remove binary name
    path = std::string(exepath);
    int lastSlashPos = path.rfind(slash);
    path = path.substr(0, lastSlashPos);
    lastSlashPos = path.rfind(slash);
    path = path.substr(0, lastSlashPos);
    path = path + slash;
#endif


    return path;
}

// Initialize statics
bool Config::mConfigurationLoaded = false;

// Default paths
std::string Config::mTestDataPath = getPath() + "../data/";
std::string Config::mKernelSourcePath = getPath() + "../source/FAST/";
std::string Config::mKernelBinaryPath = getPath() + "kernel_binaries/";

std::string Config::getTestDataPath() {
    loadConfiguration();
    return mTestDataPath;
}

std::string Config::getKernelSourcePath() {
    loadConfiguration();
    return mKernelSourcePath;
}

std::string Config::getKernelBinaryPath() {
    loadConfiguration();
    return mKernelBinaryPath;
}

void Config::loadConfiguration() {
    if(mConfigurationLoaded)
        return;

    // Read and parse configuration file
    // It should reside in the build folder when compiling, and in the root folder when using release
    std::string filename = getPath() + "fast_configuration.txt";
    std::ifstream file(filename);
    if(!file.is_open()) {
        Reporter::warning() << "Unable to open the configuration file " << filename << " using defaults." << Reporter::end;
        mConfigurationLoaded = true;
        return;
    }
    Reporter::info() << "Loaded configuration file: " << filename << Reporter::end;

    std::string line;
    std::getline(file, line);
    while(!file.eof()) {
        trim(line);
        if(line[0] == '#' || line.size() == 0) {
            // Comment or empty line, skip
            std::getline(file, line);
            continue;
        }
        std::vector<std::string> list = split(line, "=");
        if(list.size() != 2) {
            throw Exception("Error parsing configuration file. Expected key = value pair, got: " + line);
        }

        std::string key = list[0];
        std::string value = list[1];
        trim(key);
        trim(value);
        value = replace(value, "@ROOT@", getPath());

        if(key == "TestDataPath") {
            mTestDataPath = value;
        } else if(key == "KernelSourcePath") {
            mKernelSourcePath = value;
        } else if(key == "KernelBinaryPath") {
            mKernelBinaryPath = value;
        } else {
            throw Exception("Error parsing configuration file. Unrecognized key: " + key);
        }

        std::getline(file, line);
    }
    file.close();

    mConfigurationLoaded = true;
}

}; // end namespace fast
