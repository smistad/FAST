#ifndef STREAM_EXPORTER_HPP_
#define STREAM_EXPORTER_HPP_

#include "FAST/Exporters/FileExporter.hpp"

namespace fast {

class FAST_EXPORT  StreamExporter : public ProcessObject {
    FAST_OBJECT(StreamExporter)
    public:
        void setFilenameFormat(std::string format);
        void setExporter(std::shared_ptr<FileExporter> exporter);
        bool isFinished();
    private:
        StreamExporter();
        void execute();

        std::string mFilenameFormat;
        std::shared_ptr<FileExporter> mExporter;
        int mFrameCounter;
        bool mFinished;
};

}

#endif