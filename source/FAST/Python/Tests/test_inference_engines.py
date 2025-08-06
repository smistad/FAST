import platform

import fast
import sys


def test_inference_engine_presence():
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


def test_inference_engine_tensorflow():
    """ TensorFlow is not bundled with pyfast, test that the automatic download works"""
    if not (sys.platform == 'darwin' and platform.machine() == 'arm64'):
        engine = fast.InferenceEngineManager.loadEngine('TensorFlow')
        list = fast.InferenceEngineManager.getEngineList()
        assert 'TensorFlow' in list
