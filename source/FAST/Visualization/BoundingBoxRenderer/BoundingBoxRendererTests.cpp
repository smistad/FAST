#include "FAST/Testing.hpp"
#include "BoundingBoxRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("BoundingBox renderer", "[BoundingBoxRenderer][fast][visual]") {
	auto bbset = BoundingBoxSet::create();
	{
		auto access = bbset->getAccess(ACCESS_READ_WRITE);
		access->addBoundingBox({2.0f, 2.0f}, {10.f, 3.0f}, 1, 1);
		access->addBoundingBox({6.0f, 1.0f}, {3.f, 5.0f}, 1, 1);
	}

	auto bbset2 = BoundingBoxSet::create();
	{
		auto access = bbset2->getAccess(ACCESS_READ_WRITE);
		access->addBoundingBox({1.0f, 1.0f}, {3.0f, 3.0f}, 1, 1);
	}

	auto accumulate = BoundingBoxSetAccumulator::New();
	accumulate->setInputData(bbset);
	auto result = accumulate->updateAndGetOutputData<BoundingBoxSet>();
	accumulate->setInputData(bbset2);
	accumulate->update();

	auto renderer = BoundingBoxRenderer::New();
	renderer->addInputData(result);
	//renderer->addInputConnection(accumulate->getOutputPort());

	auto window = SimpleWindow::New();
	window->addRenderer(renderer);
	window->setTimeout(1000);
	window->set2DMode();
	window->start();
}
