fast_add_sources(
    NonLocalMeans.cpp
    NonLocalMeans.hpp
)
fast_add_test_sources(
    NonLocalMeansTests.cpp
)
fast_add_process_object(NonLocalMeans NonLocalMeans.hpp)
