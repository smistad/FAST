fast_add_sources(
    VectorFieldRenderer.hpp
    VectorFieldRenderer.cpp
    VectorFieldColorRenderer.hpp
    VectorFieldColorRenderer.cpp
)
fast_add_process_object(CORE VectorFieldRenderer VectorFieldRenderer.hpp)
fast_add_process_object(CORE VectorFieldColorRenderer VectorFieldColorRenderer.hpp)