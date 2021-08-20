fast_add_sources(
        CoherentPointDrift.cpp
        CoherentPointDrift.hpp
        Rigid.cpp
        Rigid.hpp
        Affine.cpp
        Affine.hpp
)
fast_add_test_sources(Tests.cpp)
fast_add_python_interfaces(CoherentPointDrift.hpp)
fast_add_python_shared_pointers(CoherentPointDrift)
fast_add_process_object(CoherentPointDriftAffine Affine.hpp)
fast_add_process_object(CoherentPointDriftRigid Rigid.hpp)
