#include "catch.hpp"
#include "ITKImageImporter.hpp"

using namespace fast;

TEST_CASE("Import image from ITK to FAST", "[fast][ITK]") {
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
    Image::pointer fastImage = importer->getOutput();
    importer->update();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
}
