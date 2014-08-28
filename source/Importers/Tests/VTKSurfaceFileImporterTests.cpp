#include "catch.hpp"
#include "VTKSurfaceFileImporter.hpp"

namespace fast {

TEST_CASE("No filename given to VTKSurfaceFileImporter throws exception", "[fast][VTKSurfaceFileImporter]") {
    VTKSurfaceFileImporter::pointer importer = VTKSurfaceFileImporter::New();
    CHECK_THROWS(importer->update());
}

TEST_CASE("Wrong filename VTKSurfaceFileImporter throws exception", "[fast][VTKSurfaceFileImporter]") {
    VTKSurfaceFileImporter::pointer importer = VTKSurfaceFileImporter::New();
    importer->setFilename("asdasd");
    CHECK_THROWS(importer->update());
}

TEST_CASE("Importer VTK surface from file", "[fast][VTKSurfaceFileImporter]") {
    VTKSurfaceFileImporter::pointer importer = VTKSurfaceFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    Surface::pointer surface = importer->getOutput();
    importer->update();

    // Check the surface
    CHECK(surface->getNrOfTriangles() == 768);
    CHECK(surface->getNrOfVertices() == 386);
}

} // end namespace fast
