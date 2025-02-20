fast_add_sources(
	ImageSlicer.cpp
	ImageSlicer.hpp
)
fast_add_process_object(ALGORITHMS ImageSlicer ImageSlicer.hpp)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
	ImageSlicerTests.cpp
)
endif()