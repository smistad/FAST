if(FAST_MODULE_NeuralNetwork)
	fast_add_sources(
		UltrasoundVesselDetection.cpp
		UltrasoundVesselDetection.hpp
		UltrasoundVesselSegmentation.cpp
		UltrasoundVesselSegmentation.hpp
		VesselCrossSection.cpp
		VesselCrossSection.hpp
		VesselCrossSectionAccess.cpp
		VesselCrossSectionAccess.hpp
	)
	fast_add_test_sources(
		Tests.cpp
	)
	fast_add_example(extractVesselRegionProposals extractVesselRegionProposals.cpp)
	fast_add_example(ultrasoundVesselDetection ultrasoundVesselDetectionExample.cpp)
	fast_add_example(ultrasoundVesselSegmentation ultrasoundVesselSegmentationExample.cpp)
endif()
