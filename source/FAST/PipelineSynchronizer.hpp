#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Synchronize multiple process objects
 * This PO takes in N input connections and creates N output connections.
 * It keeps the last frame of every connection, and every time a connection
 * has a new data frame, it send out the latest frame to all output connections.
 */
class FAST_EXPORT PipelineSynchronizer : public ProcessObject {
    FAST_PROCESS_OBJECT(PipelineSynchronizer)
    public:
        FAST_CONSTRUCTOR(PipelineSynchronizer);
        /**
         * Adds a new input connection
         * @param port
         * @return the input nr of the new connection
         */
        virtual uint addInputConnection(DataChannel::pointer port);
    protected:
        void execute() override;

        std::unordered_map<uint, std::shared_ptr<DataObject>> m_latestData;
};

}