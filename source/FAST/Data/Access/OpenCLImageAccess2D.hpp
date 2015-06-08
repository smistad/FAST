#ifndef OPENCLIMAGEACCESS2D_HPP_
#define OPENCLIMAGEACCESS2D_HPP_

#include "OpenCL.hpp"
#include "Access.hpp"
#include "FAST/SmartPointers.hpp"

namespace fast {

class OpenCLImageAccess2D {
    public:
        cl::Image2D* get() const;
        OpenCLImageAccess2D(cl::Image2D* image, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLImageAccess2D();
		typedef UniquePointer<OpenCLImageAccess2D> pointer;
    private:
		OpenCLImageAccess2D(const OpenCLImageAccess2D& other);
		OpenCLImageAccess2D& operator=(const OpenCLImageAccess2D& other);
        cl::Image2D* mImage;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS2D_HPP_ */
