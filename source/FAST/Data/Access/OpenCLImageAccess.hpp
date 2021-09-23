#ifndef OPENCLIMAGEACCESS_HPP_
#define OPENCLIMAGEACCESS_HPP_

#include "FAST/OpenCL.hpp"
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
        OpenCLImageAccess(cl::Image2D* image, std::shared_ptr<Image> object);
        OpenCLImageAccess(cl::Image3D* image, std::shared_ptr<Image> object);
        void release();
        ~OpenCLImageAccess();
		typedef std::unique_ptr<OpenCLImageAccess> pointer;
    private:
		OpenCLImageAccess(const OpenCLImageAccess& other);
		OpenCLImageAccess& operator=(const OpenCLImageAccess& other);
        cl::Image* mImage;
        bool mIsDeleted;
        std::shared_ptr<Image> mImageObject;

};

} // end namespace fast



#endif /* OPENCLIMAGEACCESS3D_HPP_ */
