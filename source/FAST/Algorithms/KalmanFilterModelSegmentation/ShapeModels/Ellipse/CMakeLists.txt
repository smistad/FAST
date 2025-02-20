fast_add_sources(
	EllipseModel.cpp
	EllipseModel.hpp
)
fast_add_python_interfaces(ALGORITHMS EllipseModel.hpp)
fast_add_python_shared_pointers(ALGORITHMS EllipseModel)