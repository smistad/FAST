#ifndef META_IMAGE_IMPORTER_HPP_
#define META_IMAGE_IMPORTER_HPP_

#include "Importer.hpp"
#include "Image.hpp"

namespace fast {

class MetaImageImporter : public Importer {
    FAST_OBJECT(MetaImageImporter)
    public:
        Image::pointer getOutput();
        void setFilename(std::string filename);
        void setDevice(ExecutionDevice::pointer device);
    private:
        MetaImageImporter();
        Image::pointer mOutput;
        std::string mFilename;
        ExecutionDevice::pointer mDevice;
        void execute();
};

} // end namespace fast

#endif
