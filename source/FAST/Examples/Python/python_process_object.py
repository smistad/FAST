## @example python_process_object.py
# An example showing how to make a FAST process object in python.
# A process object (PO) is a pipeline object which performs processing on zero or more input data
# and generates zero or more output data.
# @image html images/examples/python/python_process_object.jpg width=400px;
import fast
import numpy as np

# Check if OpenCV is available
use_opencv = False
try:
    import cv2
    use_opencv = True
except ImportError:
    pass


""" Make a python process object which simply inverts image with numpy """
class Inverter(fast.PythonProcessObject):
    def __init__(self):
        super().__init__()
        self.createInputPort(0)
        self.createOutputPort(0)

    def execute(self):
        # Get image and invert it with numpy
        image = self.getInputData()
        np_image = np.asarray(image)
        np_image = 255 - np_image # invert

        # If OpenCV is available, add some text using OpenCV
        if use_opencv:
            cv2.putText(np_image, 'OpenCV!', (40, 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,0), 2)

        # Create new fast image and add as output
        new_output_image = fast.Image.createFromArray(np_image)
        new_output_image.setSpacing(image.getSpacing())
        self.addOutputData(0, new_output_image)


# Set up pipeline as normal
importer = fast.ImageFileStreamer.create(
    fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_#.mhd',
    loop=True,
    framerate=40,
)

inverter = Inverter.create().connect(importer)

renderer = fast.ImageRenderer.create().connect(inverter)

window = fast.SimpleWindow2D.create().connect(renderer).run()
