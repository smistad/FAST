#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "FAST/Reporter.hpp"

int main(int argc, char* argv[]) {
    fast::Reporter::setGlobalReportMethod(fast::Reporter::COUT);
    int result = Catch::Session().run(argc, argv);

    return result;
}
