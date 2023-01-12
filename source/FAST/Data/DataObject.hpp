#pragma once

#include "FAST/Object.hpp"
#include "FAST/ExecutionDevice.hpp"
#include <map>
#include <set>
#include <condition_variable>

namespace fast {


/**
 * @defgroup data Data objects
 * FAST data objects are objects which can flow between ProcessObject objects.
 * They must inherit from @ref DataObject
*/

/**
 * @brief Abstract data object class.
 *
 * All data which should flow between process objects should derive from this class.
 */
class FAST_EXPORT  DataObject : public Object {
    public:
        DataObject();
        typedef std::shared_ptr<DataObject> pointer;
        void setMetadata(std::string name, std::string value);
        void setMetadata(std::map<std::string, std::string> metadata);
        std::string getMetadata(std::string name) const;
        std::map<std::string, std::string> getMetadata() const;
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
        void removeLastFrame(std::string streamer);
        void clearLastFrame();
        std::set<std::string> getLastFrame();
        void setFrameData(std::string name, std::string value);
        void setFrameData(std::map<std::string, std::string> frameData);
        std::string getFrameData(std::string name);
        template <class T>
        T getFrameData(std::string name);
        bool hasFrameData(std::string name) const;
        std::map<std::string, std::string> getFrameData();
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

        std::map<std::string, std::string> mMetadata;

        // Frame data
        // Similar to metadata, only this is transferred from input to output
        std::map<std::string, std::string> m_frameData;
        // Indicates whether this data object is the last frame in a stream, and if so, the name of the stream
        std::set<std::string> m_lastFrame;


};

template <>
int DataObject::getFrameData(std::string name);
template <>
float DataObject::getFrameData(std::string name);

}
