#include "catch.hpp"
#include "IterativeClosestPoint.hpp"
#include "VTKPointSetFileImporter.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    PointSet::pointer A = PointSet::New();
    PointSetAccess accessA = A->getAccess(ACCESS_READ_WRITE);
    accessA.addPoint(Vector3f(2,2,1));
    accessA.addPoint(Vector3f(6,2,2));
    accessA.addPoint(Vector3f(2,6,1));
    accessA.release();

    PointSet::pointer B = PointSet::New();
    PointSetAccess accessB = B->getAccess(ACCESS_READ_WRITE);
    accessB.addPoint(Vector3f(3,2,0));
    accessB.addPoint(Vector3f(7,0,0));
    accessB.addPoint(Vector3f(9,5,0));
    accessB.addPoint(Vector3f(2,1,1));
    accessB.addPoint(Vector3f(2,1,8));
    accessB.release();

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
    Eigen::Transform<float, 3, Eigen::Affine> transform = Eigen::Transform<float,3,Eigen::Affine>::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    LinearTransformation transformation;
    transformation.setTransform(transform);
    importerB->update();
    PointSet::pointer B = importerB->getOutputData<PointSet>(0);
    B->getSceneGraphNode()->setTransformation(transformation);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().getTransform().translation();

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
    Eigen::Transform<float, 3, Eigen::Affine> transform = Eigen::Transform<float,3,Eigen::Affine>::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    LinearTransformation transformation;
    transformation.setTransform(transform);
    importerB->update();
    PointSet::pointer B = importerB->getOutputData<PointSet>(0);
    B->getSceneGraphNode()->setTransformation(transformation);

    importerA->update();
    PointSet::pointer A = importerA->getOutputData<PointSet>(0);
    PointSetAccess access = A->getAccess(ACCESS_READ_WRITE);
    access.addPoint(access.getPoint(0));
    access.release();

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSet(A);
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().getTransform().translation();

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

    Eigen::Transform<float, 3, Eigen::Affine> transformInit = Eigen::Transform<float,3,Eigen::Affine>::Identity();
    LinearTransformation FASTtransformInit;
    FASTtransformInit.setTransform(transformInit);
    SceneGraph::insertParentNode(A, FASTtransformInit);
    SceneGraph::insertParentNode(B, FASTtransformInit);

    // Apply a transformation to B surface
    Eigen::Transform<float, 3, Eigen::Affine> transform = Eigen::Transform<float,3,Eigen::Affine>::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    LinearTransformation transformation;
    transformation.setTransform(transform);
    B->getSceneGraphNode()->setTransformation(transformation);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingPointSetPort(importerA->getOutputPort());
    icp->setFixedPointSetPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getOutputData<PointSet>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation().getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation().getTransform().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()));
}



} // end namespace fast
