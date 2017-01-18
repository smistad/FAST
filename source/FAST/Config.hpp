#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

namespace fast {

class Config {
public:
    static std::string getTestDataPath();
    static std::string getKernelSourcePath();
    static std::string getKernelBinaryPath();
private:
    static void loadConfiguration();
    static bool mConfigurationLoaded;
    static std::string mTestDataPath;
    static std::string mKernelSourcePath;
    static std::string mKernelBinaryPath;
};

}

#endif
