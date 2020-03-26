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
fast_add_process_object(AlphaBlendingVolumeRenderer AlphaBlendingVolumeRenderer.hpp)