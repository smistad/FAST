fast_add_sources(
    ImageSharpening.hpp
    ImageSharpening.cpp
)
fast_add_process_object(ImageSharpening ImageSharpening.hpp)
if(FAST_MODULE_WholeSlideImaging)
    fast_add_test_sources(Tests.cpp)
endif()