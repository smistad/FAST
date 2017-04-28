#include "FAST/Testing.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Data/Mesh.hpp"

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
    importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    importer->update();
    Mesh::pointer surface = importer->getOutputData<Mesh>(0);

    // Check the surface
    CHECK(surface->getNrOfTriangles() == 768);
    CHECK(surface->getNrOfVertices() == 386);
}

} // end namespace fast
