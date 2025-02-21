fast_add_sources(
        VolumeRenderer.cpp
        VolumeRenderer.hpp
        MaximumIntensityProjection.cpp
        MaximumIntensityProjection.hpp
        ThresholdVolumeRenderer.cpp
        ThresholdVolumeRenderer.hpp
        AlphaBlendingVolumeRenderer.cpp
        AlphaBlendingVolumeRenderer.hpp
        TransferFunction.cpp
        TransferFunction.hpp
)
fast_add_test_sources(Tests.cpp)
fast_add_python_interfaces(VISUALIZATION VolumeRenderer.hpp TransferFunction.hpp)
fast_add_python_shared_pointers(VISUALIZATION VolumeRenderer)
fast_add_process_object(VISUALIZATION AlphaBlendingVolumeRenderer AlphaBlendingVolumeRenderer.hpp)
fast_add_process_object(VISUALIZATION ThresholdVolumeRenderer ThresholdVolumeRenderer.hpp)
fast_add_process_object(VISUALIZATION MaximumIntensityProjection MaximumIntensityProjection.hpp)
