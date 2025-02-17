#pragma once

#include <FAST/ProcessObject.hpp>
#include <FAST/Streamers/Streamer.hpp>

namespace fast {

/**
 * @brief Synchronize multiple process objects
 * This PO takes in N input connections and creates N output connections.
 * It keeps the last frame of every connection, and every time a connection
 * has a new data frame, it send out the latest frame to all output connections.
 */
class FAST_EXPORT FramerateSynchronizer : public ProcessObject {
    FAST_PROCESS_OBJECT(FramerateSynchronizer)
    public:
        FAST_CONSTRUCTOR(FramerateSynchronizer, int, priorityPort, = -1);
        ~FramerateSynchronizer();
        void setInputConnection(uint portID, DataChannel::pointer port) override;
    protected:
        void execute() override;

        std::unordered_map<uint, std::shared_ptr<DataObject>> m_latestData;

        int m_priorityPort = -1;
        std::map<uint, DataChannel::pointer> m_parents;
        std::mutex m_latestDataMutex;
        std::condition_variable m_dataCV;
        bool m_newData = false;
        std::vector<std::thread*> m_threads;
};

}