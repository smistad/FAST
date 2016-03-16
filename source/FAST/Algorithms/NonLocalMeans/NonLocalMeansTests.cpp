#include "FAST/Tests/catch.hpp"
#include "FAST/DeviceManager.hpp"
#include "NonLocalMeans.hpp"

namespace fast{

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
