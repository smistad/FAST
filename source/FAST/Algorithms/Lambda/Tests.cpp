#include <FAST/Testing.hpp>
#include "RunLambda.hpp"
#include <FAST/Data/Text.hpp>

namespace fast {

TEST_CASE("RunLambda test", "[fast][RunLambda]") {
	auto testText = Text::New();
	testText->create("test");
	auto runLambda = RunLambda::New();
	runLambda->setInputData(testText);
	bool hasRun = false;
	runLambda->setLambda([&hasRun]() {
		hasRun = true;
	});

	auto resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == true);
	CHECK(resultData->getText() == "test");
}

TEST_CASE("RunLambda test with input", "[fast][RunLambda]") {
	auto testText = Text::New();
	testText->create("test");
	auto runLambda = RunLambda::New();
	runLambda->setInputData(testText);
	bool hasRun = false;
	runLambda->setLambda([&hasRun](DataObject::pointer data) {
		hasRun = true;
		std::static_pointer_cast<Text>(data)->setText("changed");
	});

	auto resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == true);
	CHECK(resultData->getText() == "changed");
}

TEST_CASE("RunLambda on last frame only", "[fast][RunLambda]") {
	auto testText = Text::New();
	testText->create("test1");
	auto runLambda = RunLambda::New();
	runLambda->setRunOnLastFrameOnly(true);
	runLambda->setInputData(testText);
	bool hasRun = false;
	runLambda->setLambda([&hasRun]() {
		hasRun = true;
	});

	auto resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == false);
	CHECK(resultData->getText() == "test1");

	auto lastText = Text::New();
	lastText->create("test2");
	lastText->setLastFrame("");
	runLambda->setInputData(lastText);

	resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == true);
	CHECK(resultData->getText() == "test2");
}

TEST_CASE("RunLambda on last frame only separate object", "[fast][RunLambda]") {
	auto testText = Text::New();
	testText->create("test1");
	auto runLambda = RunLambdaOnLastFrame::New();
	runLambda->setInputData(testText);
	bool hasRun = false;
	runLambda->setLambda([&hasRun]() {
		hasRun = true;
	});

	auto resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == false);
	CHECK(resultData->getText() == "test1");

	auto lastText = Text::New();
	lastText->create("test2");
	lastText->setLastFrame("");
	runLambda->setInputData(lastText);

	resultData = runLambda->updateAndGetOutputData<Text>();
	CHECK(hasRun == true);
	CHECK(resultData->getText() == "test2");
}



}