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
    if(!mRunning) {
        mThread = new std::thread(std::bind(&DummyStreamer::produce, this));
        mRunning = true;
    }

    // Block until first frame as been added
    std::unique_lock<std::mutex> lock(mFramesGeneratedMutex);
    mFirstFrameConditionVariable.wait(lock, [this]{ return mFramesGenerated > 0;});
}

void DummyStreamer::produce() {
    std::cout << "DummyStreamer thread started" << std::endl;
    for(int i = 0; i < mFramesToGenerate; ++i) {
        DummyDataObject::pointer image = DummyDataObject::New();
        std::cout << "DummyDataObject " << i << " created in streamer" << std::endl;
        image->create(i);
        addOutputData(0, image);
        {
            std::unique_lock<std::mutex> lock(mFramesGeneratedMutex);
            mFramesGenerated++;
        }
        if(i == 0)
            mFirstFrameConditionVariable.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
    }
    mRunning = false;
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

    if(mThread != nullptr) {
        if(mThread->get_id() != std::this_thread::get_id()) { // avoid deadlock
            mThread->join();
            delete mThread;
            mThread = nullptr;
        }
    }
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
    std::cout << "Executing importer" << std::endl;
    DummyDataObject::pointer output = getOutputData<DummyDataObject>(0);
    output->create(mExecuted);
    mExecuted++;
}

void DummyImporter::setModified() {
    mIsModified = true;
}


}