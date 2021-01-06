import fast
import numpy as np
fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Show debug info


""" Make a python process object which simply inverts image with numpy """
class Inverter(fast.PythonProcessObject):
    def __init__(self):
        super().__init__()
        self.createInputImagePort(0)
        self.createOutputImagePort(0)

    def execute(self):
        print('In execute...')

        # Get image and invert it with numpy
        image = self.getInputImage()
        np_image = np.asarray(image)
        np_image = 255 - np_image # invert

        # Create new fast image and add as output
        new_output_image = fast.Image.New()
        new_output_image.createFromArray(np_image)
        new_output_image.setSpacing(image.getSpacing())
        self.addOutputImage(0, new_output_image)
        print('Done in execute')



# Set up pipeline as normal
importer = fast.ImageFileStreamer.New()
importer.setFilenameFormat(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_#.mhd')
importer.enableLooping()

inverter = Inverter.New()
inverter.setInputConnection(importer.getOutputPort())

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(inverter.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
