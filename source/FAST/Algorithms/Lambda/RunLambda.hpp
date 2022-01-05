#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief A list of FAST data input/output objects
 * Used in RunLambda
 * @sa RunLambda
 */
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

/**
 * @brief Run a C++ lambda in a FAST pipeline
 */
class FAST_EXPORT RunLambda : public ProcessObject {
	FAST_PROCESS_OBJECT(RunLambda)
	public:
        /**
         * @brief Create instance
         * @param lambda Lambda function which takes a single DataObject as input, and returns a DataList
         * @return instance
         */
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList(DataObject::pointer data)>, lambda,)
        /**
         * @brief Create instance
         * @param lambda Lambda function which takes no input, and returns a DataList
         * @return instance
         */
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList()>, lambda,)
        /**
         * @brief Create instance
         * @param lambda Lambda function which takes a DataList as input, and returns a DataList
         * @return instance
         */
        FAST_CONSTRUCTOR(RunLambda, std::function<DataList(DataList)>, lambda,)
        /**
         * @brief Specify whether the lambda function only should be run for the last frame
         * @param lastFrameOnly
         */
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