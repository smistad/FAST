#ifndef META_IMAGE_IMPORTER_HPP_
#define META_IMAGE_IMPORTER_HPP_

#include "Importer.hpp"
#include "StaticImage.hpp"

namespace fast {

/**
 * TODO Currently only supports 2D
 */
class MetaImageImporter : public Importer {
    FAST_OBJECT(MetaImageImporter)
    public:
        StaticImage::pointer getOutput();
        void setFilename(std::string filename);
        void setDevice(ExecutionDevice::pointer device);
    private:
        MetaImageImporter();
        StaticImage::pointer mTempOutput;
        WeakPointer<StaticImage> mOutput;
        std::string mFilename;
        ExecutionDevice::pointer mDevice;
        void execute();
};

} // end namespace fast

#endif
