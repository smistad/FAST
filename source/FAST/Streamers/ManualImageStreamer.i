%include "FAST/ProcessObject.i"
%include "std_string.i"
%shared_ptr(fast::ImageFileStreamer);

namespace fast {

enum StreamingMode { STREAMING_MODE_NEWEST_FRAME_ONLY, STREAMING_MODE_STORE_ALL_FRAMES, STREAMING_MODE_PROCESS_ALL_FRAMES };

class ImageFileStreamer : public Streamer, public ProcessObject {
    public:
    	static std::shared_ptr<ImageFileStreamer> New();
        void setFilenameFormat(std::string str);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void setZeroFilling(uint digits);
        void setStreamingMode(StreamingMode mode);
        void setMaximumNumberOfFrames(uint nrOfFrames);
        void setTimestampFilename(std::string filepath);
        void enableLooping();
        void disableLooping();
};

%template(ImageFileStreamerPtr) std::shared_ptr<ImageFileStreamer>;

}