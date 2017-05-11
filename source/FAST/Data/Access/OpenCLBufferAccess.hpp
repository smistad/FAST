#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class Image;
class OpenCLDevice;

class FAST_EXPORT  OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer,  SharedPointer<Image> image);
        void release();
        ~OpenCLBufferAccess();
		typedef UniquePointer<OpenCLBufferAccess> pointer;
    private:
		OpenCLBufferAccess(const OpenCLBufferAccess& other);
		OpenCLBufferAccess& operator=(const OpenCLBufferAccess& other);
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        SharedPointer<Image> mImage;
};

} // end namespace fast

#endif
