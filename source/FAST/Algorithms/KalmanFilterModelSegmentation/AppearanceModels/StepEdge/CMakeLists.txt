fast_add_sources(
	StepEdgeModel.cpp
	StepEdgeModel.hpp
)
fast_add_python_interfaces(ALGORITHMS StepEdgeModel.hpp)
fast_add_python_shared_pointers(ALGORITHMS StepEdgeModel)