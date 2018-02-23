#include "FAST/Testing.hpp"
#include "FAST/Importers/VTKImageImporter.hpp"
#include "FAST/Exporters/VTKImageExporter.hpp"
#include "FAST/Importers/ImageImporter.hpp"
#include <vtkSmartPointer.h>

using namespace fast;

// TODO rewrite this test so that it doesn't use the vtk exporter
TEST_CASE("Import an image from VTK to FAST", "[fast][VTK][VTKImageImporter]") {
    ImageImporter::pointer importer = ImageImporter::New();
    importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
    DataPort::pointer port = importer->getOutputPort();
    importer->update(0);
    Image::pointer fastImage = port->getNextFrame();

    // VTK Export
    vtkSmartPointer<VTKImageExporter> vtkExporter = VTKImageExporter::New();
    vtkExporter->setInputConnection(importer->getOutputPort());
    vtkExporter->Update();

    // VTK Import example
    vtkSmartPointer<VTKImageImporter> vtkImporter = VTKImageImporter::New();
    vtkImporter->SetInputConnection(vtkExporter->GetOutputPort());
    DataPort::pointer port2 = vtkImporter->getOutputPort();
    vtkImporter->update(0);
    Image::pointer importedImage = port2->getNextFrame();

    CHECK(importedImage->getWidth() == fastImage->getWidth());
    CHECK(importedImage->getHeight() == fastImage->getHeight());
    CHECK(importedImage->getDepth() == 1);
    CHECK(importedImage->getDimensions() == 2);
    CHECK(importedImage->getDataType() == TYPE_FLOAT);
}
