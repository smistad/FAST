#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "OpenCL.hpp"

namespace fast {

class OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer, bool* accessFlag);
        void release();
        ~OpenCLBufferAccess();
    private:
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        bool* mAccessFlag;
};

} // end namespace fast

#endif
