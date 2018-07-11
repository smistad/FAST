#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#define NOMINMAX // Removes windows min and max macros
#define _USE_MATH_DEFINES
#include "FAST/Exception.hpp"
#include "FAST/Reporter.hpp"
#include "FAST/SmartPointers.hpp"



namespace fast {

class FAST_EXPORT  Object {
    public:
        typedef SharedPointer<Object> pointer;
        virtual ~Object() {};
        static std::string getStaticNameOfClass() {
            return "Object";
        }
        Reporter& getReporter();
    protected:
        Reporter& reportError();
        Reporter& reportWarning();
        Reporter& reportInfo();
        ReporterEnd reportEnd() const;
        std::weak_ptr<Object> mPtr;
    private:
        Reporter mReporter;

};

}



#endif /* OBJECT_HPP_ */
