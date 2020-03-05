#pragma once

#include <FAST/Streamers/Streamer.hpp>

namespace fast {

class FAST_EXPORT UFFStreamer : public Streamer {
	FAST_OBJECT(UFFStreamer)
public:
	UFFStreamer();
	void setFilename(std::string filename);
	void execute() override;
	void setLooping(bool loop);
	// Set name of which HDF5 group to stream
	void setName(std::string name);
	void loadAttributes() override;
protected:
	void generateStream() override;
	std::string m_filename;
	std::string m_name;
	bool m_loop;
};
}