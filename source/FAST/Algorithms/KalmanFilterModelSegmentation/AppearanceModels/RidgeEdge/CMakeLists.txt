fast_add_sources(
    RidgeEdgeModel.cpp
    RidgeEdgeModel.hpp
)
fast_add_python_interfaces(ALGORITHMS RidgeEdgeModel.hpp)
fast_add_python_shared_pointers(ALGORITHMS RidgeEdgeModel)