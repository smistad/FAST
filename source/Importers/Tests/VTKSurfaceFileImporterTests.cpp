#include "VTKMeshFileImporter.hpp"
#include "catch.hpp"

namespace fast {

TEST_CASE("No filename given to VTKMeshFileImporter throws exception", "[fast][VTKMeshFileImporter]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Wrong filename VTKMeshFileImporter throws exception", "[fast][VTKMeshFileImporter]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename("asdasd");
    CHECK_THROWS(importer->update());
}

TEST_CASE("Importer VTK surface from file", "[fast][VTKMeshFileImporter]") {
    VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    Mesh::pointer surface = importer->getOutput();
    importer->update();

    // Check the surface
    CHECK(surface->getNrOfTriangles() == 768);
    CHECK(surface->getNrOfVertices() == 386);
}

} // end namespace fast
