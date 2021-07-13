fast_add_sources(
        IntensityNormalization.cpp
        IntensityNormalization.hpp
)
fast_add_process_object(IntensityNormalization IntensityNormalization.hpp)
fast_add_test_sources(
    ScaleImageTests.cpp
)
