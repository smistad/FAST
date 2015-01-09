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

TEST_CASE("Calling update on modified process object does execute first time, second time it does not", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->setIsModified();
    process->update();
    CHECK(process->hasExecuted() == true);
    process->setHasExecuted(false);
    process->update();
    CHECK(process->hasExecuted() == false);
}

TEST_CASE("Calling update on a PO with a modified parent PO executes both POs", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->setIsModified();
    DummyProcessObject::pointer process2 = DummyProcessObject::New();
    process2->setInputConnection(process->getOutputPort());
    process2->update();
    CHECK(process->hasExecuted() == true);
    CHECK(process2->hasExecuted() == true);
}

TEST_CASE("Calling update on a PO with a unmodified parent PO does not execute any POs", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    DummyProcessObject::pointer process2 = DummyProcessObject::New();
    process2->setInputConnection(process->getOutputPort());
    process2->update();
    CHECK(process->hasExecuted() == false);
    CHECK(process2->hasExecuted() == false);
}

TEST_CASE("Calling update on a PO with two connections, one modified, one unmodified", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    DummyProcessObject::pointer process2 = DummyProcessObject::New();
    process2->setIsModified();
    DummyProcessObject::pointer process3 = DummyProcessObject::New();
    process3->setInputConnection(process->getOutputPort());
    process3->setInputConnection(process2->getOutputPort());
    process3->update();
    CHECK(process->hasExecuted() == false);
    CHECK(process2->hasExecuted() == true);
    CHECK(process3->hasExecuted() == true);
}
