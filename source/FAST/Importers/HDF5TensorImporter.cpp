#include "HDF5TensorImporter.hpp"
#include <FAST/Data/Tensor.hpp>
#define H5_BUILT_AS_DYNAMIC_LIB
#include <H5Cpp.h>

namespace fast {

void HDF5TensorImporter::setFilename(std::string filename) {
	m_filename = filename;
	setModified(true);
}

void HDF5TensorImporter::setDatasetName(std::string name) {
	m_datasetName = name;
	setModified(true);
}

void HDF5TensorImporter::loadAttributes() {
	setFilename(getStringAttribute("filename"));
	setDatasetName(getStringAttribute("name"));
}

HDF5TensorImporter::HDF5TensorImporter() {
	createOutputPort<Tensor>(0);

	createStringAttribute("filename", "Filename", "Path to file to open", "");
	createStringAttribute("name", "Dataset name", "Name of dataset tensor to open", m_datasetName);
}

void HDF5TensorImporter::execute() {
	if(m_filename.empty())
		throw Exception("HDF5TensorImporter needs a filename to be set.");
	if(m_datasetName.empty())
		throw Exception("HDF5TensorImporter needs a dataset name to be set.");

	auto tensor = Tensor::New();

	// Open file
	H5::H5File file(m_filename.c_str(), H5F_ACC_RDONLY);

	auto dataset = file.openDataSet(m_datasetName.c_str());
	auto dataspace = dataset.getSpace();
	hsize_t dims_out[16];
	int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);

	TensorShape shape;
	for(int i = 0; i < ndims; ++i)
		shape.addDimension(dims_out[i]);

	auto data = std::make_unique<float[]>(shape.getTotalSize());
	dataset.read(data.get(), H5::PredType::NATIVE_FLOAT, dataspace, dataspace);
	file.close();
	tensor->create(std::move(data), shape);
	addOutputData(0, tensor);
}

}