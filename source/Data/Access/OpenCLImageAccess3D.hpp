#ifndef OPENCLIMAGEACCESS3D_HPP_
#define OPENCLIMAGEACCESS3D_HPP_

#include "OpenCL.hpp"
#include "Access.hpp"

namespace fast {

class OpenCLImageAccess3D {
    public:
        cl::Image3D* get() const;
        OpenCLImageAccess3D(cl::Image3D* image, bool* accessFlag, bool* accessFlag2);
        void release();
        ~OpenCLImageAccess3D();
    private:
        cl::Image3D* mImage;
        bool mIsDeleted;
        bool* mAccessFlag;
        bool* mAccessFlag2;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS3D_HPP_ */
