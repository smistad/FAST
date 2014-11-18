#ifndef SMARTPOINTERS_HPP_
#define SMARTPOINTERS_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "Exception.hpp"

#define FAST_OBJECT(className)                                  \
    public:                                                     \
        typedef SharedPointer<className> pointer;               \
        static className::pointer New() {                       \
            className * ptr = new className();                  \
            className::pointer smartPtr(ptr);                   \
            ptr->setPtr(smartPtr);                              \
                                                                \
            return smartPtr;                                    \
        }                                                       \
    private:                                                    \
        void setPtr(className::pointer ptr) {                   \
            mPtr = ptr;                                         \
        }                                                       \


namespace fast {

template <class T>
class SharedPointer;

template <class T>
class WeakPointer {
    public:
        WeakPointer() {};
        WeakPointer(const SharedPointer<T> object) {
            mWeakPtr = object.getPtr();
        }
        SharedPointer<T> lock() const {
            return SharedPointer<T>(mWeakPtr.lock());
        };
        boost::weak_ptr<T> getPtr() { return mWeakPtr; };
        WeakPointer<T> &operator=(const SharedPointer<T> &other);
        bool operator==(const WeakPointer<T> &other) const {
            return mWeakPtr.lock() == other.lock().getPtr();
        }
    private:
        boost::weak_ptr<T> mWeakPtr;

};

class Object;

template <class T>
class SharedPointer {
    public:
        SharedPointer() {

        }

        SharedPointer(Object * object) {
            mSmartPtr = boost::shared_ptr<T>(dynamic_cast<T*>(object));
        }

        template <class U>
        SharedPointer(boost::shared_ptr<U> sharedPtr) {
            mSmartPtr = boost::dynamic_pointer_cast<T>(sharedPtr);
        }

        template <class U>
        SharedPointer(SharedPointer<U> object) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(object.getPtr());
            if(ptr == NULL)
                throw Exception("Illegal cast");
            mSmartPtr = boost::shared_ptr<T>(ptr);
        }
        template <class U>
        SharedPointer(WeakPointer<U> object) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(object.getPtr().lock());
            if(ptr == NULL)
                throw Exception("Illegal cast");
            mSmartPtr = boost::shared_ptr<T>(ptr);
        }

        template <class U>
        SharedPointer<T> &operator=(const SharedPointer<U> &other) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(other.getPtr());
            if(ptr == NULL)
                throw Exception("Illegal cast");
            mSmartPtr = boost::shared_ptr<T>(ptr);
            return *this;
        }

        template <class U>
        void swap(SharedPointer<U> &other) {
            mSmartPtr.swap(other.getReferenceToPointer());
        }

        bool isValid() {
            // Check if smart pointer actually points to something
            return mSmartPtr.get() != NULL;
        }

        operator unsigned long int() const {
            return (unsigned long int)mSmartPtr.get();
        }

        bool operator==(const SharedPointer<T> &other) {
            return this->getPtr() == other.getPtr();
        }

        boost::shared_ptr<T> operator->() {
            return mSmartPtr;
        }
        boost::shared_ptr<T> getPtr() const { return mSmartPtr; };
        boost::shared_ptr<T> & getReferenceToPointer() { return mSmartPtr; };
    private:
        boost::shared_ptr<T> mSmartPtr;

};

;

template <class T>
WeakPointer<T> &WeakPointer<T>::operator=(const SharedPointer<T> &other) {
    mWeakPtr = other.getPtr();
    return *this;
}

} // end namespace fast

// A custum boost hashing function for the SharedPointers so that they can be used
// in unordered data structures. TODO verify that this works
namespace boost {
template <class U>
std::size_t hash_value(fast::SharedPointer<U> const& obj) {
    return (std::size_t)obj.getPtr().get();
}
template <class U>
std::size_t hash_value(fast::WeakPointer<U> const& obj) {
    return (std::size_t)obj.lock().getPtr().get();
}
}


#endif /* SMARTPOINTERS_HPP_ */
