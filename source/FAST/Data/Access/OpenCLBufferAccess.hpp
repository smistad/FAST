#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "FAST/Object.hpp"


namespace fast {

class OpenCLDevice;
class DataObject;

class FAST_EXPORT OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer,  SharedPointer<DataObject> dataObject);
        void release();
        ~OpenCLBufferAccess();
		typedef std::unique_ptr<OpenCLBufferAccess> pointer;
    private:
		OpenCLBufferAccess(const OpenCLBufferAccess& other);
		OpenCLBufferAccess& operator=(const OpenCLBufferAccess& other);
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        SharedPointer<DataObject> mDataObject;
};

} // end namespace fast

#endif
