#include "FAST/Testing.hpp"
#include "FAST/Exporters/VTKMeshFileExporter.hpp"
#include "FAST/Data/Mesh.hpp"

using namespace fast;

TEST_CASE("No filename given to VTKMeshFileExporter", "[fast][VTKMeshFileExporter]") {
	Mesh::pointer mesh = Mesh::New();
	VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
	exporter->setInputData(mesh);
	CHECK_THROWS(exporter->update());
}

TEST_CASE("Export 2D mesh", "[fast][VTKMeshFileExporter]") {
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

	VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
	exporter->setInputData(mesh);
	exporter->setFilename("VTKMeshFileExporter2DTest.vtk");
	CHECK_NOTHROW(exporter->update());
}

TEST_CASE("Export 3D mesh", "[fast][VTKMeshFileExporter]") {
	Mesh::pointer mesh = Mesh::New();
	std::vector<Vector3f> vertices = {
			Vector3f(0, 0, 1),
			Vector3f(1, 0, 0),
			Vector3f(0, 1.5, 0)
	};
	std::vector<Vector3f> normals = {
			Vector3f(1, 0, 0),
			Vector3f(0, 1, 0),
			Vector3f(1, 1, 1)
	};
	std::vector<VectorXui> triangles = {
			Vector3ui(0, 1, 2)
	};
	mesh->create(vertices, normals, triangles);

	VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
	exporter->setInputData(mesh);
	exporter->setFilename("VTKMeshFileExporter3DTest.vtk");
	CHECK_NOTHROW(exporter->update());
}
