#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/ImagePatch/PatchGenerator.hpp>
#include <QFile>
#include "TIFFImagePyramidExporter.hpp"


namespace fast {

void fast::TIFFImagePyramidExporter::loadAttributes() {
    FileExporter::loadAttributes();
}

void TIFFImagePyramidExporter::execute() {
    if(m_filename.empty())
        throw Exception("Must set filename in TIFFImagePyramidExporter");

    auto input = getInputData<DataObject>();
    auto imagePyramid = std::dynamic_pointer_cast<ImagePyramid>(input);
    if(imagePyramid == nullptr) {
        reportInfo() << "Data given to TIFFImagePyramidExporter was an Image, not an ImagePyramid, converting ..." << reportEnd();
        auto image = std::dynamic_pointer_cast<Image>(input);
        imagePyramid = ImagePyramid::create(image->getWidth(), image->getHeight(), image->getNrOfChannels(), 256, 256);
        imagePyramid->setSpacing(image->getSpacing());
        SceneGraph::setParentNode(imagePyramid, image);
        auto access = imagePyramid->getAccess(ACCESS_READ_WRITE);
        for(int y = 0; y < image->getHeight(); y += 256) {
            for(int x = 0; x < image->getWidth(); x += 256) {
                auto width = std::min(image->getWidth() - x - 1, 256);
                auto height = std::min(image->getHeight() - y - 1, 256);
                access->setPatch(0, x, y, image->crop(Vector2i(x, y), Vector2i(width, height)));
            }
        }
    }

    if(imagePyramid->usesTIFF()) {
        // If image pyramid is using TIFF backend. It is already stored on disk, we just need to copy it..
        if(fileExists(m_filename)) {
            // If destination file already exists, we have to remove the existing file, or copy will not run.
            QFile::remove(m_filename.c_str());
        }
        QFile::copy(imagePyramid->getTIFFPath().c_str(), m_filename.c_str());
        return;
    }
    // If not, we need to do a patch based copy

    const Vector3f spacing = imagePyramid->getSpacing();

    ImageCompression compression = m_compression;
    if(!m_compressionSet || m_compression == ImageCompression::UNSPECIFIED) {
        // Default compression
        if(imagePyramid->getNrOfChannels() == 1) {
            compression = ImageCompression::LZW;
        } else if(imagePyramid->getNrOfChannels() == 3 || imagePyramid->getNrOfChannels() == 4) {
            compression = ImageCompression::JPEG;
        } else {
            throw Exception("Unexpected nr of channels in ImagePyramid: " + std::to_string(imagePyramid->getNrOfChannels()));
        }
    }

    uint photometric = PHOTOMETRIC_RGB;
    uint bitsPerSample = 8;
    uint samplesPerPixel = 3; // RGBA image pyramid is converted to RGB with getPatchAsImage
    if(imagePyramid->getNrOfChannels() == 1) {
        photometric = PHOTOMETRIC_MINISBLACK; // Photometric mask causes crash..
        samplesPerPixel = 1;
    }

    auto tiff = TIFFOpen(m_filename.c_str(), "w8");
    if(tiff == nullptr) {
        throw Exception("Unable to open file " + m_filename + " in TIFFImagePyramidExporter");
    }

    // For each level, we need to 1) write fields, 2) write tiles
    // We have to go from highest res level first
    for(int level = 0; level < imagePyramid->getNrOfLevels(); ++level) {
        reportInfo() << "Writing level " << level << reportEnd();

        // Write base tags
        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, photometric);
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, (uint16_t)samplesPerPixel);
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
                auto paddedImage = Image::create(imagePyramid->getLevelTileWidth(level), imagePyramid->getLevelTileHeight(level), image->getDataType(), image->getNrOfChannels());
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
            mRuntimeManager->stopRegularTimer("TIFF write");
            ++counter;
            mRuntimeManager->printAll();
        } while(!image->isLastFrame());

        TIFFWriteDirectory(tiff);
    }

    TIFFClose(tiff);
}

TIFFImagePyramidExporter::TIFFImagePyramidExporter() : TIFFImagePyramidExporter("") {
}

TIFFImagePyramidExporter::TIFFImagePyramidExporter(std::string filename, ImageCompression compression) : FileExporter(filename) {
    createInputPort<ImagePyramid>(0);
    if(compression != ImageCompression::UNSPECIFIED)
        setCompression(compression);
}

void TIFFImagePyramidExporter::setCompression(ImageCompression compression) {
    m_compressionSet = true;
    m_compression = compression;
}

}