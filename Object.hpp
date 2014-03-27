#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "SmartPointers.hpp"

namespace fast {

class Object {
    public:
        typedef SharedPointer<Object> pointer;
        virtual ~Object() {};
};

}



#endif /* OBJECT_HPP_ */
