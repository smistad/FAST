#pragma once
#include <memory>

#define FAST_OBJECT(className)                                  \
    public:                                                     \
        typedef SharedPointer<className> pointer;               \
        static SharedPointer<className> New() {                       \
            SharedPointer<className> smartPtr(new className());   \
            smartPtr->setPtr(smartPtr);                              \
                                                                \
            return smartPtr;                                    \
        }                                                       \
        virtual std::string getNameOfClass() const {            \
            return std::string(#className);                     \
        };                                                      \
        static std::string getStaticNameOfClass() {             \
            return std::string(#className);                     \
        };                                                      \
    private:                                                    \
        void setPtr(className::pointer ptr) {                   \
            mPtr = ptr;                                         \
        }                                                       \



#ifdef WIN32

namespace fast {

template<class T>
class SharedPointer : public std::shared_ptr<T> {
    using std::shared_ptr<T>::shared_ptr; // inherit constructor

};

}; // end namespace fast

// Custom hashing functions for the smart pointers so that they can be used in unordered_map etc.
namespace std {
template <class U>
class hash<fast::SharedPointer<U> >{
    public:
        size_t operator()(const fast::SharedPointer<U> &object) const {
            return (std::size_t)object.get();
        }
};

} // end namespace std

#else

namespace fast {

    template<class T>
    using SharedPointer = std::shared_ptr<T>;

}

#endif


