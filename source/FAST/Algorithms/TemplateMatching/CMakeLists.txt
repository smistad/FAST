fast_add_sources(
        TemplateMatching.hpp
        TemplateMatching.cpp
)
fast_add_process_object(TemplateMatching TemplateMatching.hpp)
fast_add_test_sources(Tests.cpp)