fast_add_sources(
    EulerGradientVectorFlow.cpp
    EulerGradientVectorFlow.hpp
    MultigridGradientVectorFlow.cpp
    MultigridGradientVectorFlow.hpp
)
fast_add_test_sources(
    GradientVectorFlowTests.cpp
)
fast_add_process_object(EulerGradientVectorFlow EulerGradientVectorFlow.hpp)
fast_add_process_object(MultigridGradientVectorFlow MultigridGradientVectorFlow.hpp)
