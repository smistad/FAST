#ifndef IMAGEIMPORTER2D_HPP_
#define IMAGEIMPORTER2D_HPP_

#include "Importer.hpp"
#include <string>
#include "Image2D.hpp"
#include "OpenCLManager.hpp"

namespace fast {

class ImageImporter2D : public Importer {
    FAST_OBJECT(ImageImporter2D)
    public:
        Image2D::Ptr getOutput();
        void setFilename(std::string filename);
        void setDevice(ExecutionDevice::Ptr device);
        ~ImageImporter2D() {};
    private:
        ImageImporter2D();
        Image2D::Ptr mOutput;
        WeakPointer<Image2D> mOutput2;
        std::string mFilename;
        ExecutionDevice::Ptr mDevice;
        void execute();

};


} // end namespace fast



#endif /* IMAGEIMPORTER2D_HPP_ */
