#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>
#include "FASTExport.hpp"

namespace fast {

/**
 * A static class for handling loading of the fast configuration file
 * which contains several paths to kernel source files, test data etc.
 * which are needed at runtime.
 */
class Config {
public:
    static FAST_EXPORT std::string getTestDataPath();
    static FAST_EXPORT std::string getKernelSourcePath();
    static FAST_EXPORT std::string getKernelBinaryPath();
    static FAST_EXPORT std::string getDocumentationPath();
    static FAST_EXPORT std::string getPipelinePath();
    static FAST_EXPORT void setConfigFilename(std::string filename);
    static FAST_EXPORT void setBasePath(std::string path);
private:
    static void loadConfiguration();
    static std::string getPath();
    static bool mConfigurationLoaded;
    static std::string mTestDataPath;
    static std::string mKernelSourcePath;
    static std::string mKernelBinaryPath;
    static std::string mDocumentationPath;
    static std::string mPipelinePath;
    static std::string mConfigFilename;
    static std::string mBasePath;
};

}

#endif
