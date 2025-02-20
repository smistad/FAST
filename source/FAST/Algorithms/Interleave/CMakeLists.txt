fast_add_sources(
        Interleave.cpp
        Interleave.hpp
)
fast_add_test_sources(Tests.cpp)
fast_add_process_object(ALGORITHMS Interleave Interleave.hpp)
fast_add_process_object(ALGORITHMS InterleavePlayback Interleave.hpp)
