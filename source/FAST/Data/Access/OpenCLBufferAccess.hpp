#ifndef OPENCLBUFFERACCESS_HPP_
#define OPENCLBUFFERACCESS_HPP_

#include "FAST/OpenCL.hpp"
#include "FAST/Object.hpp"


namespace fast {

class OpenCLDevice;
class DataObject;

class FAST_EXPORT OpenCLBufferAccess {
    public:
        cl::Buffer* get() const;
        OpenCLBufferAccess(cl::Buffer* buffer,  std::shared_ptr<DataObject> dataObject);
        void release();
        ~OpenCLBufferAccess();
		typedef std::unique_ptr<OpenCLBufferAccess> pointer;
    private:
		OpenCLBufferAccess(const OpenCLBufferAccess& other);
		OpenCLBufferAccess& operator=(const OpenCLBufferAccess& other);
        cl::Buffer* mBuffer;
        bool mIsDeleted;
        std::shared_ptr<DataObject> mDataObject;
};

} // end namespace fast

#endif
