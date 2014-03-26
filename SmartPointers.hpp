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
        SharedPointer<T> lock() { return SharedPointer<T>(weakPtr.lock()); };
        boost::weak_ptr<T> getPtr() { return weakPtr; };
        WeakPointer<T> &operator=(const SharedPointer<T> &other);
    private:
        boost::weak_ptr<T> weakPtr;

};

class PipelineObject;

template <class T>
class SharedPointer {
    public:
        SharedPointer() {

        }

        SharedPointer(PipelineObject * object) {
            smartPtr = boost::shared_ptr<T>(dynamic_cast<T*>(object));
        }

        template <class U>
        SharedPointer(boost::shared_ptr<U> sharedPtr) {
            smartPtr = boost::dynamic_pointer_cast<T>(sharedPtr);
        }

        template <class U>
        SharedPointer(SharedPointer<U> object) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(object.getPtr());
            std::cout << "asd" << std::endl;
            if(ptr == NULL)
                throw Exception("Illegal cast");
            smartPtr = boost::shared_ptr<T>(ptr);
        }
        template <class U>
        SharedPointer(WeakPointer<U> object) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(object.getPtr().lock());
            std::cout << "asd3" << std::endl;
            if(ptr == NULL)
                throw Exception("Illegal cast");
            smartPtr = boost::shared_ptr<T>(ptr);
        }

        template <class U>
        SharedPointer<T> &operator=(const SharedPointer<U> &other) {
            boost::shared_ptr<T> ptr = boost::dynamic_pointer_cast<T>(other.getPtr());
            std::cout << "asd2" << std::endl;
            if(ptr == NULL)
                throw Exception("Illegal cast");
            smartPtr = boost::shared_ptr<T>(ptr);
        }

        template <class U>
        void swap(SharedPointer<U> &other) {
            smartPtr.swap(other.getReferenceToPointer());
        }

        bool isValid() {
            return smartPtr != NULL;
        }

        bool operator==(const SharedPointer<T> &other) {
            return this->getPtr() == other.getPtr();
        }

        boost::shared_ptr<T> operator->() {
            return smartPtr;
        }
        boost::shared_ptr<T> getPtr() const { return smartPtr; };
        boost::shared_ptr<T> & getReferenceToPointer() { return smartPtr; };
    private:
        boost::shared_ptr<T> smartPtr;

};

template <class T>
WeakPointer<T> &WeakPointer<T>::operator=(const SharedPointer<T> &other) {
    weakPtr = other.getPtr();
    return *this;
}

} // end namespace fast



#endif /* SMARTPOINTERS_HPP_ */
