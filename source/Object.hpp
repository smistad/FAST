#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "SmartPointers.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace fast {

class Object {
    public:
        typedef SharedPointer<Object> pointer;
        virtual ~Object() {};
        static unsigned long currentDrawable;
    protected:
        void setOpenGLContext(unsigned long* OpenGLContext);
        void releaseOpenGLContext();
    private:
        static boost::condition_variable condition;
        static bool GLcontextReady;
        static boost::mutex GLmutex;

};

}



#endif /* OBJECT_HPP_ */
