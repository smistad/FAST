#include "FAST/Tests/catch.hpp"
#include "FAST/DeviceManager.hpp"
#include "NonLocalMeans.hpp"
#include "NonLocalMeansNew.hpp"
#include <FAST/Streamers/ImageFileStreamer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/DualViewWindow.hpp>
#include <FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp>

namespace fast{

    TEST_CASE("New non local means", "[fast][mnlm]") {
        auto streamer = ImageFileStreamer::New();
        streamer->setFilenameFormat(Config::getTestDataPath() + "US/Heart/ApicalFourChamber/US-2D_#.mhd");
        streamer->enableLooping();

        auto filter = NonLocalMeansNew::New();
        filter->setInputConnection(streamer->getOutputPort());

        auto enhance = UltrasoundImageEnhancement::New();
        enhance->setInputConnection(filter->getOutputPort());

        auto renderer = ImageRenderer::New();
        renderer->addInputConnection(enhance->getOutputPort());

        auto enhance2 = UltrasoundImageEnhancement::New();
        enhance2->setInputConnection(streamer->getOutputPort());

        auto renderer2 = ImageRenderer::New();
        renderer2->addInputConnection(enhance2->getOutputPort());

        auto window = DualViewWindow::New();
        window->addRendererToBottomRightView(renderer2);
        window->addRendererToTopLeftView(renderer);
        window->getView(0)->set2DMode();
        window->getView(1)->set2DMode();
        window->start();

    }

    TEST_CASE("No input given to NonLocalMeans throws exception", "[fast][NonLocalMeans]") {
        NonLocalMeans::pointer filter = NonLocalMeans::New();
        CHECK_THROWS(filter->update());
    }

    TEST_CASE("Negative or zero sigma input throws exception in NonLocalMeans", "[fast][NonLocalMeans]") {
        NonLocalMeans::pointer filter = NonLocalMeans::New();

        CHECK_THROWS(filter->setWindowSize(-4));
        CHECK_THROWS(filter->setWindowSize(0));
        CHECK_THROWS(filter->setGroupSize(-4));
        CHECK_THROWS(filter->setGroupSize(0));
        CHECK_THROWS(filter->setK(-4));
        CHECK_THROWS(filter->setEuclid(-4));
        CHECK_THROWS(filter->setSigma(-4));
        CHECK_THROWS(filter->setSigma(0));
        CHECK_THROWS(filter->setDenoiseStrength(-4));
        CHECK_THROWS(filter->setDenoiseStrength(0));
    }

    TEST_CASE("Even input as window size or group size throws exception in NonLocalMeans", "[fast][NonLocalMeans]") {
       NonLocalMeans::pointer filter = NonLocalMeans::New();

        CHECK_THROWS(filter->setWindowSize(2));
        CHECK_THROWS(filter->setGroupSize(2));
    }
}
