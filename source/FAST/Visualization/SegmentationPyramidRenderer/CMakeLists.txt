if(FAST_MODULE_WholeSlideImaging)
fast_add_sources(
        SegmentationPyramidRenderer.cpp
        SegmentationPyramidRenderer.hpp
)
fast_add_process_object(SegmentationPyramidRenderer SegmentationPyramidRenderer.hpp)
endif()