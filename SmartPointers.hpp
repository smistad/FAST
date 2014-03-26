#ifndef SMARTPOINTERS_HPP_
#define SMARTPOINTERS_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "Exception.hpp"

namespace fast {

template <class T>
class SharedPointer;

template <class T>
class WeakPointer {
    public:
        SharedPointer<T> lock() { return SharedPointer<T>(mWeakPtr.lock()); };
        boost::weak_ptr<T> getPtr() { return mWeakPtr; };
        WeakPointer<T> &operator=(const SharedPointer<T> &other);
    private:
        boost::weak_ptr<T> mWeakPtr;

};

class PipelineObject;

template <class T>
class SharedPointer {
    public:
        SharedPointer() {

        }

        SharedPointer(PipelineObject * object) {
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
        }

        template <class U>
        void swap(SharedPointer<U> &other) {
            mSmartPtr.swap(other.getReferenceToPointer());
        }

        bool isValid() {
            return mSmartPtr != NULL;
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

template <class T>
WeakPointer<T> &WeakPointer<T>::operator=(const SharedPointer<T> &other) {
    mWeakPtr = other.getPtr();
    return *this;
}

} // end namespace fast



#endif /* SMARTPOINTERS_HPP_ */
