After the framework has been compiled you can download the test data and run the unit and system tests to make sure the framework is working properly on your system. Note that the CMake option FAST_BUILD_TESTS must be set for the tests to be compiled.

Download and install test data
----------------------------------------------------------
To be able to run all the tests you must download the public test data from:

[http://www.idi.ntnu.no/~smistad/FAST_Test_Data.zip](http://www.idi.ntnu.no/~smistad/FAST_Test_Data.zip)

CMake will download the test data automatically on configure unless FAST_DOWNLOAD_TEST_DATA is set to OFF/FALSE.

Running the tests
-----------------------------------------------------------------------
The test framework used is called Catch and here are some examples of how to use it:

```bash
# Run all tests
./bin/testFAST

# See a list of all tests available
./bin/testFAST -l

# Run tests with a specific tag
./bin/testFAST [SliceRenderer]
```