#pragma once

#include <FAST/Importers/FileImporter.hpp>

namespace fast {

/**
 * @brief Read tensor data stored in HDF5 format.
 *
 * This importer uses the HDF5 C++ library to load Tensor (N-D array) data from disk.
 *
 * @ingroup importers
 * @sa HDF5TensorExporter
 */
class FAST_EXPORT HDF5TensorImporter : public FileImporter {
	FAST_PROCESS_OBJECT(HDF5TensorImporter)
	public:
        /**
         * @brief Create instance
         * @param filename HDF5 file to read
         * @param datasetName Name of dataset in HDF5 file to import
         * @return instance
         */
        FAST_CONSTRUCTOR(HDF5TensorImporter,
                         std::string, filename,,
                         std::string, datasetName, = "tensor"
        );
		void setDatasetName(std::string datasetName);
		void loadAttributes() override;
	private:
		HDF5TensorImporter();
		void execute() override;

		std::string m_datasetName = "tensor";

};

}