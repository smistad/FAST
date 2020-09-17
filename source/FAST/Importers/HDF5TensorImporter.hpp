#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT HDF5TensorImporter : public ProcessObject {
	FAST_OBJECT(HDF5TensorImporter)
	public:
		void setFilename(std::string filename);
		void setDatasetName(std::string datasetName);
		void loadAttributes() override;
	private:
		HDF5TensorImporter();
		void execute() override;

		std::string m_filename = "";
		std::string m_datasetName = "tensor";

};

}