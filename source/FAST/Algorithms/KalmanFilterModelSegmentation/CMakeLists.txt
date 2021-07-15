fast_add_sources(
	KalmanFilterModelSegmentation.cpp
	KalmanFilterModelSegmentation.hpp
	AppearanceModel.hpp
	ShapeModel.hpp
	Shape.cpp
	Shape.hpp
)
fast_add_python_interfaces(KalmanFilterModelSegmentation.hpp AppearanceModel.hpp ShapeModel.hpp Shape.hpp)
fast_add_python_shared_pointers(KalmanFilterModelSegmentation AppearanceModel ShapeModel Shape)
if(FAST_MODULE_Visualization)
fast_add_test_sources(
	Tests.cpp
)
endif()
#fast_add_example(exportLVmeshes exportLVmeshes.cpp)
#fast_add_example(exportLVmeshes3D exportLVmeshes3D.cpp)
fast_add_all_subdirectories()
