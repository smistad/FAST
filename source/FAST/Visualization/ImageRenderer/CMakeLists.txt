fast_add_sources(
    ImageRenderer.cpp
    ImageRenderer.hpp
)
fast_add_process_object(ImageRenderer ImageRenderer.hpp)
fast_add_test_sources(
    ImageRendererTests.cpp
)
fast_add_python_interfaces(
	ImageRenderer.i
)