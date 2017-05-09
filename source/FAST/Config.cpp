#include "Config.hpp"
#include "Exception.hpp"
#include "Utility.hpp"
#include <fstream>

// Includes needed to get path of dynamic library
#ifdef _WIN32
#include <windows.h>
#else
#define __USE_GNU 1
#include <dlfcn.h>
#endif

namespace fast {

std::string Config::getPath() {
    if(mBasePath != "")
        return mBasePath;
    std::string path;
	char slash = '/';
    // Find path of the FAST dynamic library
    // The fast_configuration.txt file should lie in the folder below
#ifdef _WIN32
    // http://stackoverflow.com/questions/6924195/get-dll-path-at-runtime
    HMODULE hm = NULL;

    if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR) &localFunc,
            &hm)) {
        int ret = GetLastError();
        throw Exception("Error reading dyanmic library address in getPath()");
    }
    char dlpath[MAX_PATH];
    GetModuleFileNameA(hm, path, sizeof(path));
    slash = '\\'; // use this slash on windows
#else
    // http://stackoverflow.com/questions/1681060/library-path-when-dynamically-loaded
    Dl_info dl_info;
    int ret = dladdr((void *)&Config::getPath, &dl_info);
    if(ret == 0)
        throw Exception("Error reading dynamic library address in getPath()");
    const char* dlpath = dl_info.dli_fname;
#endif
    path = std::string(dlpath);

    // Remove lib name and lib folder
    int lastSlashPos = path.rfind("lib");
    path = path.substr(0, lastSlashPos-4);
    path = path + slash; // Make sure there is a slash at the end


    return path;
}

// Initialize statics
bool Config::mConfigurationLoaded = false;
std::string Config::mConfigFilename = "";
std::string Config::mBasePath = "";

// Default paths
std::string Config::mTestDataPath = getPath() + "../data/";
std::string Config::mKernelSourcePath = getPath() + "../source/FAST/";
std::string Config::mKernelBinaryPath = getPath() + "kernel_binaries/";
std::string Config::mDocumentationPath = getPath() + "../doc/";
std::string Config::mPipelinePath = getPath() + "../pipelines/";

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

std::string Config::getDocumentationPath() {
    loadConfiguration();
    return mDocumentationPath;
}

std::string Config::getPipelinePath() {
    loadConfiguration();
    return mPipelinePath;
}

void Config::setConfigFilename(std::string filename) {
    mConfigFilename = filename;
    loadConfiguration();
}

void Config::setBasePath(std::string path) {
    mBasePath = path;
    if(mBasePath[mBasePath.size()-1] != '/')
        mBasePath += "/";
    mTestDataPath = getPath() + "../data/";
    mKernelSourcePath = getPath() + "../source/FAST/";
    mKernelBinaryPath = getPath() + "kernel_binaries/";
    mDocumentationPath = getPath() + "../doc/";
    mPipelinePath = getPath() + "../pipelines/";
    loadConfiguration();
}

void Config::loadConfiguration() {
    if(mConfigurationLoaded)
        return;

    // Read and parse configuration file
    // It should reside in the build folder when compiling, and in the root folder when using release
    std::string filename;
    if(mConfigFilename == "") {
        filename = getPath() + "fast_configuration.txt";
    } else {
        filename = mConfigFilename;
    }
    //std::string filename = "/home/smistad/workspace/FAST/build_Debug/lib/fast_configuration.txt";
    std::ifstream file(filename);
    if(!file.is_open()) {
        Reporter::warning() << "Unable to open the configuration file " << filename << ". Using defaults instead." << Reporter::end;
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
        } else if(key == "DocumentationPath") {
            mDocumentationPath = value;
        } else if(key == "PipelinePath") {
            mPipelinePath = value;
        } else {
            throw Exception("Error parsing configuration file. Unrecognized key: " + key);
        }

        std::getline(file, line);
    }
    file.close();

    Reporter::info() << "Test data path: " << mTestDataPath << Reporter::end;
    Reporter::info() << "Kernel source path: " << mKernelSourcePath << Reporter::end;
    Reporter::info() << "Kernel binary path: " << mKernelBinaryPath << Reporter::end;
    Reporter::info() << "Documentation path: " << mDocumentationPath << Reporter::end;
    Reporter::info() << "Pipeline path: " << mPipelinePath << Reporter::end;

    mConfigurationLoaded = true;
}

}; // end namespace fast
