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
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    vtkSmartPointer<VTKMeshExporter> exporter = VTKMeshExporter::New();
    exporter->setInputConnection(importer->getOutputPort());

    exporter->Update();
    vtkSmartPointer<vtkPolyData> mesh = exporter->GetOutput();

    CHECK(mesh->GetPoints()->GetNumberOfPoints() == 386);
    CHECK(mesh->GetPolys()->GetNumberOfCells() == 768);
}

TEST_CASE("VTKMeshExporter 2D", "[fast][VTK][VTKMeshExporter]") {
	Mesh::pointer mesh = Mesh::New();
	std::vector<Vector2f> vertices = {
			Vector2f(0, 0),
			Vector2f(1, 0),
			Vector2f(0, 1.5)
	};
	std::vector<Vector2f> normals = {
			Vector2f(1, 0),
			Vector2f(0, 1),
			Vector2f(1, 1)
	};
	std::vector<VectorXui> lines = {
			Vector2ui(0, 1),
			Vector2ui(1, 2)
	};
	mesh->create(vertices, normals, lines);

    vtkSmartPointer<VTKMeshExporter> exporter = VTKMeshExporter::New();
    exporter->setInputData(mesh);

    exporter->Update();
    vtkSmartPointer<vtkPolyData> polydata = exporter->GetOutput();

    CHECK(polydata->GetPoints()->GetNumberOfPoints() == 3);
    CHECK(polydata->GetLines()->GetNumberOfCells() == 2);
}

