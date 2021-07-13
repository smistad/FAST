#include <FAST/Testing.hpp>
#include "RunLambda.hpp"
#include <FAST/Data/Text.hpp>

namespace fast {

TEST_CASE("RunLambda test", "[fast][RunLambda]") {
    bool hasRun = false;
	auto runLambda = RunLambda::create([&hasRun]() {
		hasRun = true;
		return DataList();
	});
	runLambda->run();
	CHECK(hasRun == true);
}

TEST_CASE("RunLambda test with input", "[fast][RunLambda]") {
    bool hasRun = false;
	auto testText = Text::create("test");
	auto runLambda = RunLambda::create([&hasRun](DataObject::pointer data) {
        hasRun = true;
        std::static_pointer_cast<Text>(data)->setText("changed");
        return DataList();
    })->connect(testText);

	runLambda->run();
	CHECK(hasRun == true);
	CHECK(testText->getText() == "changed");
}

TEST_CASE("RunLambda test with input and output", "[fast][RunLambda]") {
    bool hasRun = false;
    auto testText = Text::create("test");
    auto runLambda = RunLambda::create([&hasRun](DataObject::pointer data) {
        hasRun = true;
        auto text = Text::create("test2");
        return DataList(text);
    })->connect(testText);

    auto resultData = runLambda->runAndGetOutputData<Text>();
    CHECK(hasRun == true);
    CHECK(resultData->getText() == "test2");
}

TEST_CASE("RunLambda on last frame only", "[fast][RunLambda]") {
	auto testText = Text::create("test1");
	bool hasRun = false;
    auto runLambda = RunLambda::create([&hasRun](DataObject::pointer data) {
		hasRun = true;
		return DataList();
	})->connect(testText);
    runLambda->setRunOnLastFrameOnly(true);

	runLambda->run();
	CHECK(hasRun == false);

	auto lastText = Text::create("test2");
	lastText->setLastFrame("");
	runLambda->connect(lastText);

	runLambda->run();
	CHECK(hasRun == true);
}

}