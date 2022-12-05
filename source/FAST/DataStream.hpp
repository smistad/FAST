#pragma once

#include <FAST/Object.hpp>
#include <FAST/DataChannels/DataChannel.hpp>
#include <map>
#include <vector>
#include <queue>

namespace fast {

class ProcessObject;
class DataChannel;
class DataObject;

/**
 * @brief Object for iterating through a data stream coming from a pipeline
 */
class FAST_EXPORT DataStream : public Object {
    public:
        /**
         * Get next data object from the data stream
         *
         * @param outputPortID
         */
        template <typename DataType>
        std::shared_ptr<DataType> getNextFrame(uint portID = 0);
        /**
         * True if stream is done.
         * This happens if any of the data objects are marked as being the "last frame".
         *
         * @return
         */
        bool isDone();
        /**
         * Create a data stream of multiple process objects
         * @param processObjects
         */
        explicit DataStream(std::vector<std::shared_ptr<ProcessObject>> processObjects);
        /**
         * Creat a data stream of a single process objects
         * @param processObject
         */
        explicit DataStream(std::shared_ptr<ProcessObject> processObject);
    protected:
        std::vector<std::shared_ptr<ProcessObject>> m_processObjects;
        std::vector<std::shared_ptr<DataChannel>> m_outputPorts;
        std::map<uint, std::shared_ptr<DataObject>> m_nextDataObjects;
        bool m_done = false;
        uint m_executeToken = 0;
        std::shared_ptr<DataObject> getNextDataObject(uint portID);
};

template<typename DataType>
std::shared_ptr<DataType> DataStream::getNextFrame(uint portID) {
    auto dataObject = getNextDataObject(portID);
    auto converted = std::dynamic_pointer_cast<DataType>(dataObject);
    if(converted == nullptr)
        throw Exception("Failed to cast data object to " + DataType::getStaticNameOfClass() + " in DataStream::getNextFrame(). Object is  " + dataObject->getNameOfClass());
    return converted;
}

}