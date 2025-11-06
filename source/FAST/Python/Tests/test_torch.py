import torch
import fast
import numpy as np
import pytest


def test_torch_tensor_fast_image_2d():
    types_to_test = [
        (torch.uint8, fast.TYPE_UINT8, 255),
        (torch.float32, fast.TYPE_FLOAT, 1),
        (torch.uint16, fast.TYPE_UINT16, 128),
        (torch.int16, fast.TYPE_INT16, 128)
    ]
    width = 64
    height = 37

    for torch_type, fast_type, scale in types_to_test:
        tensor = (torch.rand(height, width, dtype=torch.float32)*scale).to(torch_type)
        image = fast.Image.createFromTensor(tensor)

        assert image.getWidth() == width
        assert image.getHeight() == height
        assert image.getNrOfChannels() == 1
        assert image.getDataType() == fast_type
        assert torch.equal(tensor, torch.tensor(np.asarray(image)).squeeze())

        for channels in range(1, 5):
            tensor = (torch.rand(channels, height, width, dtype=torch.float32)*scale).to(torch_type)
            image = fast.Image.createFromTensor(tensor)

            assert image.getWidth() == width
            assert image.getHeight() == height
            assert image.getNrOfChannels() == channels
            assert image.getDataType() == fast_type
            assert torch.equal(tensor, torch.tensor(np.asarray(image)).permute((2, 0, 1)))


def test_torch_tensor_fast_image_3d():
    types_to_test = [
        (torch.uint8, fast.TYPE_UINT8, 255),
        (torch.float32, fast.TYPE_FLOAT, 1),
        (torch.uint16, fast.TYPE_UINT16, 128),
        (torch.int16, fast.TYPE_INT16, 128)
    ]
    width = 64
    height = 37
    depth = 42

    for torch_type, fast_type, scale in types_to_test:
        tensor = (torch.rand(depth, height, width, dtype=torch.float32)*scale).to(torch_type)
        image = fast.Image.createFromTensor(tensor)

        assert image.getWidth() == width
        assert image.getHeight() == height
        assert image.getDepth() == depth
        assert image.getNrOfChannels() == 1
        assert image.getDataType() == fast_type
        assert torch.equal(tensor, torch.tensor(np.asarray(image)).squeeze())

        for channels in range(1, 5):
            tensor = (torch.rand(channels, depth, height, width, dtype=torch.float32)*scale).to(torch_type)
            image = fast.Image.createFromTensor(tensor)

            assert image.getWidth() == width
            assert image.getHeight() == height
            assert image.getDepth() == depth
            assert image.getNrOfChannels() == channels
            assert image.getDataType() == fast_type
            assert torch.equal(tensor, torch.tensor(np.asarray(image)).permute((3, 0, 1, 2)))


def test_torch_tensor_to_image_exceptions():
    # Have to use createFromTensor not createFromArray
    tensor = torch.rand((1, 32, 32), dtype=torch.float32)
    with pytest.raises(ValueError):
        fast.Image.createFromArray(tensor)

    # Only 1 dim
    tensor = torch.rand((7,), dtype=torch.float32)
    with pytest.raises(ValueError):
        fast.Image.createFromTensor(tensor)

    # More than 4 dims
    tensor = torch.rand((7, 32, 13, 23, 54), dtype=torch.float32)
    with pytest.raises(ValueError):
        fast.Image.createFromTensor(tensor)

    # Incorrect type
    tensor = torch.rand((1, 32, 32), dtype=torch.float64)
    with pytest.raises(TypeError):
        fast.Image.createFromTensor(tensor)
    with pytest.raises(ValueError):
        fast.Image.createFromTensor('')


def test_torch_tensor_to_tensor_exceptions():
    # Incorrect type
    tensor = torch.rand((1, 32, 32), dtype=torch.float32)
    with pytest.raises(ValueError):
        fast.Tensor.createFromArray(tensor)
    with pytest.raises(ValueError):
        fast.Tensor.createFromTensor('')


def test_torch_tensor_fast_tensor():
    types_to_test = [
        (torch.uint8, 255),
        (torch.int8, 127),
        (torch.float32, 1),
        (torch.float64, 1),
        (torch.uint16, 128),
        (torch.int16, 128)
    ]
    for type, scale in types_to_test:
        shape = (23,)
        tensor = (torch.rand(shape)*scale).to(type)
        fast_tensor = fast.Tensor.createFromTensor(tensor)
        assert fast_tensor.getShape().getDimensions() == len(shape)
        assert fast_tensor.getShape().getAll()[0] == shape[0]
        assert torch.equal(tensor.to(torch.float32), torch.tensor(np.array(fast_tensor)))

        shape = (23,1,23,67)
        tensor = (torch.rand(shape)*scale).to(type)
        fast_tensor = fast.Tensor.createFromTensor(tensor)
        assert fast_tensor.getShape().getDimensions() == len(shape)
        for i in range(len(shape)):
            assert fast_tensor.getShape().getAll()[i] == shape[i]
        assert torch.equal(tensor.to(torch.float32), torch.tensor(np.array(fast_tensor)))


def test_torch_tensor_fast_tensor_channels_last_conversion():
    shape = (2, 32, 40)
    tensor = torch.rand(shape, dtype=torch.float32)
    fast_tensor = fast.Tensor.createFromTensor(tensor, convertToChannelsLast=True)
    assert fast_tensor.getShape().getDimensions() == len(shape)
    assert fast_tensor.getShape().getAll()[0] == shape[1]
    assert fast_tensor.getShape().getAll()[1] == shape[2]
    assert fast_tensor.getShape().getAll()[2] == shape[0]
    assert torch.equal(tensor.permute((1,2,0)), torch.tensor(np.array(fast_tensor)))
