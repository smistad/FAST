#include "Interleave.hpp"

namespace fast {

Interleave::Interleave(int framerate) {
    for(int i = 0; i < 10; ++i)
        createInputPort(i, "", "", false);
    createOutputPort(0);
    setFramerate(framerate);
    createIntegerAttribute("framerate", "Framerate", "", framerate);
}

void Interleave::setFramerate(int framerate) {
    m_framerate = framerate;
    setModified(true);
}

void Interleave::execute() {
    startStream();

    waitForFirstFrame();
}

void Interleave::generateStream() {
    int i = 1;
    bool lastFrame = false;
    // Update will eventually block, therefore we need to call this in a separate thread
    auto parents = mInputConnections;
    mInputConnections.clear(); // Severe the connections
    bool firstTime = true;
    auto previousTime = std::chrono::high_resolution_clock::now();
    while(!lastFrame) {
        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        if(!firstTime) { // parent is execute the first time, thus drop it here
            for(int port = 0; port < parents.size(); ++port) {
                parents.at(port)->getProcessObject()->run(i);
            }
        }
        firstTime = false;
        bool lastFrame = false;
        try {
            if(m_framerate > 0) {
                std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
            }
            for(int port = 0; port < parents.size(); ++port) {
                auto data = parents[port]->getNextFrame<DataObject>();
                addOutputData(0, data);
                if(port < parents.size()-1 && m_framerate > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds((int)round(1000.0f/(float)m_framerate)));
                lastFrame = lastFrame || data->isLastFrame();
            }
            previousTime = std::chrono::high_resolution_clock::now();
        } catch(ThreadStopped &e) {
            break;
        }
        frameAdded();
        if(lastFrame)
            break;
        i++;
    }
}

Interleave::~Interleave() {
    stop();
}

void Interleave::loadAttributes() {
    setFramerate(getIntegerAttribute("framerate"));
}

InterleavePlayback::InterleavePlayback(std::shared_ptr<RandomAccessStreamer> sourceStreamer, int framerate) {
    for(int i = 0; i < 10; ++i)
        createInputPort(i, "", "", false);
    createOutputPort(0);
    if(!sourceStreamer)
        throw Exception("Must give valid streamer to InterleavePlayback");
    m_sourceStreamer = sourceStreamer;
    if(framerate <= 0 && m_sourceStreamer->getFramerate() > 0) { // If this has not gotten framerate, but streamer has; use that
        setFramerate(m_sourceStreamer->getFramerate());
    } else {
        if(framerate > 0) {
            setFramerate(framerate);
        } else {
            setFramerate(2);
        }
    }
    m_currentFrameIndex = 0;
    //m_sourceStreamer->setStreamingMode(StreamingMode::NewestFrameOnly);
    //setStreamingMode(StreamingMode::NewestFrameOnly);
}

int InterleavePlayback::getNrOfFrames() {
    return m_sourceStreamer->getNrOfFrames()*m_nrOfConnections;
}

void InterleavePlayback::execute() {
    startStream();

    waitForFirstFrame();
}

void InterleavePlayback::generateStream() {
    // FIXME going back and forward one frame at a time is not perfect.. It lags behind..
    m_sourceStreamer->setPause(true);
    m_sourceStreamer->setFramerate(-1);
    //m_sourceStreamer->setCurrentFrameIndex(0);
    int counter = 1;
    bool lastFrame = false;
    // Update will eventually block, therefore we need to call this in a separate thread
    auto parents = mInputConnections;
    m_nrOfConnections = parents.size();
    mInputConnections.clear(); // Severe the connections
    bool firstTime = true;
    auto previousTime = std::chrono::high_resolution_clock::now();
    while(!lastFrame) {
        bool pause = getPause();
        if(pause)
            waitForUnpause();
        pause = getPause();

        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        if(!firstTime) { // parent is execute the first time, thus drop it here
            if( parents.at(RandomAccessStreamer::getCurrentFrameIndex() % m_nrOfConnections)->getProcessObject() != m_sourceStreamer)
                parents.at(RandomAccessStreamer::getCurrentFrameIndex() % m_nrOfConnections)->getProcessObject()->run(counter);
        }
        //if(RandomAccessStreamer::getCurrentFrameIndex() % m_nrOfConnections == 0)
            counter++;
        firstTime = false;
        bool lastFrame = false;
        try {
            //std::cout << RandomAccessStreamer::getCurrentFrameIndex() << ": " << RandomAccessStreamer::getCurrentFrameIndex() % m_nrOfConnections << std::endl;
            auto data = parents[RandomAccessStreamer::getCurrentFrameIndex() % m_nrOfConnections]->getNextFrame<DataObject>();
            addOutputData(0, data);
            lastFrame = lastFrame || data->isLastFrame();
            previousTime = std::chrono::high_resolution_clock::now();
        } catch(ThreadStopped &e) {
            break;
        }
        if(!pause) {
            std::chrono::duration<float, std::milli> passedTime = std::chrono::high_resolution_clock::now() - previousTime;
            if(m_framerate > 0) {
                std::chrono::duration<int, std::milli> sleepFor(1000 / m_framerate - (int)passedTime.count());
                if(sleepFor.count() > 0)
                    std::this_thread::sleep_for(sleepFor);
            }
            previousTime = std::chrono::high_resolution_clock::now();
            int index = getCurrentFrameIndexAndUpdate();
            m_sourceStreamer->setCurrentFrameIndex(getCurrentFrameIndex()/m_nrOfConnections);
        }
        frameAdded();
        //if(lastFrame)
        //    break;
    }
}

void InterleavePlayback::setCurrentFrameIndex(int index) {
    RandomAccessStreamer::setCurrentFrameIndex(index);
    m_sourceStreamer->setCurrentFrameIndex(index/m_nrOfConnections);
}

int InterleavePlayback::getCurrentFrameIndex() {
    return RandomAccessStreamer::getCurrentFrameIndex();
}

void InterleavePlayback::setPause(bool pause) {
    RandomAccessStreamer::setPause(pause);
}

void InterleavePlayback::setFramerate(int framerate) {
    RandomAccessStreamer::setFramerate(framerate);
}

int InterleavePlayback::getFramerate() const {
    return m_framerate;
}

void InterleavePlayback::setLooping(bool loop) {
    RandomAccessStreamer::setLooping(loop);
}

}