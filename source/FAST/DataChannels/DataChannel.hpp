#pragma once

#include <FAST/Data/DataObject.hpp>
#include <FAST/Data/DataTypes.hpp>

namespace fast {

class ProcessObject;

class FAST_EXPORT DataChannel : public Object {
    public:
        typedef std::shared_ptr<DataChannel> pointer;

        /**
         * Add frame to the data channel. This call may block
         * if the buffer is full.
         */
        virtual void addFrame(DataObject::pointer data) = 0;

        /**
         * Get next frame in the data channel. 
         * It will block until the frame is available.
         */
        template <class T = DataObject>
        std::shared_ptr<T> getNextFrame();

        /**
         * @return the number of frames stored in this DataChannel
         */
        virtual int getSize() = 0;

        /**
         * Set the maximum nr of frames that can be stored in this data channel
         */
        virtual void setMaximumNumberOfFrames(uint frames) = 0;

        virtual int getMaximumNumberOfFrames() const = 0;

        /**
         * @brief This will unblock if this DataChannel is currently blocking. Used to stop a pipeline.
         * @param Error message to supply.
         */
        virtual void stop(std::string errorMessage = "") = 0;

        // TODO consider removing, it is equal to getSize() > 0 atm
        virtual bool hasCurrentData() = 0;

        /**
         * @brief Get current frame, throws if current frame is not available.
         */
        virtual DataObject::pointer getFrame() = 0;

        std::shared_ptr<ProcessObject> getProcessObject() const;
        void setProcessObject(std::shared_ptr<ProcessObject> po);
    protected:
        bool m_stop;
        std::string m_errorMessage = "";
        std::mutex m_mutex;
        std::shared_ptr<ProcessObject> m_processObject;

        virtual DataObject::pointer getNextDataFrame() = 0;
        DataChannel();
};

// Template specialization when T = DataObject
template <>
FAST_EXPORT std::shared_ptr<DataObject> DataChannel::getNextFrame<DataObject>();

template <class T>
std::shared_ptr<T> DataChannel::getNextFrame() {
    auto data = getNextDataFrame();
    auto convertedData = std::dynamic_pointer_cast<T>(data);
    // Check if the conversion went ok
    if(!convertedData)
        throw BadCastException(data->getNameOfClass(), T::getStaticNameOfClass());
    return convertedData;
}

}
