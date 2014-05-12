#include "catch.hpp"
#include "GaussianSmoothingFilter.hpp"

namespace fast {

TEST_CASE("No input given to GaussianSmoothingFilter throws exception", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();
    CHECK_THROWS(filter->update());
}

TEST_CASE("Negative or zero sigma and mask size input throws exception in GaussianSmoothingFilter" , "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();

    CHECK_THROWS(filter->setMaskSize(-4));
    CHECK_THROWS(filter->setMaskSize(0));
    CHECK_THROWS(filter->setStandardDeviation(-4));
    CHECK_THROWS(filter->setStandardDeviation(0));
}

TEST_CASE("Even input as mask size throws exception in GaussianSmoothingFilter", "[fast][GaussianSmoothingFilter]") {
    GaussianSmoothingFilter::pointer filter = GaussianSmoothingFilter::New();

    CHECK_THROWS(filter->setMaskSize(2));
}

} // end namespace fast
