#pragma once

#include "DataObject.hpp"

namespace fast {

    // A macro for creating new simple data objects
#define FAST_SIMPLE_DATA_OBJECT(NAME, DATA_TYPE)                                            \
class FAST_EXPORT NAME : public SimpleDataObject<DATA_TYPE > {                                      \
	FAST_DATA_OBJECT(NAME)                                                                       \
public:                                                                                     \
    static std::shared_ptr<NAME> create(DATA_TYPE data) {                                                         \
        std::shared_ptr<NAME> ptr(new NAME(std::move(data)));   \
        ptr->setPtr(ptr);\
        return ptr;\
    } \
protected:                                                                                  \
    NAME(DATA_TYPE data) : SimpleDataObject<DATA_TYPE>(data) {};                                                                                            \
};                                                                                          \

// A macro for creating new simple data objects with init
#define FAST_SIMPLE_DATA_OBJECT2(NAME, DATA_TYPE, VAL)                                            \
class FAST_EXPORT NAME : public SimpleDataObject<DATA_TYPE > {                                      \
	FAST_OBJECT_V4(NAME)                                                                       \
public:                                                                                     \
    static std::shared_ptr<NAME> create(DATA_TYPE data = VAL) {                                                         \
        std::shared_ptr<NAME> ptr(new NAME(std::move(data)));   \
        ptr->setPtr(ptr);\
        return ptr;\
    } \
protected:                                                                                  \
    NAME(DATA_TYPE data) : SimpleDataObject<DATA_TYPE>(data) {};                                                                                            \
};                                                                                          \

template <class DataType>
class SimpleDataObject : public DataObject {
    public:
        DataType get();
        void set(DataType data);
    protected:
        SimpleDataObject(DataType data) { m_data = data; };
        SimpleDataObject() {};
        virtual void free(ExecutionDevice::pointer device) override {};
        virtual void freeAll() override {};

        DataType m_data;
        std::mutex m_mutex;
};


template<class DataType>
DataType SimpleDataObject<DataType>::get() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_data;
}

template<class DataType>
void SimpleDataObject<DataType>::set(DataType data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data = data;
}

} // end namespace
