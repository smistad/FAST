#include <FAST/Utility.hpp>
#include <FAST/Exporters/MetaImageExporter.hpp>
#include "OpenIGTLinkClient.hpp"
#include "FAST/Data/Image.hpp"
#include <QDir>

namespace fast {

OpenIGTLinkClient::OpenIGTLinkClient() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

    mRecording = false;
}

inline std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H%M%S", &tstruct);

    return buf;
}

void OpenIGTLinkClient::execute() {
    Image::pointer image = getStaticInputData<Image>(0);

    if(mRecording) {
        // Save frame to disk
        MetaImageExporter::pointer exporter = MetaImageExporter::New();
        exporter->setFilename(mRecordStoragePath + "US-2D_" + std::to_string(mRecordFrameNr) + ".mhd");
        exporter->setInputData(image);
        //exporter->enableCompression();
        exporter->update();
        ++mRecordFrameNr;

        // Failsafe: if someone forgets to turn of record, turn it off automatically after about 15 minutes
        if(mRecordFrameNr > 10800) { // 15 minutes with 12 fps
            mRecording = false;
        }
    }

    setStaticOutputData<Image>(0, image);
}

bool OpenIGTLinkClient::toggleRecord(std::string storageDir) {
    mRecording = !mRecording;
    if(mRecording) {
        mRecordFrameNr = 0;
        mRecordingName = currentDateTime();
        mRecordStoragePath = (QString(storageDir.c_str()) + QDir::separator() + QString(mRecordingName.c_str()) + QDir::separator()).toStdString();
        createDirectories(mRecordStoragePath);
    }
    return mRecording;
}

std::string OpenIGTLinkClient::getRecordingName() {
    return mRecordingName;
}

bool OpenIGTLinkClient::isRecording() {
    return mRecording;
}

uint OpenIGTLinkClient::getFramesStored() {
    return mRecordFrameNr;
}

}