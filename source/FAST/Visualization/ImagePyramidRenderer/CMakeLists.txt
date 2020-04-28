if(FAST_MODULE_WholeSlideImaging)
fast_add_sources(
        ImagePyramidRenderer.cpp
        ImagePyramidRenderer.hpp
)
fast_add_process_object(ImagePyramidRenderer ImagePyramidRenderer.hpp)
endif()