#ifndef OBJECT_HPP_
#define OBJECT_HPP_

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
        WeakPointer<Object> mPtr;
        Reporter& reportError();
        Reporter& reportWarning();
        Reporter& reportInfo();
        ReporterEnd reportEnd() const;
    private:
        Reporter mReporter;

};

}



#endif /* OBJECT_HPP_ */
