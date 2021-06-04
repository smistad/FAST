#include <FAST/Testing.hpp>
#include <FAST/Importers/HDF5TensorImporter.hpp>
#include <FAST/Exporters/HDF5TensorExporter.hpp>
#include <FAST/Data/Tensor.hpp>

using namespace fast;

TEST_CASE("HDF5TensorImporter", "[fast][HDF5][HDF5TensorImporter]") {
	// Export a file first
	TensorShape shape({2, 32, 32, 8});
    auto tensor = Tensor::create(shape);

	auto exporter = HDF5TensorExporter::New();
	exporter->setFilename("tensor.hd5");
	exporter->setInputData(tensor);
	exporter->update();

	auto importer = HDF5TensorImporter::New();
	importer->setFilename("tensor.hd5");
	importer->setDatasetName("tensor");
	auto resultTensor = importer->updateAndGetOutputData<Tensor>();
	auto resultShape = resultTensor->getShape();
	CHECK(resultShape[0] == 2);
	CHECK(resultShape[1] == 32);
	CHECK(resultShape[2] == 32);
	CHECK(resultShape[3] == 8);

}