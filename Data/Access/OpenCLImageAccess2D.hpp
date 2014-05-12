#ifndef OPENCLIMAGEACCESS2D_HPP_
#define OPENCLIMAGEACCESS2D_HPP_

#include "OpenCL.hpp"
#include "Access.hpp"

namespace fast {

class OpenCLImageAccess2D {
    public:
        cl::Image2D* get() const;
        OpenCLImageAccess2D(cl::Image2D* image, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLImageAccess2D();
    private:
        cl::Image2D* mImage;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS2D_HPP_ */
