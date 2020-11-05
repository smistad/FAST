fast_add_subdirectories(
    #OpenIGTLinkClient
    OpenIGTLinkServer
    Pipeline
    UFFViewer
)
fast_add_sources(
    CommandLineParser.cpp
    CommandLineParser.hpp
)
fast_add_tool(downloadTestData downloadTestData.cpp)