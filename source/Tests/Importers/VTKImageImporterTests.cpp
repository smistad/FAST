#include "catch.hpp"
#include "VTKImageImporter.hpp"
#include "VTKImageExporter.hpp"
#include "ImageImporter.hpp"

using namespace fast;

// TODO rewrite this test so that it doesn't use the vtk exporter
TEST_CASE("Import an image from VTK to FAST", "[fast][VTK]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    Image::pointer fastImage = importer->getOutput();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->SetInput(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // VTK Import example
    VTKImageImporter::pointer vtkImporter = VTKImageImporter::New();
    vtkImporter->setInput(vtkImage);
    Image::pointer importedImage = vtkImporter->getOutput();
    vtkImporter->update();

    CHECK(fastImage->getWidth() == importedImage->getWidth());
    CHECK(fastImage->getHeight() == importedImage->getHeight());
    CHECK(fastImage->getDepth() == 1);
    CHECK(fastImage->getDimensions() == 2);
    CHECK(fastImage->getDataType() == TYPE_FLOAT);
}
