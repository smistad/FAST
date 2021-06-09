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
template <typename... Args>
class FAST_EXPORT DataStream : public Object {
    public:
        /**
         * Get next data object from the data stream
         *
         * @param outputPortID
         */
        std::tuple<Args...> getNextFrame();
        bool isDone();
        explicit DataStream(std::shared_ptr<ProcessObject> po);
    protected:
        std::shared_ptr<ProcessObject> m_processObject;
        std::vector<std::shared_ptr<DataChannel>> m_outputPorts;
        bool m_done = false;
};

// Two helper functions to deal with circular depedency
std::vector<std::shared_ptr<DataChannel>> FAST_EXPORT _DataStreamGetOutputPorts(std::shared_ptr<ProcessObject> po);
void FAST_EXPORT _DataStreamRunPipeline(std::shared_ptr<ProcessObject> po);

// Two helper functions to convert a list of data objects to a tuple with correct types
template<typename T>
std::tuple<T> _parseTuple(std::queue<std::shared_ptr<DataObject>> data) {
    return std::make_tuple(std::dynamic_pointer_cast<typename T::element_type>(data.front()));
}
template<typename T, typename Arg, typename... Args>
std::tuple<T, Arg, Args...> _parseTuple(std::queue<std::shared_ptr<DataObject>> data) {
    auto dataObject = data.front();
    data.pop();
    return std::tuple_cat(std::tuple<T>(std::dynamic_pointer_cast<typename T::element_type>(dataObject)), _parseTuple<Arg, Args...>(data));
}

template<typename... Args>
std::tuple<Args...> DataStream<Args...>::getNextFrame() {
    _DataStreamRunPipeline(m_processObject);
    std::queue<std::shared_ptr<DataObject>> data;
    for(auto outputPort : m_outputPorts) {
        auto dataObject = outputPort->getNextFrame();
        if(dataObject->isLastFrame())
            m_done = true;
        data.push(dataObject);
    }
    std::tuple<Args...> result = _parseTuple<Args...>(data);
    return result;
}


template<typename... Args>
DataStream<Args...>::DataStream(std::shared_ptr<ProcessObject> po) {
    m_processObject = po;
    m_outputPorts = _DataStreamGetOutputPorts(po);
}

template<typename... Args>
bool DataStream<Args...>::isDone() {
    return m_done;
}

#ifdef SWIG
%extend fast::DataStream {
%pythoncode %{
def __iter__(self):
    return self
def __next__(self):
    output_data = []
    for i in range(self.getNrOfOutputPorts()):
        output_dataself.getNextFrame(i)

%}
}
#endif
}