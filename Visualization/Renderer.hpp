#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "SmartPointers.hpp"
#include "ProcessObject.hpp"

namespace fast {

class Renderer : public ProcessObject {
    public:
        typedef SharedPointer<Renderer> pointer;
        void draw();

};

}



#endif /* RENDERER_HPP_ */
