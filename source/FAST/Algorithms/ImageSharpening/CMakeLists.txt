fast_add_sources(
    ImageSharpening.hpp
    ImageSharpening.cpp
)
if(FAST_MODULE_WholeSlideImaging)
    fast_add_test_sources(Tests.cpp)
endif()