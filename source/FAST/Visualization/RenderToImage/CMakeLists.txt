fast_add_sources(
    RenderToImage.cpp
    RenderToImage.hpp
)
fast_add_process_object(RenderToImage RenderToImage.hpp)

fast_add_test_sources(Tests.cpp)