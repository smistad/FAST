fast_add_all_subdirectories()
fast_add_sources(
	KalmanFilter.cpp
	KalmanFilter.hpp
	AppearanceModel.hpp
	ShapeModel.hpp
	Shape.cpp
	Shape.hpp
)
fast_add_test_sources(
	Tests.cpp
)
fast_add_python_interfaces(
	KalmanFilter.i
)
fast_add_example(exportLVmeshes exportLVmeshes.cpp)