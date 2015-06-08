#ifndef OPENCLIMAGEACCESS3D_HPP_
#define OPENCLIMAGEACCESS3D_HPP_

#include "OpenCL.hpp"
#include "Access.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class OpenCLImageAccess3D {
    public:
        cl::Image3D* get() const;
        OpenCLImageAccess3D(cl::Image3D* image, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLImageAccess3D();
		typedef UniquePointer<OpenCLImageAccess3D> pointer;
    private:
		OpenCLImageAccess3D(const OpenCLImageAccess3D& other);
		OpenCLImageAccess3D& operator=(const OpenCLImageAccess3D& other);
        cl::Image3D* mImage;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS3D_HPP_ */
