#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "SmartPointers.hpp"
#include "ProcessObject.hpp"

namespace fast {

class Renderer : public ProcessObject {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw() = 0;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
    protected:
        Renderer();
        void setOpenGLContext(unsigned long* OpenGLContext);

        // Level and window intensities
        float mWindow;
        float mLevel;
};

}



#endif /* RENDERER_HPP_ */
