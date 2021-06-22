## @example python_process_object.py
# An example showing how to make FAST process object in python.
import fast
import numpy as np

fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Show debug info


""" Make a python process object which simply inverts image with numpy """
class Inverter(fast.PythonProcessObject):
    def __init__(self):
        super().__init__()
        self.createInputPort(0)
        self.createOutputPort(0)

    def execute(self):
        print('In execute...')

        # Get image and invert it with numpy
        image = self.getInputImage()
        np_image = np.asarray(image)
        np_image = 255 - np_image # invert

        # Create new fast image and add as output
        new_output_image = fast.Image.createFromArray(np_image)
        new_output_image.setSpacing(image.getSpacing())
        self.addOutputData(0, new_output_image)
        print('Done in execute')


# Set up pipeline as normal
importer = fast.ImageFileStreamer.create(
    fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_#.mhd',
    loop=True,
)

inverter = Inverter.create().connect(importer)

renderer = fast.ImageRenderer.create().connect(inverter)

window = fast.SimpleWindow2D.create().connect(renderer).run()
