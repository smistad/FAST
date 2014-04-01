#ifndef IMAGERENDERER_HPP_
#define IMAGERENDERER_HPP_

#include "Renderer.hpp"

namespace fast {

class ImageRenderer : public Renderer {
    FAST_OBJECT(ImageRenderer)
    private:
        void execute();
        void draw();

};

}




#endif /* IMAGERENDERER_HPP_ */
