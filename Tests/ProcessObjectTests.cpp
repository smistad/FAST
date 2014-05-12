#include "catch.hpp"
#include "ProcessObject.hpp"
#include "DataObject.hpp"

using namespace fast;

// Create a dummy class that extends the ProcessObject which is an abstract class
class DummyProcessObject : public ProcessObject {
    FAST_OBJECT(DummyProcessObject)
    public:
        void setIsModified() { mIsModified = true; };
        bool hasExecuted() { return mHasExecuted; };
    private:
        DummyProcessObject() : mHasExecuted(false) {};
        void execute() { mHasExecuted = true; };
        bool mHasExecuted;
};


// Create a dummy class that extends the DataObject which is an abstract class
class DummyDataObject : public DataObject {
    FAST_OBJECT(DummyDataObject)
    private:
        void free(ExecutionDevice::pointer device) {};
        void freeAll() {};
};

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
    data->setParent(process);
    data->update();
    CHECK(process->hasExecuted() == false);
}

TEST_CASE("Calling update on a data object with modified process object does execute parent process", "[fast][ProcessObject]") {
    DummyProcessObject::pointer process = DummyProcessObject::New();
    process->setIsModified();
    DummyDataObject::pointer data = DummyDataObject::New();
    data->setParent(process);
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
