fast_add_sources(
    Skeletonization.cpp
    Skeletonization.hpp
)
fast_add_process_object(ALGORITHMS Skeletonization Skeletonization.hpp)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
    SkeletonizationTests.cpp
)
endif()