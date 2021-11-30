fast_add_sources(
    EnvelopeAndLogCompressor.cpp
    EnvelopeAndLogCompressor.hpp
    ScanConverter.cpp
    ScanConverter.hpp
)
fast_add_process_object(EnvelopeAndLogCompressor EnvelopeAndLogCompressor.hpp)
fast_add_process_object(ScanConverter ScanConverter.hpp)
