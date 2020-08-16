fast_add_sources(
        ImageResampler.cpp
        ImageResampler.hpp
)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
        ImageResamplerTests.cpp
)
endif()
fast_add_process_object(ImageResampler ImageResampler.hpp)