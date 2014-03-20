#ifndef IMAGEIMPORTER2D_HPP_
#define IMAGEIMPORTER2D_HPP_

#include "Importer.hpp"
#include <string>
#include "Image2D.hpp"
#include "OpenCLManager.hpp"

namespace fast {

class ImageImporter2D : public Importer {
    public:
        ImageImporter2D(oul::Context context, std::string filename);
        Image2DPtr getOutput();
    private:
        std::string mFilename;
        oul::Context mContext;
        void execute() {};

};

typedef boost::shared_ptr<ImageImporter2D> ImageImporter2DPtr;

} // end namespace fast



#endif /* IMAGEIMPORTER2D_HPP_ */
