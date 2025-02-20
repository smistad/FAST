fast_add_sources(
    EnvelopeAndLogCompressor.cpp
    EnvelopeAndLogCompressor.hpp
    ScanConverter.cpp
    ScanConverter.hpp
)
fast_add_process_object(ALGORITHMS EnvelopeAndLogCompressor EnvelopeAndLogCompressor.hpp)
fast_add_process_object(ALGORITHMS ScanConverter ScanConverter.hpp)
