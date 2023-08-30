fast_add_sources(
        ImageResampler.cpp
        ImageResampler.hpp
        IsotropicResampler.cpp
        IsotropicResampler.hpp
)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
    ImageResamplerTests.cpp
    IsotropicResamplerTests.cpp
)
endif()
fast_add_process_object(ImageResampler ImageResampler.hpp)
fast_add_process_object(IsotropicResampler IsotropicResampler.hpp)
