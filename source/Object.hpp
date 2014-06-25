#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "SmartPointers.hpp"

namespace fast {

class Object {
    public:
        typedef SharedPointer<Object> pointer;
        virtual ~Object() {};
    protected:
        void setOpenGLContext(unsigned long* OpenGLContext);

};

}



#endif /* OBJECT_HPP_ */
