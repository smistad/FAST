#include "FAST/Testing.hpp"
#include "FAST/Exporters/VTKLineSetExporter.hpp"
#include "FAST/Importers/VTKLineSetFileImporter.hpp"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

using namespace fast;


TEST_CASE("VTKLineSetExporter", "[fast][VTK][VTKLineSetExporter]") {
    VTKLineSetFileImporter::pointer importer = VTKLineSetFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "centerline.vtk");

    vtkSmartPointer<VTKLineSetExporter> exporter = VTKLineSetExporter::New();
    exporter->setInputConnection(importer->getOutputPort());

    exporter->Update();
    vtkSmartPointer<vtkPolyData> lines = exporter->GetOutput();

    CHECK(lines->GetPoints()->GetNumberOfPoints() == 89);
    CHECK(lines->GetLines()->GetNumberOfCells() == 97);
}

