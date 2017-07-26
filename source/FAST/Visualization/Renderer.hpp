#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/ProcessObject.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include <mutex>


namespace fast {

class View;
class BoundingBox;

class FAST_EXPORT  Renderer : public ProcessObject {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw() = 0;
        virtual void addInputConnection(DataPort::pointer port);
        virtual BoundingBox getBoundingBox() = 0;

        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
        virtual void draw2D(
                cl::Buffer PBO,
                uint width,
                uint height,
                Affine3f pixelToViewportTransform,
                float PBOspacing,
                Vector2f translation
        ) {};
    protected:
        Renderer();

        // Level and window intensities
        float mWindow;
        float mLevel;

        std::mutex mMutex;
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
