#pragma once

#include <FAST/Exporters/FileExporter.hpp>

namespace fast {

/**
 * @brief Write a Tensor to a HDF5 file
 *
 * Uses the HDF5 C++ library to write a Tensor to a HDF5 file
 *
 * <h3>Input ports</h3>
 * - 0: Tensor
 *
 * @ingroup exporters
 * @sa HDF5TensorImporter
 */
class FAST_EXPORT HDF5TensorExporter : public FileExporter {
	FAST_PROCESS_OBJECT(HDF5TensorExporter)
	public:
        /**
         * @brief Create instance
         * @param filename Filename to open
         * @param datasetName Dataset in HDF file to open. Default is "tensor"
         * @return instance
         */
        FAST_CONSTRUCTOR(HDF5TensorExporter,
                         std::string, filename,,
                         std::string, datasetName, = "tensor"
        );
        void setDatasetName(std::string name);
		void loadAttributes() override;
	private:
		HDF5TensorExporter();
		void execute() override;

		std::string m_datasetName = "tensor";
};

}
