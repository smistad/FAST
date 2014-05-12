#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "SmartPointers.hpp"
#include "Renderer.hpp"
#include <vector>
#include <QtOpenGL/QGLWidget>
#include <QTimer>

namespace fast {

class View : public QGLWidget, public ProcessObject {
    Q_OBJECT
    FAST_OBJECT(View)
    public:
        void addRenderer(Renderer::pointer renderer);
        void keyPressEvent(QKeyEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
    private:
        View();
        std::vector<Renderer::pointer> mRenderers;
        void execute();
        QTimer* timer;

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

};

} // end namespace fast




#endif /* VIEW_HPP_ */
