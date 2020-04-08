fast_add_sources(
    VectorFieldRenderer.hpp
    VectorFieldRenderer.cpp
    VectorFieldColorRenderer.hpp
    VectorFieldColorRenderer.cpp
)
fast_add_process_object(VectorFieldRenderer VectorFieldRenderer.hpp)
fast_add_process_object(VectorFieldColorRenderer VectorFieldColorRenderer.hpp)