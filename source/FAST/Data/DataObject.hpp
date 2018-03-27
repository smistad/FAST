#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Object.hpp"
#include "FAST/ExecutionDevice.hpp"
#include <unordered_map>
#include <condition_variable>

namespace fast {

class FAST_EXPORT  DataObject : public Object {
    public:
        DataObject();
        typedef SharedPointer<DataObject> pointer;
        void setMetadata(std::string name, std::string value);
        void setMetadata(std::unordered_map<std::string, std::string> metadata);
        std::string getMetadata(std::string name) const;
        std::unordered_map<std::string, std::string> getMetadata() const;
        unsigned long getTimestamp() const;
        void updateModifiedTimestamp();
        uint64_t getTimestep() const;
        void setTimestep(uint64_t);
        void retain(ExecutionDevice::pointer device);
        void release(ExecutionDevice::pointer device);
        virtual ~DataObject() { };
        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "DataObject";
        };
        unsigned long getCreationTimestamp() const;
        void setCreationTimestamp(unsigned long timestamp);
    protected:
        virtual void free(ExecutionDevice::pointer device) = 0;
        virtual void freeAll() = 0;

        void accessFinished();
        void blockIfBeingWrittenTo();
        void blockIfBeingAccessed();

        std::mutex mDataIsBeingWrittenToMutex;
        std::condition_variable mDataIsBeingWrittenToCondition;
        bool mDataIsBeingWrittenTo;

        std::mutex mDataIsBeingAccessedMutex;
        std::condition_variable mDataIsBeingAccessedCondition;
        bool mDataIsBeingAccessed;
    private:
        std::unordered_map<WeakPointer<ExecutionDevice>, unsigned int> mReferenceCount;

        // Timestamp is set to 0 when data object is constructed
        unsigned long mTimestampModified;

        // Timestamp is set to 0 when data object is constructed
        unsigned long mTimestampCreated;

        uint64_t mTimestep;

        std::unordered_map<std::string, std::string> mMetadata;

};

}




#endif /* DATAOBJECT_HPP_ */
