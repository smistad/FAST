#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_


#include "FAST/Object.hpp"
#include "FAST/ExecutionDevice.hpp"
#include <unordered_map>
#include <unordered_set>
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
        void deleteMetadata(std::string name);
        uint64_t getTimestamp() const;
        void updateModifiedTimestamp();
        virtual ~DataObject() { };
        virtual std::string getNameOfClass() const = 0;
        static std::string getStaticNameOfClass() {
            return "DataObject";
        };
        uint64_t getCreationTimestamp() const;
        void setCreationTimestamp(uint64_t timestamp);

        void setLastFrame(std::string streamer);
        bool isLastFrame();
        bool isLastFrame(std::string streamer);
        std::unordered_set<std::string> getLastFrame();
        void setFrameData(std::string name, std::string value);
        std::string getFrameData(std::string name);
        std::unordered_map<std::string, std::string> getFrameData();
        void accessFinished();
    protected:
        virtual void free(ExecutionDevice::pointer device) = 0;
        virtual void freeAll() = 0;

        void blockIfBeingWrittenTo();
        void blockIfBeingAccessed();

        std::mutex mDataIsBeingWrittenToMutex;
        std::condition_variable mDataIsBeingWrittenToCondition;
        bool mDataIsBeingWrittenTo;

        std::mutex mDataIsBeingAccessedMutex;
        std::condition_variable mDataIsBeingAccessedCondition;
        bool mDataIsBeingAccessed;
    private:

        // Timestamp is set to 0 when data object is constructed
        uint64_t mTimestampModified;

        // Timestamp is set to 0 when data object is constructed
        uint64_t mTimestampCreated;

        std::unordered_map<std::string, std::string> mMetadata;

        // Frame data
        // Similar to metadata, only this is transferred from input to output
        std::unordered_map<std::string, std::string> m_frameData;
        // Indicates whether this data object is the last frame in a stream, and if so, the name of the stream
        std::unordered_set<std::string> m_lastFrame;


};

}




#endif /* DATAOBJECT_HPP_ */
