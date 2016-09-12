%include "FAST/ProcessObject.i"
%include "FAST/Data/Image.i"
%include "std_string.i"
%shared_ptr(fast::ManualImageStreamer);

namespace fast {


class ManualImageStreamer : public Streamer, public ProcessObject {
    public:
    	static SharedPointer<ManualImageStreamer> New();
    	void addImage(SharedPointer<Image> image);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void enableLooping();
        void disableLooping();
        void setSleepTime(uint milliseconds);
};

%template(ManualImageStreamerPtr) SharedPointer<ManualImageStreamer>;

}