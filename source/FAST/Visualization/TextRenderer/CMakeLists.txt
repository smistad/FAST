fast_add_sources(
    TextRenderer.cpp
    TextRenderer.hpp
)
fast_add_test_sources(Tests.cpp)
fast_add_process_object(CORE TextRenderer TextRenderer.hpp)
