#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

class FAST_EXPORT HDF5TensorExporter : public ProcessObject {
	FAST_OBJECT(HDF5TensorExporter)
	public:
		void setFilename(std::string name);
		void setDatasetName(std::string name);
	private:
		HDF5TensorExporter();
		void execute() override;

		std::string m_filename = "";
		std::string m_datasetName = "tensor";
};

}
