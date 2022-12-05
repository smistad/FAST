#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>
#include <ONNXRuntimeExport.hpp>

namespace Ort {
	class Session;
	class Env;
	namespace detail {
		class AllocatedFree;
	}
}


namespace fast {

/**
 * @brief Microsofts ONNX Runtime inference engine with DirectX/ML support
 *
 * This inferene engine is windows only.
 * @ingroup neural-network
*/
class INFERENCEENGINEONNXRUNTIME_EXPORT ONNXRuntimeEngine : public InferenceEngine {
	FAST_OBJECT(ONNXRuntimeEngine)
public:
	void run() override;
	void load() override;
	ImageOrdering getPreferredImageOrdering() const override;
	std::string getName() const override;
	std::vector<ModelFormat> getSupportedModelFormats() const {
		return { ModelFormat::ONNX };
	};
	ModelFormat getPreferredModelFormat() const {
		return ModelFormat::ONNX;
	};
	~ONNXRuntimeEngine() override;
	void setMaxBatchSize(int maxBathSize);
	int getMaxBatchSize() const;
	ONNXRuntimeEngine();
	/**
	 * Get a list of devices available for this inference engine.
	 *
	 * @return vector with info on each device
	 */
	std::vector<InferenceDeviceInfo> getDeviceList() override;
private:
	std::unique_ptr<Ort::Session> m_session;
	std::unique_ptr<Ort::Env> m_env;
};

DEFINE_INFERENCE_ENGINE(ONNXRuntimeEngine, INFERENCEENGINEONNXRUNTIME_EXPORT)

}
