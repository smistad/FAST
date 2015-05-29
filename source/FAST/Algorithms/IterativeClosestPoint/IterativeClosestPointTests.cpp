#include "FAST/Tests/catch.hpp"
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/Importers/VTKPointSetFileImporter.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    PointSet::pointer A = PointSet::New();
    PointSetAccess::pointer accessA = A->getAccess(ACCESS_READ_WRITE);
    accessA->addPoint(Vector3f(2,2,1));
	accessA->addPoint(Vector3f(6, 2, 2));
	accessA->addPoint(Vector3f(2, 6, 1));
	accessA->release();

    PointSet::pointer B = PointSet::New();
	PointSetAccess::pointer accessB = B->getAccess(ACCESS_READ_WRITE);
	accessB->addPoint(Vector3f(3, 2, 0));
	accessB->addPoint(Vector3f(7, 0, 0));
	accessB->addPoint(Vector3f(9, 5, 0));
	accessB->addPoint(Vector3f(2, 1, 1));
	accessB->addPoint(Vector3f(2, 1, 8));
	accessB->release();

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setInputData(0, B);
    icp->setInputData(1, A);

    CHECK_NOTHROW(icp->update());

}

TEST_CASE("ICP on two point sets", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);

    VTKPointSetFileImporter::pointer importerA = VTKPointSetFileImporter::New();
    importerA->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    VTKPointSetFileImporter::pointer importerB = VTKPointSetFileImporter::New();
    importerB->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    // Apply a transformation to B surface
    AffineTransformation transformation;
    transformation.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transformation.rotate(R);
    importerB->update();
    PointSet::pointer B = importerB->getOutputData<PointSet>(0);
    B->getSceneGraphNode()->setTransformation(transformation);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()));
}


TEST_CASE("ICP on two point sets where moving point set is larger than the fixed", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.0, 0.5, 0.0);

    VTKPointSetFileImporter::pointer importerA = VTKPointSetFileImporter::New();
    importerA->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    VTKPointSetFileImporter::pointer importerB = VTKPointSetFileImporter::New();
    importerB->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    // Apply a transformation to B surface
    AffineTransformation transformation;
    transformation.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transformation.rotate(R);
    importerB->update();
    PointSet::pointer B = importerB->getOutputData<PointSet>(0);
    B->getSceneGraphNode()->setTransformation(transformation);

    importerA->update();
    PointSet::pointer A = importerA->getOutputData<PointSet>(0);
    PointSetAccess::pointer access = A->getAccess(ACCESS_READ_WRITE);
    access->addPoint(access->getPoint(0));
    access->release();

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSet(A);
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()));
}

TEST_CASE("ICP on two point sets which are already transformed by scene graph", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);

    VTKPointSetFileImporter::pointer importerA = VTKPointSetFileImporter::New();
    importerA->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");
    VTKPointSetFileImporter::pointer importerB = VTKPointSetFileImporter::New();
    importerB->setFilename(std::string(FAST_TEST_DATA_DIR) + "Surface_LV.vtk");

    importerA->update();
    importerB->update();
    PointSet::pointer A = importerA->getOutputData<PointSet>();
    PointSet::pointer B = importerB->getOutputData<PointSet>();

    AffineTransformation FASTtransformInit;
    SceneGraph::insertParentNodeToData(A, FASTtransformInit);
    SceneGraph::insertParentNodeToData(B, FASTtransformInit);

    // Apply a transformation to B surface
    AffineTransformation transformation;
    transformation.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transformation.rotate(R);
    B->getSceneGraphNode()->setTransformation(transformation);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()));
}



} // end namespace fast
