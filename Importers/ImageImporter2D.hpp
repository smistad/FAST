#ifndef IMAGEIMPORTER2D_HPP_
#define IMAGEIMPORTER2D_HPP_

#include "Importer.hpp"
#include <string>
#include "Image2D.hpp"
#include "OpenCLManager.hpp"

namespace fast {

class ImageImporter2D : public Importer {
    public:
        Image2DPtr getOutput();
        typedef boost::shared_ptr<ImageImporter2D> Ptr;
        static ImageImporter2D::Ptr New() {
            ImageImporter2D * ptr = new ImageImporter2D();
            ImageImporter2D::Ptr smartPtr(ptr);
            ptr->setPtr(smartPtr);

            return smartPtr;
        }

        void setFilename(std::string filename);
        void setContext(oul::Context context);
    private:
        void setPtr(ImageImporter2D::Ptr ptr) {mPtr = ptr;};
        ImageImporter2D();
        ImageImporter2D::Ptr mPtr;
        Image2D::Ptr mOutput;
        std::string mFilename;
        oul::Context mContext;
        void execute();

};


} // end namespace fast



#endif /* IMAGEIMPORTER2D_HPP_ */
