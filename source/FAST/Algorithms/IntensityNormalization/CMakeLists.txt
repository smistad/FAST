fast_add_sources(
        IntensityNormalization.cpp
        IntensityNormalization.hpp
        ZeroMeanUnitVariance.cpp
        ZeroMeanUnitVariance.hpp
)
fast_add_process_object(ALGORITHMS IntensityNormalization IntensityNormalization.hpp)
fast_add_process_object(ALGORITHMS ZeroMeanUnitVariance ZeroMeanUnitVariance.hpp)
fast_add_test_sources(
    IntensityNormalizationTests.cpp
)
