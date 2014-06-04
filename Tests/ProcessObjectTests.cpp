#include "catch.hpp"
#include "DummyObjects.hpp"

using namespace fast;

TEST_CASE("Calling update on unmodified process object does not execute", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->update();
    CHECK(process->hasExecuted() == false);
}

TEST_CASE("Calling update on modified process object does execute", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->setIsModified();
    process->update();
    CHECK(process->hasExecuted() == true);
}

TEST_CASE("Calling update on a data object with unmodified process object does not execute parent process", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->setSource(process);
    data->update();
    CHECK(process->hasExecuted() == false);
}

TEST_CASE("Calling update on a data object with modified process object does execute parent process", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->setIsModified();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->setSource(process);
    data->update();
    CHECK(process->hasExecuted() == true);
}

TEST_CASE("Calling update on a process object which has a unmodified parent data object is not executed", "[fast][ProcessObject]") {
    DummyDataObject::pointer data = DummyDataObject::New();
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->addParent(data);
    process->update();
    CHECK(process->hasExecuted() == false);
}

TEST_CASE("Calling update on a process object which has a modified parent data object is executed", "[fast][ProcessObject]") {
    DummyDataObject::pointer data = DummyDataObject::New();
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->addParent(data);
    data->updateModifiedTimestamp();
    process->update();
    CHECK(process->hasExecuted() == true);
}

TEST_CASE("Calling update on a process object which has a modified parent data object an one unmodified parent data object is executed", "[fast][ProcessObject]") {
    DummyDataObject::pointer data = DummyDataObject::New();
    DummyDataObject::pointer data2 = DummyDataObject::New();
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->addParent(data);
    process->addParent(data2);
    data2->updateModifiedTimestamp();
    process->update();
    CHECK(process->hasExecuted() == true);
}
