#pragma once

#include <FAST/ProcessObject.hpp>
#include <chrono>

namespace fast {

class FAST_EXPORT StreamToFileExporter : public ProcessObject {
    FAST_OBJECT(StreamToFileExporter)
    public:
        void setPath(std::string path);
        void setRecordingFolderName(std::string folder);
        void setFrameFilename(std::string name);
        void setEnabled(bool enabled);
        void setFrameLimit(uint64_t limit);
        uint64_t getFrameCounter() const;
        std::string getCurrentDestinationFolder() const;
        float getRecordingDuration() const;
        void reset();
        bool isEnabled();
    private:
        StreamToFileExporter();
        void execute() override;

        std::string m_path = "";
        std::string m_folder;
        std::string m_filename = "frame";
        std::string m_currentFolder;
        uint64_t m_frameCounter = 0;
        uint64_t m_frameLimit = 10000;
        std::chrono::high_resolution_clock::time_point m_recordingStartTime;
        bool m_enabled = true;
        bool m_hasStarted = false;
};

}