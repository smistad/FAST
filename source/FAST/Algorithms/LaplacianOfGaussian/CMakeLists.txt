fast_add_sources(
    LaplacianOfGaussian.cpp
    LaplacianOfGaussian.hpp
)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
    LaplacianOfGaussianTests.cpp
)
endif()
fast_add_process_object(LaplacianOfGaussian LaplacianOfGaussian.hpp)
