#ifndef ImageImporter_HPP_
#define ImageImporter_HPP_

#include "Importer.hpp"
#include <string>
#include "Image.hpp"
#include "OpenCLManager.hpp"

namespace fast {

class ImageImporter : public Importer {
    FAST_OBJECT(ImageImporter)
    public:
        Image::pointer getOutput();
        void setFilename(std::string filename);
        void setDevice(ExecutionDevice::pointer device);
        ~ImageImporter() {};
    private:
        ImageImporter();
        Image::pointer mOutput;
        WeakPointer<Image> mOutput2;
        std::string mFilename;
        ExecutionDevice::pointer mDevice;
        void execute();

};


} // end namespace fast



#endif /* ImageImporter_HPP_ */
