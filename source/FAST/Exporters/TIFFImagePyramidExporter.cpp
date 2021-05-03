#include <tiffio.h>
#include <FAST/Data/ImagePyramid.hpp>
#include "TIFFImagePyramidExporter.hpp"


namespace fast {

void fast::TIFFImagePyramidExporter::loadAttributes() {
}

void TIFFImagePyramidExporter::execute() {
    if(mFilename.empty())
        throw Exception("Must set filename in TIFFImagePyramidExporter");

    auto imagePyramid = getInputData<ImagePyramid>();

    auto tiff = TIFFOpen(mFilename.c_str(), "w8");
}

TIFFImagePyramidExporter::TIFFImagePyramidExporter() {
    createInputPort<ImagePyramid>(0);
}

}