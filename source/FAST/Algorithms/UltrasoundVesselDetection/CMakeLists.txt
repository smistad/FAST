if(FAST_MODULE_DeepLearning)
	fast_add_sources(
		UltrasoundVesselDetection.cpp
		UltrasoundVesselDetection.hpp
		VesselCrossSection.cpp
		VesselCrossSection.hpp
		VesselCrossSectionAccess.cpp
		VesselCrossSectionAccess.hpp
	)
	fast_add_test_sources(
		Tests.cpp
	)
	fast_add_example(extractVesselRegionProposals extractVesselRegionProposals.cpp)
endif()