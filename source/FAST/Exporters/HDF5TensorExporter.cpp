#include "HDF5TensorExporter.hpp"
#include <FAST/Data/Tensor.hpp>
#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>

namespace fast {

void HDF5TensorExporter::setFilename(std::string filename) {
	m_filename = filename;
	setModified(true);
}

void HDF5TensorExporter::setDatasetName(std::string name) {
	m_datasetName = name;
	setModified(true);
}

void HDF5TensorExporter::loadAttributes() {
	setFilename(getStringAttribute("filename"));
	setDatasetName(getStringAttribute("name"));
}

HDF5TensorExporter::HDF5TensorExporter() {
	createInputPort<Tensor>(0);

	createStringAttribute("filename", "Filename", "Path to file to export to", "");
	createStringAttribute("name", "Dataset name", "Name of dataset tensor to write", m_datasetName);
}

void HDF5TensorExporter::execute() {
	if(m_filename.empty())
		throw Exception("HDF5TensorExporter needs a filename to be set.");

	auto tensor = getInputData<Tensor>();

	auto shape = tensor->getShape();
	if(shape.getUnknownDimensions() > 0)
		throw Exception("Tensor has unknown dimensions");

	// Open file
	H5::H5File file(m_filename.c_str(), H5F_ACC_TRUNC);

	std::vector<hsize_t> h5shape;
	for(int i = 0; i < shape.getDimensions(); ++i)
		h5shape.push_back(shape[i]);
	H5::DataSpace memspace(shape.getDimensions(), h5shape.data());
	auto tensorAccess = tensor->getAccess(ACCESS_READ);
	auto dataset = file.createDataSet(m_datasetName.c_str(), H5::PredType::NATIVE_FLOAT, memspace);
	dataset.write(tensorAccess->getRawData(), H5::PredType::NATIVE_FLOAT, memspace, memspace);
	file.close();
}

}