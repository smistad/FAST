import platform
import fast
import sys


def test_inference_engine_presence():
    """ Test that all inference engines are loaded successfully """
    # This will load all possible inference engines:
    list = fast.InferenceEngineManager.getEngineList()
    if sys.platform == 'win32':
        # Windows
        assert 'ONNXRuntime' in list
        assert 'OpenVINO' in list
    elif sys.platform == 'darwin':
        # macOS
        assert 'ONNXRuntime' in list
        arch = platform.machine()
        if arch == 'x86_64':
            assert 'OpenVINO' in list
    else:
        # Linux
        assert 'ONNXRuntime' in list
        assert 'OpenVINO' in list


def test_inference_engine_tensorflow_download():
    """ TensorFlow is not bundled with pyfast, test that the automatic download works"""
    #if not (sys.platform == 'darwin' and platform.machine() == 'arm64'):
    if not sys.platform == 'darwin':
        engine = fast.InferenceEngineManager.loadEngine('TensorFlow')
        list = fast.InferenceEngineManager.getEngineList()
        assert 'TensorFlow' in list


def segmentation_inference(engine: str, extension: str):
    importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_100.mhd')

    segmentationNetwork = fast.SegmentationNetwork.create(
        fast.Config.getTestDataPath() + 'NeuralNetworkModels/jugular_vein_segmentation' + extension,
        scaleFactor=1./255.,
        inferenceEngine=engine
    ).connect(importer)

    assert segmentationNetwork.getInferenceEngine().getName() == engine

    segmentation = segmentationNetwork.runAndGetOutputData()
    assert segmentation.getWidth() == 256
    assert segmentation.getHeight() == 256
    assert segmentation.calculateSumIntensity() > 0


def test_onnxruntime_inference():
    """ Test if ONNX Runtime is able to do inference """
    segmentation_inference('ONNXRuntime', '.onnx')


def test_openvino_onnx_inference():
    """ Test if OpenVINO is able to do inference on ONNX model """
    if sys.platform == 'darwin' and platform.machine() == 'arm64':
        # OpenVINO not available on macOS arm64
        return

    segmentation_inference('OpenVINO', '.onnx')


def test_openvino_ir_inference():
    """ Test if OpenVINO is able to do inference on IR model """
    if sys.platform == 'darwin' and platform.machine() == 'arm64':
        # OpenVINO not available on macOS arm64
        return

    segmentation_inference('OpenVINO', '.xml')


def test_tensorflow_inference():
    """ Test if TensorFlow is able to do inference on protobuf model """
    if sys.platform == 'darwin' and platform.machine() == 'arm64':
        # TensorFlow not available on macOS
        return

    segmentation_inference('TensorFlow', '.pb')


def test_tensorrt_inference():
    """ Test if TensorRT is able to do inference """
    if 'TensorRT' not in fast.InferenceEngineManager.getEngineList():
        return

    segmentation_inference('TensorRT', '.onnx')
