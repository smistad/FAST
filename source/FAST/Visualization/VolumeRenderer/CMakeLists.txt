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
fast_add_python_interfaces(CORE VolumeRenderer.hpp TransferFunction.hpp)
fast_add_python_shared_pointers(CORE VolumeRenderer)
fast_add_process_object(CORE AlphaBlendingVolumeRenderer AlphaBlendingVolumeRenderer.hpp)
fast_add_process_object(CORE ThresholdVolumeRenderer ThresholdVolumeRenderer.hpp)
fast_add_process_object(CORE MaximumIntensityProjection MaximumIntensityProjection.hpp)
