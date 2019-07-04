#include "DummyObjects.hpp"

namespace fast {


void DummyStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;

}

void DummyStreamer::setTotalFrames(uint frames) {
    mFramesToGenerate = frames;

}

DummyStreamer::DummyStreamer() {
    createOutputPort<DummyDataObject>(0);
    mIsModified = true;
}

void DummyStreamer::execute() {
    std::cout << "DummyStreamer execuete" << std::endl;
    // If thread does not exist, create it, start it
    startStream();

    waitForFirstFrame();
}

void DummyStreamer::generateStream() {
    std::cout << "DummyStreamer thread started" << std::endl;
    for(int i = 0; i < mFramesToGenerate; ++i) {
        {
            std::lock_guard<std::mutex> lock(m_stopMutex);
            if(m_stop)
                break;
        }
        auto image = DummyDataObject::New();
        std::cout << "DummyDataObject " << i << " created in streamer" << std::endl;
        image->create(i);
        if(i == mFramesToGenerate-1)
            image->setLastFrame(getNameOfClass());
        addOutputData(0, image);
        {
            std::unique_lock<std::mutex> lock(mFramesGeneratedMutex);
            mFramesGenerated++;
        }
        frameAdded();
        std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
    }
}

bool DummyStreamer::hasReachedEnd() {
    std::unique_lock<std::mutex> lock(mFramesGeneratedMutex);
    return mFramesToGenerate == mFramesGenerated;
}

uint DummyStreamer::getFramesToGenerate() {
    std::unique_lock<std::mutex> lock(mFramesGeneratedMutex);
    return mFramesToGenerate;
}

DummyStreamer::~DummyStreamer() {
    stop();
}

DummyProcessObject2::DummyProcessObject2() {
    createInputPort<DummyDataObject>(0);
    createInputPort<DummyDataObject>(1);
    createOutputPort<DummyDataObject>(0);
}

void DummyProcessObject2::execute() {
    std::cout << "Executing PO 2" << std::endl;
    DummyDataObject::pointer dyanmicInput = getInputData<DummyDataObject>(0);
    DummyDataObject::pointer staticInput = getInputData<DummyDataObject>(1);
    mStaticID = staticInput->getID();

    DummyDataObject::pointer output = getOutputData<DummyDataObject>(0);
    output->create(dyanmicInput->getID());
}

int DummyProcessObject2::getStaticDataID() const {
    return mStaticID;
}


DummyImporter::DummyImporter() {
    createOutputPort<DummyDataObject>(0);
    mIsModified = true;
}

void DummyImporter::execute() {
    auto output = getOutputData<DummyDataObject>(0);
    std::cout << "Executing importer to create new object: " << output << std::endl;
    output->create(mExecuted);
    mExecuted++;
}

void DummyImporter::setModified() {
    mIsModified = true;
}


}