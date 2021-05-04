#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include "TIFFImagePyramidExporter.hpp"


namespace fast {

void fast::TIFFImagePyramidExporter::loadAttributes() {
    FileExporter::loadAttributes();
}

void TIFFImagePyramidExporter::execute() {
    if(mFilename.empty())
        throw Exception("Must set filename in TIFFImagePyramidExporter");

    auto imagePyramid = getInputData<ImagePyramid>();
    const Vector3f spacing = imagePyramid->getSpacing();

    ImageCompression compression = m_compression;
    if(!m_compressionSet) {
        // Default compression
        if(imagePyramid->getNrOfChannels() == 1) {
            compression = ImageCompression::LZW;
        } else if(imagePyramid->getNrOfChannels() == 3 || imagePyramid->getNrOfChannels() == 4) {
            compression = ImageCompression::JPEG;
        } else {
            throw Exception("Unexpected nr of channels in ImagePyramid");
        }
    }
    uint photometric = PHOTOMETRIC_RGB;
    uint bitsPerSample = 8;
    uint samplesPerPixel = 3; // RGBA image pyramid is converted to RGB with getPatchAsImage
    if(imagePyramid->getNrOfChannels() == 1) {
        photometric = PHOTOMETRIC_MINISBLACK; // Photometric mask causes crash..
        samplesPerPixel = 1;
    }

    auto tiff = TIFFOpen(mFilename.c_str(), "w8");
    if(tiff == nullptr) {
        throw Exception("Unable to open file " + mFilename + " in TIFFImagePyramidExporter");
    }

    // For each level, we need to 1) write fields, 2) write tiles
    // We have to go from highest res level first
    for(int level = 0; level < imagePyramid->getNrOfLevels(); ++level) {
        reportInfo() << "Writing level " << level << reportEnd();

        // Write base tags
        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric);
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

        if(level > 0) {
            // All levels except highest res level should have this tag?
            TIFFSetField(tiff, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE);
        }
        switch(compression) {
            case ImageCompression::RAW:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
                break;
            case ImageCompression::LZW:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
                break;
            case ImageCompression::JPEG:
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
                break;
            case ImageCompression::JPEG2000:
                // TODO NOT IMPLEMENTED
                throw NotImplementedException();
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_JP2000);
                break;
        }

        TIFFSetField(tiff, TIFFTAG_TILEWIDTH, imagePyramid->getLevelTileWidth(level));
        TIFFSetField(tiff, TIFFTAG_TILELENGTH, imagePyramid->getLevelTileHeight(level));
        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, imagePyramid->getLevelWidth(level));
        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, imagePyramid->getLevelHeight(level));
        if(spacing.x() != 1 && spacing.y() != 1) { // Spacing == 1 means not set.
            TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER);
            float scaleX = (float) imagePyramid->getFullWidth() / imagePyramid->getLevelWidth(level);
            float scaleY = (float) imagePyramid->getFullHeight() / imagePyramid->getLevelHeight(level);
            TIFFSetField(tiff, TIFFTAG_XRESOLUTION,
                         1.0f / (spacing.x() / 10) * scaleX); // Convert to cm, and adjust for level
            TIFFSetField(tiff, TIFFTAG_YRESOLUTION,
                         1.0f / (spacing.y() / 10) * scaleY); // Convert to cm, and adjust for level
        }

        auto generator = PatchGenerator::New();
        generator->setInputData(imagePyramid);
        generator->setPatchLevel(level);
        generator->setPatchSize(imagePyramid->getLevelTileWidth(level), imagePyramid->getLevelTileHeight(level));
        auto port = generator->getOutputPort();

        Image::pointer image;
        int counter = 0;
        do {
            generator->update();
            image = port->getNextFrame<Image>();

            // Write tile to tiff level
            if(image->getWidth() != imagePyramid->getLevelTileWidth(level) || image->getHeight() != imagePyramid->getLevelTileHeight(level)) {
                // Have to pad the image, TIFF expects all tiles to be equal
                // TODO improve
                auto paddedImage = Image::New();
                paddedImage->create(imagePyramid->getLevelTileWidth(level), imagePyramid->getLevelTileHeight(level), image->getDataType(), image->getNrOfChannels());
                if(imagePyramid->getNrOfChannels() >= 3) {
                    paddedImage->fill(255);
                } else {
                    paddedImage->fill(0);
                }
                auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
                {
                    auto dest = paddedImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                    auto src = image->getOpenCLImageAccess(ACCESS_READ, device);
                    device->getCommandQueue().enqueueCopyImage(*src->get2DImage(), *dest->get2DImage(),
                                                               createOrigoRegion(), createOrigoRegion(),
                                                               createRegion(image->getSize()));
                    device->getCommandQueue().finish();
                }
                if(image->isLastFrame())
                    paddedImage->setLastFrame("PatchGenerator");
                image = paddedImage;
            }
            auto data = image->getImageAccess(ACCESS_READ)->get();
            std::size_t byteSize = getSizeOfDataType(image->getDataType(), image->getNrOfChannels())*image->getNrOfVoxels();
            mRuntimeManager->startRegularTimer("TIFF write");
            TIFFWriteEncodedTile(tiff, counter, data, byteSize);
            mRuntimeManager->startRegularTimer("TIFF stop");
            ++counter;
        } while(!image->isLastFrame());

        TIFFWriteDirectory(tiff);
    }

    TIFFClose(tiff);
}

TIFFImagePyramidExporter::TIFFImagePyramidExporter() {
    createInputPort<ImagePyramid>(0);
}

void TIFFImagePyramidExporter::setCompression(ImageCompression compression) {
    m_compressionSet = true;
    m_compression = compression;
}

}