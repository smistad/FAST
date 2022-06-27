import fast
import pytest
import numpy as np


class Rotate(fast.PythonProcessObject):
    """
    Simply rotate input image
    """
    def __init__(self):
        super().__init__()
        self.createInputPort(0)
        self.createOutputPort(0)

    def execute(self):
        # Get image and invert it with numpy
        image = self.getInputData()
        np_image = np.asarray(image).transpose()

        # Create new fast image and add as output
        new_output_image = fast.Image.createFromArray(np_image)
        self.addOutputData(0, new_output_image)


def test_python_process_object_single():
    importer = fast.ImageFileStreamer \
        .create(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_#.mhd')

    inverter = Rotate.create().connect(importer)
    dataStream = fast.DataStream(inverter)
    previousImage = ''
    counter = 0
    for image in dataStream:
        assert previousImage != image
        assert image.getWidth() == 591
        assert image.getHeight() == 234
        previousImage = image
        counter += 1
    assert counter == 100


def test_python_process_object_multiple():
    importer = fast.ImageFileStreamer \
        .create(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_#.mhd')

    inverter = Rotate.create().connect(importer)
    inverter2 = Rotate.create().connect(inverter)
    dataStream = fast.DataStream(inverter2)
    previousImage = ''
    counter = 0
    for image in dataStream:
        assert previousImage != image
        assert image.getWidth() == 234
        assert image.getHeight() == 591
        previousImage = image
        counter += 1
    assert counter == 100
