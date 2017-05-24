#include "FAST/Testing.hpp"
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"

namespace fast {

TEST_CASE("IterativeClosestPoint", "[fast][IterativeClosestPoint][icp]") {

    std::vector<MeshVertex> vertices;
    vertices.push_back(MeshVertex(Vector3f(2,2,1)));
    vertices.push_back(MeshVertex(Vector3f(6,2,2)));
    vertices.push_back(MeshVertex(Vector3f(2,6,1)));
    Mesh::pointer A = Mesh::New();
    A->create(vertices);

    std::vector<MeshVertex> verticesB;
    verticesB.push_back(MeshVertex(Vector3f(3,2,0)));
    verticesB.push_back(MeshVertex(Vector3f(7,0,0)));
    verticesB.push_back(MeshVertex(Vector3f(9,5,0)));
    verticesB.push_back(MeshVertex(Vector3f(2,1,1)));
    verticesB.push_back(MeshVertex(Vector3f(2,1,8)));
    Mesh::pointer B = Mesh::New();
    B->create(verticesB);

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setInputData(0, B);
    icp->setInputData(1, A);

    CHECK_NOTHROW(icp->update());
}

TEST_CASE("ICP on two point sets translation only", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    importerB->update();
    Mesh::pointer B = importerB->getOutputData<Mesh>(0);
    AffineTransformation::pointer T = AffineTransformation::New();
    T->setTransform(transform);
    B->getSceneGraphNode()->setTransformation(T);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setTransformationType(IterativeClosestPoint::TRANSLATION);
    icp->setMovingMeshPort(importerA->getOutputPort());
    icp->setFixedMeshPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<Mesh>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation()->getTransform().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(0));
    CHECK(detectedRotation.y() == Approx(0));
    CHECK(detectedRotation.z() == Approx(0));
}

TEST_CASE("ICP on two point sets", "[fast][IterativeClosestPoint][icp]") {

    Vector3f translation(0.01, 0, 0.01);
    Vector3f rotation(0.5, 0, 0);

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    importerB->update();
    Mesh::pointer B = importerB->getOutputData<Mesh>(0);
    AffineTransformation::pointer T = AffineTransformation::New();
    T->setTransform(transform);
    B->getSceneGraphNode()->setTransformation(T);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingMeshPort(importerA->getOutputPort());
    icp->setFixedMeshPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<Mesh>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation()->getTransform().translation();

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

    VTKMeshFileImporter::pointer importerA = VTKMeshFileImporter::New();
    importerA->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
    VTKMeshFileImporter::pointer importerB = VTKMeshFileImporter::New();
    importerB->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");

    importerA->update();
    importerB->update();
    Mesh::pointer A = importerA->getOutputData<Mesh>();
    Mesh::pointer B = importerB->getOutputData<Mesh>();

    AffineTransformation::pointer FASTtransformInit = AffineTransformation::New();
    SceneGraph::insertParentNodeToData(A, FASTtransformInit);
    SceneGraph::insertParentNodeToData(B, FASTtransformInit);

    // Apply a transformation to B surface
    Affine3f transform = Affine3f::Identity();
    transform.translate(translation);
    Matrix3f R;
    R = Eigen::AngleAxisf(rotation.x(), Vector3f::UnitX())
    * Eigen::AngleAxisf(rotation.y(), Vector3f::UnitY())
    * Eigen::AngleAxisf(rotation.z(), Vector3f::UnitZ());
    transform.rotate(R);
    AffineTransformation::pointer T = AffineTransformation::New();
    T->setTransform(transform);
    B->getSceneGraphNode()->setTransformation(T);

    // Do ICP registration
    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setMovingMeshPort(importerA->getOutputPort());
    icp->setFixedMeshPort(importerB->getOutputPort());
    icp->update();

    // Validate result
    importerA->getStaticOutputData<Mesh>(0)->getSceneGraphNode()->setTransformation(icp->getOutputTransformation());
    Vector3f detectedRotation = icp->getOutputTransformation()->getEulerAngles();
    Vector3f detectedTranslation = icp->getOutputTransformation()->getTransform().translation();

    CHECK(detectedTranslation.x() == Approx(translation.x()));
    CHECK(detectedTranslation.y() == Approx(translation.y()));
    CHECK(detectedTranslation.z() == Approx(translation.z()));
    CHECK(detectedRotation.x() == Approx(rotation.x()));
    CHECK(detectedRotation.y() == Approx(rotation.y()));
    CHECK(detectedRotation.z() == Approx(rotation.z()));
}



} // end namespace fast
