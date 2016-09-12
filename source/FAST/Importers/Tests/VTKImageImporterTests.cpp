#include "FAST/Testing.hpp"
#include "FAST/Importers/VTKImageImporter.hpp"
#include "FAST/Exporters/VTKImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include <vtkSmartPointer.h>

using namespace fast;

// TODO rewrite this test so that it doesn't use the vtk exporter
TEST_CASE("Import an image from VTK to FAST", "[fast][VTK][VTKImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "US/US-2D.jpg");
    Image::pointer fastImage = importer->getOutputData<Image>();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputConnection(importer->getOutputPort());
    vtkExporter->Update();

    // VTK Import example
    vtkSmartPointer<VTKImageImporter> vtkImporter = VTKImageImporter::New();
    vtkImporter->SetInputConnection(vtkExporter->GetOutputPort());
    Image::pointer importedImage = vtkImporter->getOutputData<Image>();
    vtkImporter->update();

    CHECK(importedImage->getWidth() == fastImage->getWidth());
    CHECK(importedImage->getHeight() == fastImage->getHeight());
    CHECK(importedImage->getDepth() == 1);
    CHECK(importedImage->getDimensions() == 2);
    CHECK(importedImage->getDataType() == TYPE_FLOAT);
}
