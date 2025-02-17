#include "FramerateSynchronizer.hpp"

namespace fast {

FramerateSynchronizer::FramerateSynchronizer(int priorityPort) {
    m_priorityPort = priorityPort;
}

/*
uint FramerateSynchronizer::addInputConnection(DataChannel::pointer channel) {
    uint nr = getNrOfInputConnections();
    createInputPort<DataObject>(nr, false);
    createOutputPort<DataObject>(nr);
    setInputConnection(nr, channel);
    return nr;
}*/

void FramerateSynchronizer::execute() {
    if(!mInputConnections.empty()) {
        // First time
        m_parents = mInputConnections;
        mInputConnections.clear(); // Severe the connections

        // Start one thread per input
        for(const auto& parent : m_parents) {
            uint port = parent.first;
            DataChannel::pointer channel = parent.second;
            //channel->setMaximumNumberOfFrames(1);
            auto thread = new std::thread([port, channel, this]() {
                //std::cout << "Started thread" << std::endl;
                try {
                    while(true) {
                        channel->getProcessObject()->run();
                        DataObject::pointer data = channel->getNextFrame();
                        //std::cout << "Adding data to port " << port << std::endl;
                        {
                            std::lock_guard<std::mutex> lock(m_latestDataMutex);
                            m_latestData[port] = data;
                            m_newData = true;
                        }
                        m_dataCV.notify_one();
                        if(data->isLastFrame())
                            break;
                    }
                } catch(ThreadStopped &e) {
                }
                //std::cout << "THREAD STOPPED" << std::endl;
            });
            m_threads.push_back(thread);
        }
    }

    std::unique_lock<std::mutex> lock(m_latestDataMutex);
    if(m_priorityPort >= 0) {
        // Wait until priority port has new data
        while(m_latestData.size() != m_parents.size() || m_latestData.count(m_priorityPort) == 0) {
            //std::cout << "Waiting for new data on priority port" << std::endl;
            m_dataCV.wait(lock);
        }
    } else {
        while(!m_newData || m_latestData.size() != m_parents.size()) {
            //std::cout << "Waiting for new data" << std::endl;
            m_dataCV.wait(lock);
        }
    }
    auto dataToAdd = m_latestData;
    m_newData = false;
    lock.unlock();
    //std::cout << "Got data on priority port" << std::endl;
    for(const auto& parent : m_parents) {
        //std::cout << "adding output data to " << parent.first << ": " << dataToAdd[parent.first] << std::endl;
        addOutputData(parent.first, dataToAdd[parent.first]);
    }
    if(m_priorityPort >= 0) {
        lock.lock();
        m_latestData.erase(m_priorityPort);
        lock.unlock();
    }
    //std::cout << "Done adding data." << std::endl;
    mIsModified = true; // Always rerun
}

FramerateSynchronizer::~FramerateSynchronizer() {
    //std::cout << "In destructor" << std::endl;
    for(auto parent : m_parents) {
        parent.second->stop();
    }
    for(auto thread : m_threads) {
        thread->join();
        delete thread;
    }
}

void FramerateSynchronizer::setInputConnection(uint portID, DataChannel::pointer port) {
    createInputPort<DataObject>(portID, false);
    createOutputPort<DataObject>(portID);
    ProcessObject::setInputConnection(portID, port);
}

}