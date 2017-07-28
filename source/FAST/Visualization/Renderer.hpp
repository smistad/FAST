#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include <mutex>


namespace fast {

class View;
class BoundingBox;

class FAST_EXPORT  Renderer : public ProcessObject {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw() = 0;
        virtual void postDraw();
        virtual void addInputConnection(DataPort::pointer port);
        virtual BoundingBox getBoundingBox();
        virtual void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) {};
        virtual void stopPipeline();
    protected:
        Renderer();
        void execute() override;


        // Locking mechanisms to ensure thread safe synchronized rendering
        bool mHasRendered = true;
        std::condition_variable_any mRenderedCV;
        std::mutex mMutex;

        /**
         * This holds the current data to render for each input connection
         */
        std::unordered_map<uint, SpatialDataObject::pointer> mDataToRender;

        /**
         * This will lock the renderer mutex. Used by the compute thread.
         */
        void lock();
        /**
         * This will unlock the renderer mutex. Used by the compute thread.
         */
        void unlock();
        friend class View;
};

}



#endif /* RENDERER_HPP_ */
