#pragma once

#include <FAST/ProcessObject.hpp>

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
class FAST_EXPORT HDF5TensorExporter : public ProcessObject {
	FAST_OBJECT(HDF5TensorExporter)
	public:
		void setFilename(std::string name);
		void setDatasetName(std::string name);
		void loadAttributes() override;
	private:
		HDF5TensorExporter();
		void execute() override;

		std::string m_filename = "";
		std::string m_datasetName = "tensor";
};

}
