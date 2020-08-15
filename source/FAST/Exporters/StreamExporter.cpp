#include "StreamExporter.hpp"
#include "FAST/Utility.hpp"

namespace fast {

void StreamExporter::setFilenameFormat(std::string format) {
    mFilenameFormat = format;
}

void StreamExporter::setExporter(std::shared_ptr<FileExporter> exporter) {
    mExporter = exporter;
}

StreamExporter::StreamExporter() {
    createInputPort<DataObject>(0);
    mFilenameFormat = "";
    mFrameCounter = 0;
    mFinished = false;
}

bool StreamExporter::isFinished() {
    return mFinished;
}

void StreamExporter::execute() {
    if(mFilenameFormat == "")
        throw Exception("Filename format must be given to StreamExporter");

    if(mFilenameFormat.find("#") == std::string::npos)
        throw Exception("Filename format must contain # which will be replaced by frame number");

    std::string filename = replace(mFilenameFormat, "#", std::to_string(mFrameCounter));
    reportInfo() << "Exporting " << filename << reportEnd();

    DataObject::pointer data = getInputData<DataObject>();
    mExporter->setInputData(data);
    mExporter->setFilename(filename);
    mExporter->update();
    mFrameCounter++;
    //if(dynamicData->hasReachedEnd())
    //    mFinished = true;
}

}