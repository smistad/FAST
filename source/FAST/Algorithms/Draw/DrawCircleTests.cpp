#include <FAST/Testing.hpp>
#include "DrawCircle.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Visualization/ImageRenderer/ImageRenderer.hpp>
#include <FAST/Visualization/SimpleWindow.hpp>

using namespace fast;

TEST_CASE("Draw single circle", "[fast][DrawCircle][visual]") {
    auto image = Image::create(256, 256, TYPE_UINT8, 1);
    image->fill(0);

    auto draw = DrawCircle::create({Vector2f(128, 128)}, {64})
            ->connect(image);

    auto renderer = ImageRenderer::create(0.5, 1)->connect(draw);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Draw single color circle", "[fast][DrawCircle][visual]") {
    auto image = Image::create(256, 256, TYPE_UINT8, 3);
    image->fill(0);

    auto draw = DrawCircle::create({Vector2f(128, 128)}, {64}, 0, Color::Red())
            ->connect(image);

    auto renderer = ImageRenderer::create()->connect(draw);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Draw single unfilled circle", "[fast][DrawCircle][visual]") {
    auto image = Image::create(256, 256, TYPE_UINT8, 1);
    image->fill(0);

    auto draw = DrawCircle::create({Vector2f(128, 128)}, {64}, 1, Color::Null(), false)
            ->connect(image);

    auto renderer = ImageRenderer::create(0.5, 1)->connect(draw);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Draw single circle with anisotropic spacing", "[fast][DrawCircle][visual]") {
    auto image = Image::create(128, 256, TYPE_UINT8, 1);
    image->fill(0);
    image->setSpacing(2, 1, 1);

    auto draw = DrawCircle::create({Vector2f(128, 128)}, {64}, 1, Color::Null(), true, false)
            ->connect(image);

    auto renderer = ImageRenderer::create(0.5, 1)->connect(draw);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}

TEST_CASE("Draw single unfilled circle with anisotropic spacing", "[fast][DrawCircle][visual]") {
    auto image = Image::create(128, 256, TYPE_UINT8, 1);
    image->fill(0);
    image->setSpacing(2, 1, 1);

    auto draw = DrawCircle::create({Vector2f(128, 128)}, {64}, 1, Color::Null(), false, false)
            ->connect(image);

    auto renderer = ImageRenderer::create(0.5, 1)->connect(draw);
    auto window = SimpleWindow2D::create()->connect(renderer);
    window->setTimeout(1000);
    window->run();
}
