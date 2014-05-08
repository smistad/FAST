#include "catch.hpp"
#include "ITKImageExporter.hpp"
#include "ITKImageImporter.hpp"
#include "ImageImporter2D.hpp"

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

TEST_CASE("Export image to ITK from FAST", "[fast][ITK]") {

    ImageImporter2D::pointer importer = ImageImporter2D::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "lena.jpg");
    Image::pointer fastImage = importer->getOutput();

    // ITK Export example
    typedef itk::Image<float, 2> ImageType;
    ITKImageExporter<ImageType>::Pointer itkExporter = ITKImageExporter<ImageType>::New();
    itkExporter->SetInput(fastImage);
    ImageType::Pointer itkImage = itkExporter->GetOutput();
    itkExporter->Update();

    ImageType::RegionType region = itkImage->GetLargestPossibleRegion();

    CHECK(fastImage->getWidth() == region.GetSize()[0]);
    CHECK(fastImage->getHeight() == region.GetSize()[1]);
}
