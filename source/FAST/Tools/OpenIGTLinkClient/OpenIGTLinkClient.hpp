#ifndef FAST_OPENIGTLINKCLIENT_HPP_
#define FAST_OPENIGTLINKCLIENT_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class OpenIGTLinkClient : public ProcessObject {
    FAST_OBJECT(OpenIGTLinkClient)
    public:
        bool toggleRecord(std::string storageDir);
        bool isRecording();
        std::string getRecordingName();
        uint getFramesStored();
    private:
        OpenIGTLinkClient();
        void execute();

        bool mRecording;
        uint mRecordFrameNr;
        std::string mRecordStoragePath;
        std::string mRecordingName;
};

}

#endif
