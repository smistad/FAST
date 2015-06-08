#include "FAST/Tests/catch.hpp"
#include "FAST/Importers/VTKLineSetFileImporter.hpp"
#include "FAST/Data/Access/LineSetAccess.hpp"
#include "FAST/Data/LineSet.hpp"

using namespace fast;

TEST_CASE("VTKLineSetFileImporter", "[fast][VTKLineSetFileImporter]") {
    VTKLineSetFileImporter::pointer importer = VTKLineSetFileImporter::New();
    importer->setFilename(std::string(FAST_TEST_DATA_DIR) + "centerline.vtk");
    importer->update();
    LineSet::pointer lineSet = importer->getOutputData<LineSet>(0);

    LineSetAccess::pointer access = lineSet->getAccess(ACCESS_READ);
    CHECK(access->getNrOfPoints() == 89);
    CHECK(access->getNrOfLines() == 97);

}
