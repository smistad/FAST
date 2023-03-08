fast_add_sources(
        IntensityNormalization.cpp
        IntensityNormalization.hpp
        ZeroMeanUnitVariance.cpp
        ZeroMeanUnitVariance.hpp
)
fast_add_process_object(IntensityNormalization IntensityNormalization.hpp)
fast_add_process_object(ZeroMeanUnitVariance ZeroMeanUnitVariance.hpp)
fast_add_test_sources(
    IntensityNormalizationTests.cpp
)
