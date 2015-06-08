#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/VTKImageImporter.hpp"
#include "FAST/Exporters/VTKImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"

using namespace fast;

// TODO rewrite this test so that it doesn't use the vtk exporter
TEST_CASE("Import an image from VTK to FAST", "[fast][VTK]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US-2D.jpg");
    Image::pointer fastImage = importer->getOutputData<Image>();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputData(fastImage);
    vtkSmartPointer<vtkImageData> vtkImage = vtkExporter->GetOutput();
    vtkExporter->Update();

    // VTK Import example
    VTKImageImporter::pointer vtkImporter = VTKImageImporter::New();
    //vtkImporter->setInput(vtkImage);
    Image::pointer importedImage = vtkImporter->getOutputData<Image>();
    vtkImporter->update();

    CHECK(fastImage->getWidth() == importedImage->getWidth());
    CHECK(fastImage->getHeight() == importedImage->getHeight());
    CHECK(fastImage->getDepth() == 1);
    CHECK(fastImage->getDimensions() == 2);
    CHECK(fastImage->getDataType() == TYPE_FLOAT);
}
