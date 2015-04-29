#ifndef DATAOBJECT_HPP_
#define DATAOBJECT_HPP_

#include "SmartPointers.hpp"
#include "Object.hpp"
#include "ExecutionDevice.hpp"
#include <boost/unordered_map.hpp>
#include "BoundingBox.hpp"
#include "SceneGraph.hpp"
#include "Streamer.hpp"

namespace fast {

class DataObject : public Object {
    public:
        DataObject();
        typedef SharedPointer<DataObject> pointer;
        unsigned long getTimestamp();
        void updateModifiedTimestamp();
        void retain(ExecutionDevice::pointer device);
        void release(ExecutionDevice::pointer device);
        virtual BoundingBox getBoundingBox() const;
        virtual BoundingBox getTransformedBoundingBox() const;
        virtual ~DataObject() { };
        bool isDynamicData();
        SceneGraphNode::pointer getSceneGraphNode() const;
        void setStreamer(Streamer::pointer streamer);
        Streamer::pointer getStreamer();
        virtual std::string getNameOfClass() const = 0;
    protected:
        virtual void free(ExecutionDevice::pointer device) = 0;
        virtual void freeAll() = 0;
        BoundingBox mBoundingBox;
        bool mIsDynamicData;
    private:
        boost::unordered_map<WeakPointer<ExecutionDevice>, unsigned int> mReferenceCount;

        // This is only used for dynamic data, it is defined here for to make the convienice function getStaticOutput/InputData to work
        WeakPointer<Streamer> mStreamer;

        // Timestamp is set to 0 when data object is constructed
        unsigned long mTimestampModified;

        SceneGraphNode::pointer mSceneGraphNode;
};

}




#endif /* DATAOBJECT_HPP_ */
