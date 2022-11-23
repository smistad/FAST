fast_add_sources(
    Streamer.cpp
    Streamer.hpp
    FileStreamer.cpp
    FileStreamer.hpp
    ImageFileStreamer.cpp
    ImageFileStreamer.hpp
    MeshFileStreamer.cpp
    MeshFileStreamer.hpp
    ManualImageStreamer.cpp
    ManualImageStreamer.hpp
    TransformFileStreamer.cpp
    TransformFileStreamer.hpp
    RandomAccessStreamer.cpp
    RandomAccessStreamer.hpp
)
fast_add_python_interfaces(
    Streamer.hpp
    RandomAccessStreamer.hpp
    FileStreamer.hpp
    MeshFileStreamer.hpp
)
fast_add_python_shared_pointers(Streamer RandomAccessStreamer FileStreamer MeshFileStreamer)
fast_add_process_object(ImageFileStreamer ImageFileStreamer.hpp)
if(FAST_MODULE_OpenIGTLink)
    fast_add_sources(
            OpenIGTLinkStreamer.hpp
            OpenIGTLinkStreamer.cpp
        Tests/DummyIGTLServer.cpp
        Tests/DummyIGTLServer.hpp
    )
    fast_add_process_object(OpenIGTLinkStreamer OpenIGTLinkStreamer.hpp)
    if(FAST_MODULE_Visualization)
    fast_add_test_sources(
            Tests/OpenIGTLinkStreamerTests.cpp
    )
    endif()
endif()
if(FAST_MODULE_Visualization)
    fast_add_sources(
        MovieStreamer.cpp
        MovieStreamer.hpp
        CameraStreamer.cpp
        CameraStreamer.hpp
    )
    fast_add_test_sources(Tests/MovieStreamerTests.cpp)
    fast_add_test_sources(Tests/CameraStreamerTests.cpp)
    fast_add_process_object(MovieStreamer MovieStreamer.hpp)
    fast_add_process_object(CameraStreamer CameraStreamer.hpp)
endif()
if(FAST_MODULE_Kinect)
    fast_add_sources(
        KinectStreamer.cpp
        KinectStreamer.hpp
    )
    fast_add_process_object(KinectStreamer KinectStreamer.hpp)
endif()
if(FAST_MODULE_RealSense)
    fast_add_sources(
        RealSenseStreamer.cpp
        RealSenseStreamer.hpp
    )
    fast_add_process_object(RealSenseStreamer RealSenseStreamer.hpp)
endif()
if(FAST_MODULE_Clarius)
    fast_add_sources(
        ClariusStreamer.cpp
        ClariusStreamer.hpp
    )
    fast_add_process_object(ClariusStreamer ClariusStreamer.hpp)
endif()
if(FAST_MODULE_HDF5)
    fast_add_sources(
        UFFStreamer.cpp
        UFFStreamer.hpp
    )
    fast_add_process_object(UFFStreamer UFFStreamer.hpp)
endif()

fast_add_test_sources(
    Tests/ImageFileStreamerTests.cpp
)