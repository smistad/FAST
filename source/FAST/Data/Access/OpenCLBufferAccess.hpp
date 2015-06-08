#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "OpenCL.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLBufferAccess();
		typedef UniquePointer<OpenCLBufferAccess> pointer;
    private:
		OpenCLBufferAccess(const OpenCLBufferAccess& other);
		OpenCLBufferAccess& operator=(const OpenCLBufferAccess& other);
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast

#endif
