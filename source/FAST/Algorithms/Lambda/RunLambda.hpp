#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class DataList {
    public:
        DataList(std::map<int, DataObject::pointer> list) {
            m_data = list;
        }
        DataList(DataObject::pointer data) {
            m_data[0] = data;
        }
        DataList() {
        }
        template <class DataType>
        std::shared_ptr<DataType> getInputData(int portID = 0) {
            if(m_data.count(portID) == 0)
                throw Exception("No input data present at port " + std::to_string(portID));
            return std::dynamic_pointer_cast<DataType>(m_data[portID]);
        }
        std::map<int, DataObject::pointer> getAllData() {
            return m_data;
        }
    private:
    std::map<int, DataObject::pointer> m_data;
};

class FAST_EXPORT RunLambda : public ProcessObject {
	FAST_PROCESS_OBJECT(RunLambda)
	public:
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList(DataObject::pointer data)>, lambda,)
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList()>, lambda,)
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList(DataList)>, lambda,)
        void setRunOnLastFrameOnly(bool lastFrameOnly);
	protected:
		RunLambda();
		void execute();

		std::function<DataList()> m_lambdaNoInput;
		std::function<DataList(DataObject::pointer)> m_lambdaWithSingleInput;
        std::function<DataList(DataList)> m_lambdaWithMultipleInput;
		bool m_runOnLastFrameOnly = false;
};

}