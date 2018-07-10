#include "FAST/Testing.hpp"
#include "FAST/Importers/ITKImageImporter.hpp"

using namespace fast;

TEST_CASE("Import image from ITK to FAST", "[fast][ITK][ITKImageImporter]") {
    // First create an ITK image
    typedef itk::Image<float, 2> ImageType;
    ImageType::Pointer image = ImageType::New();
    ImageType::RegionType region;
    ImageType::SizeType size;
    size[0] = 128;
    size[1] = 64;
    region.SetSize(size);
    image->SetRegions(region);
    image->Allocate();

    // Try to import the image to FAST
    ITKImageImporter<ImageType>::pointer importer = ITKImageImporter<ImageType>::New();
    importer->setInput(image);
    DataPort::pointer port = importer->getOutputPort();
    importer->update(0);
    Image::pointer fastImage = port->getNextFrame<Image>();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
    CHECK(fastImage->getDimensions() == 2);
    CHECK(fastImage->getDataType() == TYPE_FLOAT);
}
