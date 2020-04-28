if(FAST_MODULE_WholeSlideImaging)
fast_add_sources(
    TissueSegmentation.cpp
    TissueSegmentation.hpp
)
fast_add_process_object(TissueSegmentation TissueSegmentation.hpp)

fast_add_test_sources(Tests.cpp)
endif()