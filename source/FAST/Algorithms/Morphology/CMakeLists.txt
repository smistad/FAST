fast_add_sources(
        Dilation.cpp
        Dilation.hpp
        Erosion.cpp
        Erosion.hpp
)
fast_add_process_object(ALGORITHMS Dilation Dilation.hpp)
fast_add_process_object(ALGORITHMS Erosion Erosion.hpp)