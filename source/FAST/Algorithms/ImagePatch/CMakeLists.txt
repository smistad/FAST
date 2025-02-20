if(FAST_MODULE_WholeSlideImaging)
fast_add_sources(
        PatchGenerator.cpp
        PatchGenerator.hpp
        ImageToBatchGenerator.cpp
        ImageToBatchGenerator.hpp
        PatchStitcher.cpp
        PatchStitcher.hpp
)
fast_add_test_sources(
        Tests.cpp
)
fast_add_process_object(ALGORITHMS PatchGenerator PatchGenerator.hpp)
fast_add_process_object(ALGORITHMS PatchStitcher PatchStitcher.hpp)
fast_add_process_object(ALGORITHMS ImageToBatchGenerator ImageToBatchGenerator.hpp)
endif()