#ifndef SIMPLEWINDOW_HPP_
#define SIMPLEWINDOW_HPP_

#include "Object.hpp"
#include "View.hpp"
#include "Renderer.hpp"
#include <QWidget>

namespace fast {

class SimpleWindow : public QWidget, public Object {
    Q_OBJECT
    FAST_OBJECT(SimpleWindow)
    public:
        void addRenderer(Renderer::pointer renderer);
        void setMaximumFramerate(unsigned char framerate);
    private:
        SimpleWindow();
        View::pointer mView;
        unsigned char mFramerate;
};

} // end namespace fast




#endif /* SIMPLEWINDOW_HPP_ */
