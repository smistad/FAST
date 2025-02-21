fast_add_sources(
    BoundingBoxRenderer.hpp
    BoundingBoxRenderer.cpp
)
fast_add_test_sources(
    BoundingBoxRendererTests.cpp
)
fast_add_process_object(VISUALIZATION BoundingBoxRenderer BoundingBoxRenderer.hpp)