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

TEST_CASE("Export mesh with lines", "[fast][VTKMeshFileExporter]") {
    Mesh::pointer mesh = Mesh::New();
    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 1, 0)),
            MeshVertex(Vector3f(1, 25, 0)),
            MeshVertex(Vector3f(25, 20, 0)),
            MeshVertex(Vector3f(20, 1, 0)),
    };
    std::vector<MeshLine> lines = {
            MeshLine(0, 1),
            MeshLine(1, 2),
            MeshLine(2, 3),
            MeshLine(3, 0)
    };

    mesh->create(vertices, lines);

	VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
	exporter->setInputData(mesh);
	exporter->setFilename("VTKMeshFileExporter2DTest.vtk");
	CHECK_NOTHROW(exporter->update());
}

TEST_CASE("Export mesh with triangles", "[fast][VTKMeshFileExporter]") {

    Mesh::pointer mesh = Mesh::New();
    std::vector<MeshVertex> vertices = {
            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(1, 10, 10)),

            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(30, 15, 15)),

            MeshVertex(Vector3f(1, 1, 10)),
            MeshVertex(Vector3f(1, 10, 10)),
            MeshVertex(Vector3f(30, 15, 15)),

            MeshVertex(Vector3f(1, 1, 1)),
            MeshVertex(Vector3f(1, 10, 10)),
            MeshVertex(Vector3f(30, 15, 15))
    };
    std::vector<MeshTriangle> triangles = {
            MeshTriangle(0, 1, 2),
            MeshTriangle(3, 4, 5),
            MeshTriangle(6, 7, 8),
            MeshTriangle(9, 10, 11)
    };

    mesh->create(vertices, {}, triangles);

	VTKMeshFileExporter::pointer exporter = VTKMeshFileExporter::New();
	exporter->setInputData(mesh);
	exporter->setFilename("VTKMeshFileExporter3DTest.vtk");
	CHECK_NOTHROW(exporter->update());
}
