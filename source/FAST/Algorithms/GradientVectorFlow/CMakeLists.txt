fast_add_sources(
    EulerGradientVectorFlow.cpp
    EulerGradientVectorFlow.hpp
    MultigridGradientVectorFlow.cpp
    MultigridGradientVectorFlow.hpp
)
fast_add_test_sources(
    GradientVectorFlowTests.cpp
)
fast_add_process_object(ALGORITHMS EulerGradientVectorFlow EulerGradientVectorFlow.hpp)
fast_add_process_object(ALGORITHMS MultigridGradientVectorFlow MultigridGradientVectorFlow.hpp)
