#pragma once

#include <FAST/Object.hpp>
#include <map>
#include <vector>

namespace fast {

class ProcessObject;
class DataChannel;
class DataObject;

/**
 * @brief Object for iterating through a data stream coming from a pipeline
 */
class DataStream : public Object {
    public:
        DataStream(std::shared_ptr<ProcessObject> po);
        /**
         * Get next data object from the data stream
         *
         * @tparam DataType
         * @param outputPortID
         */
        template <class DataType>
        void getNextFrame(uint outputPortID = 0);
    protected:
        std::shared_ptr<ProcessObject> m_processObject;
        std::vector<std::shared_ptr<DataChannel>> m_outputPorts;
        std::map<uint, std::shared_ptr<DataObject>> m_lastDataObject;
};

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