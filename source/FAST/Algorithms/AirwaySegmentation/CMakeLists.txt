fast_add_sources(
	AirwaySegmentation.cpp
	AirwaySegmentation.hpp
)
fast_add_process_object(AirwaySegmentation AirwaySegmentation.hpp)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
	airwaySegmentationTests.cpp
)
endif()