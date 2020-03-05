fast_add_sources(
    SegmentationRenderer.cpp
    SegmentationRenderer.hpp
)
fast_add_process_object(SegmentationRenderer SegmentationRenderer.hpp)
fast_add_test_sources(
    SegmentationRendererTests.cpp
)