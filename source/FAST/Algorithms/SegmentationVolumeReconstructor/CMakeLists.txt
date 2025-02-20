fast_add_sources(
        SegmentationVolumeReconstructor.cpp
        SegmentationVolumeReconstructor.hpp
)
fast_add_process_object(ALGORITHMS SegmentationVolumeReconstructor SegmentationVolumeReconstructor.hpp)
fast_add_test_sources(
        Tests.cpp
)
