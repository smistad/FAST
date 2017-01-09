#ifndef SMARTPOINTERS_HPP_
#define SMARTPOINTERS_HPP_

#define NOMINMAX // Removes windows min and max macros
#include "FAST/Exception.hpp"
#include "FAST/Reporter.hpp"
#include "FAST/Paths.hpp"
#include <memory>

#define FAST_OBJECT(className)                                  \
    public:                                                     \
        typedef SharedPointer<className> pointer;               \
        static SharedPointer<className> New() {                       \
            className * ptr = new className();                  \
            SharedPointer<className> smartPtr(ptr);                   \
            ptr->setPtr(smartPtr);                              \
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
        std::weak_ptr<T> getPtr() const { return mWeakPtr; };
        WeakPointer<T> &operator=(const SharedPointer<T> &other);
        bool operator==(const WeakPointer<T> &other) const {
            // Check if the two weak pointers, point to the same objecs
            SharedPointer<T> object1 = mWeakPtr.lock();
            SharedPointer<T> object2 = other.lock();
            if(object1.isValid() && object2.isValid()) {
                return object1 == object2;
            } else {
                return false;
            }
        }
    private:
        std::weak_ptr<T> mWeakPtr;

};

class Object;

template <class T>
class SharedPointer {
    public:
        SharedPointer() {

        }
		SharedPointer(T* object) {
            mSmartPtr = std::shared_ptr<T>(object);
        }
        template <class D>
        SharedPointer(T* p, D d) {
        	mSmartPtr = std::shared_ptr<T>(p, d);
        }

        template <class U>
        SharedPointer(std::shared_ptr<U> sharedPtr) {
            mSmartPtr = std::dynamic_pointer_cast<T>(sharedPtr);
        }

        template <class U>
        SharedPointer(SharedPointer<U> object) {
            if(!object.isValid())
                throw Exception("Cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass() + " failed because object was invalid (uninitialized or deleted).");
            std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(object.getPtr());
            if(ptr == NULL)
                throw Exception("Illegal cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass());
            mSmartPtr = std::shared_ptr<T>(ptr);
        }
        template <class U>
        SharedPointer(WeakPointer<U> object) {
            if(!object.isValid())
                throw Exception("Cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass() + " failed because object was invalid (uninitialized or deleted).");
            std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(object.getPtr().lock());
            if(ptr == NULL)
                throw Exception("Illegal cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass());
            mSmartPtr = std::shared_ptr<T>(ptr);
        }

        template <class U>
        SharedPointer<T> &operator=(const SharedPointer<U> &other) {
            if(!other.isValid())
                throw Exception("Cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass() + " failed because object was invalid (uninitialized or deleted).");
            std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(other.getPtr());
            if(ptr == NULL)
                throw Exception("Illegal cast from " + U::getStaticNameOfClass() + " to " + T::getStaticNameOfClass());
            mSmartPtr = std::shared_ptr<T>(ptr);
            return *this;
        }

        template <class U>
        void swap(SharedPointer<U> &other) {
            mSmartPtr.swap(other.getReferenceToPointer());
        }

        bool isValid() const {
            // Check if smart pointer actually points to something
            return mSmartPtr.get() != NULL;
        }

        operator unsigned long int() const {
            return (unsigned long int)mSmartPtr.get();
        }

        bool operator==(const SharedPointer<T> &other) {
            return this->getPtr() == other.getPtr();
        }

        T* operator->() {
        	return mSmartPtr.get();
		}
        T* operator->() const {
        	return mSmartPtr.get();
		}
        T* get() {
        	return mSmartPtr.get();
        }

        std::shared_ptr<T> getPtr() const { return mSmartPtr; };
        std::shared_ptr<T> & getReferenceToPointer() { return mSmartPtr; };
    private:
        std::shared_ptr<T> mSmartPtr;

};

template <class T>
using UniquePointer = std::unique_ptr<T>;

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
