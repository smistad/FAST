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
fast_add_python_interfaces(VolumeRenderer.hpp)
fast_add_python_shared_pointers(VolumeRenderer)
fast_add_process_object(AlphaBlendingVolumeRenderer AlphaBlendingVolumeRenderer.hpp)
fast_add_process_object(ThresholdVolumeRenderer ThresholdVolumeRenderer.hpp)
fast_add_process_object(MaximumIntensityProjection MaximumIntensityProjection.hpp)
