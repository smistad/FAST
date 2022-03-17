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
}