fast_add_sources(
    VertexRenderer.cpp
    VertexRenderer.hpp
)
fast_add_process_object(CORE VertexRenderer VertexRenderer.hpp)
fast_add_test_sources(
    VertexRendererTests.cpp
)
