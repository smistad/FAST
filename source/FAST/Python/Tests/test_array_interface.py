import fast
import numpy as np
import pytest


def test_2D_image_array_interface():
    types_to_test = [
        (np.uint8, fast.TYPE_UINT8, 255),
        (np.float32, fast.TYPE_FLOAT, 1),
        (np.uint16, fast.TYPE_UINT16, 128),
        (np.int16, fast.TYPE_INT16, 128)
    ]
    width = 64
    height = 37

    for np_type, fast_type, scale in types_to_test:
        # Test no channels specified
        data = (np.random.rand(height, width)*scale).astype(np_type)
        image = fast.Image.createFromArray(data)

        assert image.getWidth() == width
        assert image.getHeight() == height
        assert image.getNrOfChannels() == 1
        assert image.getDataType() == fast_type
        assert np.array_equal(np.asarray(image), data.reshape((height, width, 1)))

        for channels in range(1, 5):
            data = (np.random.rand(height, width, channels)*scale).astype(np_type)
            image = fast.Image.createFromArray(data)

            assert image.getWidth() == width
            assert image.getHeight() == height
            assert image.getNrOfChannels() == channels
            assert image.getDataType() == fast_type
            assert np.array_equal(np.asarray(image), data)


def test_3D_image_array_interface():
    types_to_test = [
        (np.uint8, fast.TYPE_UINT8, 255),
        (np.float32, fast.TYPE_FLOAT, 1),
        (np.uint16, fast.TYPE_UINT16, 128),
        (np.int16, fast.TYPE_INT16, 128)
    ]
    width = 64
    height = 37
    depth = 89

    for np_type, fast_type, scale in types_to_test:
        # Test no channels specified
        data = (np.random.rand(depth, height, width)*scale).astype(np_type)
        image = fast.Image.createFromArray(data)

        assert image.getWidth() == width
        assert image.getHeight() == height
        assert image.getDepth() == depth
        assert image.getNrOfChannels() == 1
        assert image.getDataType() == fast_type
        assert np.array_equal(np.asarray(image), data.reshape((depth, height, width, 1)))

        for channels in range(1, 5):
            data = (np.random.rand(depth, height, width, channels)*scale).astype(np_type)
            image = fast.Image.createFromArray(data)

            assert image.getWidth() == width
            assert image.getHeight() == height
            assert image.getDepth() == depth
            assert image.getNrOfChannels() == channels
            assert image.getDataType() == fast_type
            assert np.array_equal(np.asarray(image), data)

def test_image_array_interface_exceptions():
    data = ''
    with pytest.raises(ValueError):
        fast.Image.createFromArray(data)
    data = np.ndarray((16,), dtype=np.uint8)
    with pytest.raises(ValueError):
        fast.Image.createFromArray(data)
    data = np.ndarray((16,34,23,54,23), dtype=np.uint8)
    with pytest.raises(ValueError):
        fast.Image.createFromArray(data)
