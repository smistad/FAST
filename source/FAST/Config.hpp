#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>
#include "FASTExport.hpp"

namespace fast {

namespace Config {
    FAST_EXPORT std::string getTestDataPath();
    FAST_EXPORT std::string getKernelSourcePath();
    FAST_EXPORT std::string getKernelBinaryPath();
    FAST_EXPORT std::string getDocumentationPath();
    FAST_EXPORT std::string getPipelinePath();
	FAST_EXPORT void setTestDataPath(std::string path);
	FAST_EXPORT void setKernelSourcePath(std::string path);
	FAST_EXPORT void setKernelBinaryPath(std::string path);
	FAST_EXPORT void setDocumentationPath(std::string path);
	FAST_EXPORT void setPipelinePath(std::string path);
    FAST_EXPORT void setConfigFilename(std::string filename);
    FAST_EXPORT void setBasePath(std::string path);
	FAST_EXPORT void loadConfiguration();
}

}

#endif
