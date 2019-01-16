#include "GUI.hpp"
#include <FAST/Algorithms/NeuralNetwork/ImageToImageNetwork.hpp>
#include <FAST/Streamers/IGTLinkStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>
#include <FAST/Algorithms/ImageResizer/ImageResizer.hpp>
#include <QHBoxLayout>

namespace fast {

GUI::GUI() {

    IGTLinkStreamer::pointer streamer = IGTLinkStreamer::New();

    setWidth(getScreenWidth());
    setHeight(getScreenHeight());
    enableMaximized();
    setTitle("FAST Neural Network Despeckling");

    auto resizer = ImageResizer::New();
    resizer->setInputConnection(streamer->getOutputPort());
    resizer->setWidth(512);
    resizer->setHeight(512);

    UltrasoundImageEnhancement::pointer enhancer1 = UltrasoundImageEnhancement::New();
    enhancer1->setInputConnection(resizer->getOutputPort());
    ImageRenderer::pointer renderer1 = ImageRenderer::New();
    renderer1->addInputConnection(enhancer1->getOutputPort());
    View* viewOrig = createView();
    viewOrig->setBackgroundColor(Color::Black());
    viewOrig->set2DMode();
    viewOrig->addRenderer(renderer1);


    ImageToImageNetwork::pointer network = ImageToImageNetwork::New();
    network->load("/home/smistad/Downloads/filname_test.pb");
    network->addOutputNode(0, "activation_8/Tanh:0", NodeType::TENSOR);
    network->setSignedInputNormalization(true);
    network->setScaleFactor(1.0f/255.0f);
    network->setInputConnection(streamer->getOutputPort());

    UltrasoundImageEnhancement::pointer enhancer2 = UltrasoundImageEnhancement::New();
    enhancer2->setInputConnection(network->getOutputPort());
    ImageRenderer::pointer renderer2 = ImageRenderer::New();
    renderer2->addInputConnection(enhancer2->getOutputPort());
    View* view = createView();
    view->setBackgroundColor(Color::Black());
    view->set2DMode();
    view->addRenderer(renderer2);


    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(viewOrig);
    layout->addWidget(view);
    mWidget->setLayout(layout);
}

}