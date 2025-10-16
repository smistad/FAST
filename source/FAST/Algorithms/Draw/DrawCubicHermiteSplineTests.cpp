#include <FAST/Testing.hpp>
#include "DrawCubicHermiteSpline.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>
#include <FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>

using namespace fast;

TEST_CASE("Draw cubic hermite spline", "[fast][draw][DrawCubicHermiteSpline][visual]") {
    auto image = Image::create(256, 256, fast::TYPE_UINT8, 1);
    image->fill(0);

    std::vector<Vector2f> controlPoints = {
            {10, 10},
            {50, 50},
            {100, 70},
            {150, 150},
            {100, 200},
            {70, 90},
            {40, 60},
    };

    auto draw = DrawCubicHermiteSpline::create(controlPoints, DrawCubicHermiteSpline::CloseSpline::Straight)->connect(image);
    auto result = draw->runAndGetOutputData<Image>();

    auto renderer = ImageRenderer::create(0.5, 1)->connect(result);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}


/*
TEST_CASE("Draw cubic hermite spline color and fill", "[fast][draw][DrawCubicHermiteSpline][visual]") {
    auto image = Image::create(256, 256, fast::TYPE_UINT8, 3);
    image->fill(0);

    std::vector<Vector2f> controlPoints = {
            {10, 10},
            {50, 50},
            {100, 70},
            {150, 150},
            {100, 200},
            {70, 90},
            {40, 60},
    };

    auto draw = DrawCubicHermiteSpline::create(controlPoints, DrawCubicHermiteSpline::CloseSpline::Straight, 0, Color::Green(), true)->connect(image);
    auto result = draw->runAndGetOutputData<Image>();

    auto renderer = ImageRenderer::create()->connect(result);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}*/
