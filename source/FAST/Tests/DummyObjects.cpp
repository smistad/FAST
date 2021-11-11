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
        if(i == mFramesToGenerate-1) {
            std::cout << "Last frame!" << std::endl;
            image->setLastFrame(getNameOfClass());
        }
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

    auto output = DummyDataObject::New();
    output->create(dyanmicInput->getID());
    addOutputData(0, output);
}

int DummyProcessObject2::getStaticDataID() const {
    return mStaticID;
}

DummyProcessObject3::DummyProcessObject3() {
    createInputPort<DummyDataObject>(0);
    createOutputPort<DummyDataObject>(0);
    createOutputPort<DummyDataObject>(1);
}

void DummyProcessObject3::execute() {
    std::cout << "Executing PO 3" << std::endl;
    DummyDataObject::pointer dyanmicInput = getInputData<DummyDataObject>(0);

    auto output = DummyDataObject::New();
    output->create(dyanmicInput->getID());
    addOutputData(0, output);
    auto output1 = DummyDataObject::New();
    output1->create(dyanmicInput->getID());
    addOutputData(1, output1);
}

DummyImporter::DummyImporter() {
    createOutputPort<DummyDataObject>(0);
    mIsModified = true;
}

void DummyImporter::execute() {
    auto output = DummyDataObject::New();
    std::cout << "Executing importer to create new object: " << output << std::endl;
    output->create(mExecuted);
    mExecuted++;
    addOutputData(0, output);
}

void DummyImporter::setModified() {
    mIsModified = true;
}


}