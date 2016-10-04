if(FAST_MODULE_DeepLearning)
	fast_add_sources(
		ObjectDetection.cpp
		ObjectDetection.hpp
	)
	fast_add_test_sources(
		Tests.cpp
	)
	fast_add_example(arteryDetection arteryDetection.cpp)
endif()