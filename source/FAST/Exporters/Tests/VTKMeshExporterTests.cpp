#include "FAST/Testing.hpp"
#include "FAST/Exporters/VTKMeshExporter.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Data/Mesh.hpp"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

using namespace fast;


TEST_CASE("VTKMeshExporter 3D", "[fast][VTK][VTKMeshExporter]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");

    vtkSmartPointer<VTKMeshExporter> exporter = VTKMeshExporter::New();
    exporter->setInputConnection(importer->getOutputPort());

    exporter->Update();
    vtkSmartPointer<vtkPolyData> mesh = exporter->GetOutput();

    CHECK(mesh->GetPoints()->GetNumberOfPoints() == 386);
    CHECK(mesh->GetPolys()->GetNumberOfCells() == 768);
}

TEST_CASE("VTKMeshExporter centerline", "[fast][VTK][VTKMeshExporter]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(Config::getTestDataPath() + "centerline.vtk");

    vtkSmartPointer<VTKMeshExporter> exporter = VTKMeshExporter::New();
    exporter->setInputConnection(importer->getOutputPort());

    exporter->Update();
    vtkSmartPointer<vtkPolyData> lines = exporter->GetOutput();

    CHECK(lines->GetPoints()->GetNumberOfPoints() == 89);
    CHECK(lines->GetLines()->GetNumberOfCells() == 97);
}
