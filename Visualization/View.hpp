#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "SmartPointers.hpp"
#include "Renderer.hpp"
#include <vector>
#include <QtOpenGL/QGLWidget>

namespace fast {

class View : public ProcessObject {
    Q_OBJECT
    FAST_OBJECT(View)
    public:
        void addRenderer(Renderer::pointer renderer);
    private:
        View();
        std::vector<Renderer::pointer> mRenderers;
        void execute();
#if defined(CL_VERSION_1_2)
        cl::ImageGL mImageGL;
#else
        cl::Image2DGL mImageGL;
#endif
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

};

} // end namespace fast




#endif /* VIEW_HPP_ */
