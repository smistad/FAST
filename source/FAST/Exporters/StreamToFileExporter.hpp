#pragma once

#include <FAST/ProcessObject.hpp>
#include <chrono>

namespace fast {

/**
 * @brief Write a stream of Mesh or Image data as a sequence of files.
 *
 * <h3>Input ports</h3>
 * - 0: Image or Mesh
 *
 * @todo Supports more data types and formats
 * @ingroup exporters
 */
class FAST_EXPORT StreamToFileExporter : public ProcessObject {
    FAST_PROCESS_OBJECT(StreamToFileExporter)
    public:
        /**
         * @brief Create instance
         * @param path Path to folder to store recordings/streams
         * @param recordingFolderName Name of subfolder to store recordings/files in.
         *      If not specified a folder with date and time will be used
         * @return instance
         */
        FAST_CONSTRUCTOR(StreamToFileExporter,
             std::string, path,,
             std::string, recordingFolderName, = ""
        );
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