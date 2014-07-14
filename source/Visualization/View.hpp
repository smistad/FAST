#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "SmartPointers.hpp"
#include "Renderer.hpp"
#include <vector>
#include <QtOpenGL/QGLWidget>
#include <QTimer>

namespace fast {

class View : public QGLWidget, public ProcessObject {
    FAST_OBJECT(View)
    Q_OBJECT
    public:
        void addRenderer(Renderer::pointer renderer);
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
        void resizeEvent(QResizeEvent* event);
        void wheelEvent(QWheelEvent* event);
        void setMaximumFramerate(unsigned int framerate);
    private:
        View();
        std::vector<Renderer::pointer> mRenderers;
        void execute();
        QTimer* timer;
        unsigned int mFramerate;
        Float3 cameraPosition;
        Float3 originalCameraPosition;
        Float3 rotationPoint;
        Float2 rotation;
        float zNear, zFar;
        float fieldOfViewX, fieldOfViewY;
        float aspect;
        bool isIn2DMode;

        bool mLeftMouseButtonIsPressed;

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

};

} // end namespace fast




#endif /* VIEW_HPP_ */
