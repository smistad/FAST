#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "FAST/SmartPointers.hpp"

namespace fast {

class Object {
    public:
        typedef SharedPointer<Object> pointer;
        virtual ~Object() {};
        static std::string getStaticNameOfClass() {
            return "Object";
        }
    protected:
        WeakPointer<Object> mPtr;
    private:

};

}



#endif /* OBJECT_HPP_ */
