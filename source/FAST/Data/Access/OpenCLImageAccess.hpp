#ifndef OPENCLIMAGEACCESS_HPP_
#define OPENCLIMAGEACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "Access.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class OpenCLImageAccess {
    public:
        cl::Image* get() const;
        cl::Image2D* get2DImage() const;
        cl::Image3D* get3DImage() const;
        OpenCLImageAccess(cl::Image2D* image, bool* accessFlag, bool* accessFlag2);
        OpenCLImageAccess(cl::Image3D* image, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLImageAccess();
		typedef UniquePointer<OpenCLImageAccess> pointer;
    private:
		OpenCLImageAccess(const OpenCLImageAccess& other);
		OpenCLImageAccess& operator=(const OpenCLImageAccess& other);
        cl::Image* mImage;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS3D_HPP_ */
