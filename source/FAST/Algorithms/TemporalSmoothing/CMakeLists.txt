fast_add_sources(
    ImageMovingAverage.cpp
    ImageMovingAverage.hpp
    ImageWeightedMovingAverage.cpp 
    ImageWeightedMovingAverage.hpp
)
fast_add_test_sources(Tests.cpp)
fast_add_process_object(ALGORITHMS ImageWeightedMovingAverage ImageWeightedMovingAverage.hpp)
fast_add_process_object(ALGORITHMS ImageMovingAverage ImageMovingAverage.hpp)