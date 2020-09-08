#pragma once

#include <string>
#include <FASTExport.hpp>
#include <FAST/DataChannels/DataChannel.hpp>

namespace fast {

void FAST_EXPORT downloadTestDataIfNotExists(std::string overrideDestination = "", bool force = false);

class FAST_EXPORT Config {
public:
    static std::string getTestDataPath();
    static std::string getKernelSourcePath();
    static std::string getKernelBinaryPath();
    static std::string getDocumentationPath();
    static std::string getPipelinePath();
    static std::string getLibraryPath();
    static std::string getQtPluginsPath();
    static StreamingMode getStreamingMode();
    static void setStreamingMode(StreamingMode mode);
    static void setTestDataPath(std::string path);
    static void setKernelSourcePath(std::string path);
    static void setKernelBinaryPath(std::string path);
    static void setDocumentationPath(std::string path);
    static void setPipelinePath(std::string path);
    static void setConfigFilename(std::string filename);
    static void setBasePath(std::string path);
protected:
    static void loadConfiguration();
    static std::string getPath();
};

}
