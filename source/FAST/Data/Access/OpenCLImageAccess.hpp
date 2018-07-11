#ifndef OPENCLIMAGEACCESS_HPP_
#define OPENCLIMAGEACCESS_HPP_

#include "CL/OpenCL.hpp"
#include "Access.hpp"
#include "FAST/Object.hpp"


namespace fast {

class Image;
class OpenCLDevice;

class FAST_EXPORT  OpenCLImageAccess {
    public:
        cl::Image* get() const;
        cl::Image2D* get2DImage() const;
        cl::Image3D* get3DImage() const;
        OpenCLImageAccess(cl::Image2D* image, SharedPointer<Image> object);
        OpenCLImageAccess(cl::Image3D* image, SharedPointer<Image> object);
        void release();
        ~OpenCLImageAccess();
		typedef std::unique_ptr<OpenCLImageAccess> pointer;
    private:
		OpenCLImageAccess(const OpenCLImageAccess& other);
		OpenCLImageAccess& operator=(const OpenCLImageAccess& other);
        cl::Image* mImage;
        bool mIsDeleted;
        SharedPointer<Image> mImageObject;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS3D_HPP_ */
