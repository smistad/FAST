fast_add_sources(
    AddTransformation.cpp
    AddTransformation.hpp
    SetTransformation.cpp
    SetTransformation.hpp
)
fast_add_process_object(AddTransformation AddTransformation.hpp)
fast_add_process_object(SetTransformation SetTransformation.hpp)