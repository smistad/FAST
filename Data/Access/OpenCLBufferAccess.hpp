#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "OpenCL.hpp"

namespace fast {

class OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLBufferAccess();
    private:
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;
};

} // end namespace fast

#endif
