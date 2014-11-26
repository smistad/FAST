#include "SmartPointers.hpp"
#include "Exception.hpp"
#include "ImageImporter.hpp"
#include "ImageExporter.hpp"
#include "DeviceManager.hpp"
#include "GaussianSmoothingFilter.hpp"
#include "SimpleWindow.hpp"
#include "ImageRenderer.hpp"
#include "SliceRenderer.hpp"
#include "VolumeRenderer.hpp"
#include "MeshRenderer.hpp"
#include "MetaImageImporter.hpp"
#include "ImageFileStreamer.hpp"
#include "MetaImageExporter.hpp"
#include "ColorTransferFunction.hpp"
#include "OpacityTransferFunction.hpp"
#include "SurfaceExtraction.hpp"
#include "VTKSurfaceFileImporter.hpp"
#include "VesselDetection.hpp"
#include "IterativeClosestPoint.hpp"
#include "VTKPointSetFileImporter.hpp"
#include "SceneGraph.hpp"
#include "PointRenderer.hpp"
#include "Color.hpp"
#include "ImageFileStreamer.hpp"
#include "MeshRenderer.hpp"

using namespace fast;

int main(int argc, char ** argv) {
    MetaImageImporter::pointer CTimporter = MetaImageImporter::New();
    CTimporter->setFilename(std::string(FAST_TEST_DATA_DIR) + "CT-Abdomen.mhd");
    Image::pointer CTimage = CTimporter->getOutput();

    VTKSurfaceFileImporter::pointer arteryImporter = VTKSurfaceFileImporter::New();
    arteryImporter->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/artery.vtk");
    Mesh::pointer artery = arteryImporter->getOutput();

    VTKSurfaceFileImporter::pointer boneImporter = VTKSurfaceFileImporter::New();
    boneImporter->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/bones.vtk");
    Mesh::pointer bones = boneImporter->getOutput();

    MetaImageImporter::pointer importer1 = MetaImageImporter::New();
    importer1->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_0.mhd");
    Image::pointer image1 = importer1->getOutput();

    MetaImageImporter::pointer importer2 = MetaImageImporter::New();
    importer2->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_40.mhd");
    Image::pointer image2 = importer2->getOutput();

    MetaImageImporter::pointer importer3 = MetaImageImporter::New();
    importer3->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_120.mhd");
    Image::pointer image3 = importer3->getOutput();

    MetaImageImporter::pointer importer4 = MetaImageImporter::New();
    importer4->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_20.mhd");
    Image::pointer image4 = importer4->getOutput();

    MetaImageImporter::pointer importer5 = MetaImageImporter::New();
    importer5->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_60.mhd");
    Image::pointer image5 = importer5->getOutput();

    MetaImageImporter::pointer importer6 = MetaImageImporter::New();
    importer6->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/US-Acq_05_20141021T102120_Sonix_100.mhd");
    Image::pointer image6 = importer6->getOutput();

    SliceRenderer::pointer CTrenderer = SliceRenderer::New();
    CTrenderer->setInput(CTimage);
    CTrenderer->setSlicePlane(PLANE_Z);
    CTrenderer->setSliceToRender(15);
    MeshRenderer::pointer arteryRenderer = MeshRenderer::New();
    arteryRenderer->setInput(artery);
    MeshRenderer::pointer bonesRenderer = MeshRenderer::New();
    bonesRenderer->setInput(bones);
    ImageRenderer::pointer renderer = ImageRenderer::New();
    renderer->setInput(image1);
    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->setInput(image2);
    ImageRenderer::pointer renderer3 = ImageRenderer::New();
    renderer3->setInput(image3);
    ImageRenderer::pointer renderer4 = ImageRenderer::New();
    renderer4->setInput(image4);
    ImageRenderer::pointer renderer5 = ImageRenderer::New();
    renderer5->setInput(image5);
    ImageRenderer::pointer renderer6 = ImageRenderer::New();
    renderer6->setInput(image6);
    SimpleWindow::pointer window = SimpleWindow::New();
    /*
    window->addRenderer(renderer);
    window->addRenderer(renderer2);
    window->addRenderer(renderer3);
    window->addRenderer(renderer4);
    window->addRenderer(renderer5);
    window->addRenderer(renderer6);
    //window->addRenderer(CTrenderer);
    window->addRenderer(arteryRenderer);
    //window->addRenderer(bonesRenderer);
     */

    VesselDetection::pointer vesselDetection = VesselDetection::New();
    vesselDetection->addImage(image1);
    /*
    vesselDetection->addImage(image2);
    vesselDetection->addImage(image3);
    vesselDetection->addImage(image4);
    vesselDetection->addImage(image5);
    vesselDetection->addImage(image6);
    */

    Image::pointer image_circle = vesselDetection->getOutput();
    PointSet::pointer points = vesselDetection->getPointSet();
    /*

    VTKPointSetFileImporter::pointer pointSetImporter = VTKPointSetFileImporter::New();
    pointSetImporter->setFilename("/home/smistad/Dropbox/RASimAs/VesselDetection/artery.vtk");
    PointSet::pointer modelPoints = pointSetImporter->getOutput();

    IterativeClosestPoint::pointer icp = IterativeClosestPoint::New();
    icp->setFixedPointSet(points);
    icp->setMovingPointSet(modelPoints);
    icp->setTransformationType(IterativeClosestPoint::TRANSLATION);
    icp->update();

    PointRenderer::pointer pointRenderer = PointRenderer::New();
    pointRenderer->addInput(points);
    //window->addRenderer(pointRenderer);

    // Get transform and apply it to the model
    artery->update();
    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(artery);
    node->setTransformation(icp->getOutputTransformation());
    //bones->update();
    //SceneGraphNode::pointer node2 = graph.getDataNode(bones);
    //node2->setTransformation(icp->getOutputTransformation());
    //window->runMainLoop();
*/
    // Show vessel detection result
    ImageRenderer::pointer rendererCircle = ImageRenderer::New();
    rendererCircle->setInput(image_circle);
    window->addRenderer(rendererCircle);
    window->runMainLoop();
}
