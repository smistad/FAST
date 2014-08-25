#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "SmartPointers.hpp"
#include "ProcessObject.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>


namespace fast {

class View;

class Renderer : public ProcessObject {
    public:
        typedef SharedPointer<Renderer> pointer;
        virtual void draw() = 0;
        virtual BoundingBox getBoundingBox() = 0;
        void setIntensityLevel(float level);
        float getIntensityLevel();
        void setIntensityWindow(float window);
        float getIntensityWindow();
        virtual void keyPressEvent(QKeyEvent* event) {};
        virtual void mouseMoveEvent(QMouseEvent* event, View* view) {};
        virtual void mousePressEvent(QMouseEvent* event) {};
        virtual void mouseReleaseEvent(QMouseEvent* event) {};
        virtual void resizeEvent(QResizeEvent* event) {};
    protected:
        Renderer();

        // Level and window intensities
        float mWindow;
        float mLevel;
};

}



#endif /* RENDERER_HPP_ */
