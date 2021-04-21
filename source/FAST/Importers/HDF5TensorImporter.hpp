#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Read tensor data stored in HDF5 format.
 *
 * This importer uses the HDF5 C++ library to load Tensor (N-D array) data from disk.
 *
 * @ingroup importers
 * @sa HDF5TensorExporter
 */
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