#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

namespace fast {

class Config {
public:
    static std::string getTestDataPath();
    static std::string getKernelSourcePath();
    static std::string getKernelBinaryPath();
    static std::string getDocumentationPath();
    static std::string getPipelinePath();
    static void setConfigFilename(std::string filename);
    static void setBasePath(std::string path);
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
